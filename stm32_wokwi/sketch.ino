#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

// Mock Arduino environment for host compilation
#define INPUT_PULLUP 0x2
#define OUTPUT 0x1
#define LOW 0x0
#define HIGH 0x1

#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define D8 8
#define D9 9
#define D10 10
#define D11 11
#define D12 12
#define D13 13
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19

inline void pinMode(int pin, int mode) { (void)pin; (void)mode; }
inline void digitalWrite(int pin, int val) { (void)pin; (void)val; }
inline int digitalRead(int pin) { (void)pin; return HIGH; }
inline void delay(int ms) { usleep(ms * 1000); }
inline void delayMicroseconds(unsigned int us) { usleep(us); }

struct MockSerial {
    void begin(int baud) { (void)baud; }
    inline operator bool() { return true; }
    inline int available() { return 0; }
    inline char read() { return 0; }
    inline void print(const char* s) { printf("%s", s); }
    inline void print(int val) { printf("%d", val); }
    inline void print(double val, int dec = 2) { printf("%.*f", dec, val); }
    inline void println(const char* s = "") { printf("%s\n", s); }
    inline void println(int val) { printf("%d\n", val); }
    inline void println(double val, int dec = 2) { printf("%.*f\n", dec, val); }
};
static MockSerial Serial;
#endif

#include "ei_run_classifier.h"

// Hardware Pin Assignments
#define PIN_BTN_NORMAL       D2
#define PIN_BTN_DOS          D3
#define PIN_BTN_FUZZY        D4
#define PIN_BTN_IMPERSONATE  D5
#define PIN_BUZZER_ALARM     D7   // Warning Siren
#define PIN_RELAY_GATEWAY    D8   // Hardware CAN Bus Isolation Switch

// Dual Status Indicator LEDs
#define PIN_LED_SAFE_GREEN   D9   // Green Status LED: System Safe & Normal
#define PIN_LED_ALERT_RED    D13  // Red Status LED: Intrusion Threat Lock

// 10-LED CAN Bus Saturation Bar Graph Pins (Non-conflicting D6, D10-D12, A0-A5)
static const uint8_t g_bar_pins[10] = { D6, D10, D11, D12, A0, A1, A2, A3, A4, A5 };

// Global variables for active stream state
static uint8_t g_mode = 1; // 1: Normal, 2: DoS, 3: Fuzzy, 4: Impersonation
static size_t g_global_offset = 0;

// Ultra-lightweight 5x7 Font bitmap data (space through uppercase 'Z')
static const uint8_t font5x7_basic[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // 0x20 ' '
    0x00, 0x00, 0x5F, 0x00, 0x00, // 0x21 '!'
    0x00, 0x00, 0x00, 0x00, 0x00, // 0x22 '"'
    0x14, 0x7F, 0x14, 0x7F, 0x14, // 0x23 '#'
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // 0x24 '$'
    0x23, 0x13, 0x08, 0x64, 0x62, // 0x25 '%'
    0x36, 0x49, 0x55, 0x22, 0x50, // 0x26 '&'
    0x00, 0x05, 0x03, 0x00, 0x00, // 0x27 '\''
    0x00, 0x1C, 0x22, 0x41, 0x00, // 0x28 '('
    0x00, 0x41, 0x22, 0x1C, 0x00, // 0x29 ')'
    0x14, 0x08, 0x3E, 0x08, 0x14, // 0x2A '*'
    0x08, 0x08, 0x3E, 0x08, 0x08, // 0x2B '+'
    0x00, 0x50, 0x30, 0x00, 0x00, // 0x2C ','
    0x08, 0x08, 0x08, 0x08, 0x08, // 0x2D '-'
    0x00, 0x60, 0x60, 0x00, 0x00, // 0x2E '.'
    0x20, 0x10, 0x08, 0x04, 0x02, // 0x2F '/'
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0x30 '0'
    0x00, 0x42, 0x7F, 0x40, 0x00, // 0x31 '1'
    0x42, 0x61, 0x51, 0x49, 0x46, // 0x32 '2'
    0x21, 0x41, 0x45, 0x4B, 0x31, // 0x33 '3'
    0x18, 0x14, 0x12, 0x7F, 0x10, // 0x34 '4'
    0x27, 0x45, 0x45, 0x45, 0x39, // 0x35 '5'
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 0x36 '6'
    0x01, 0x71, 0x09, 0x05, 0x03, // 0x37 '7'
    0x36, 0x49, 0x49, 0x49, 0x36, // 0x38 '8'
    0x06, 0x49, 0x49, 0x29, 0x1E, // 0x39 '9'
    0x00, 0x36, 0x36, 0x00, 0x00, // 0x3A ':'
    0x00, 0x56, 0x36, 0x00, 0x00, // 0x3B ';'
    0x08, 0x14, 0x22, 0x41, 0x00, // 0x3C '<'
    0x14, 0x14, 0x14, 0x14, 0x14, // 0x3D '='
    0x00, 0x41, 0x22, 0x14, 0x08, // 0x3E '>'
    0x02, 0x01, 0x51, 0x09, 0x06, // 0x3F '?'
    0x32, 0x49, 0x79, 0x41, 0x3E, // 0x40 '@'
    0x7E, 0x11, 0x11, 0x11, 0x7E, // 0x41 'A'
    0x7F, 0x49, 0x49, 0x49, 0x36, // 0x42 'B'
    0x3E, 0x41, 0x41, 0x41, 0x22, // 0x43 'C'
    0x7F, 0x41, 0x41, 0x22, 0x1C, // 0x44 'D'
    0x7F, 0x49, 0x49, 0x49, 0x41, // 0x45 'E'
    0x7F, 0x09, 0x09, 0x09, 0x01, // 0x46 'F'
    0x3E, 0x41, 0x49, 0x49, 0x7A, // 0x47 'G'
    0x7F, 0x08, 0x08, 0x08, 0x7F, // 0x48 'H'
    0x00, 0x41, 0x7F, 0x41, 0x00, // 0x49 'I'
    0x20, 0x40, 0x41, 0x3F, 0x01, // 0x4A 'J'
    0x7F, 0x08, 0x14, 0x22, 0x41, // 0x4B 'K'
    0x7F, 0x40, 0x40, 0x40, 0x40, // 0x4C 'L'
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // 0x4D 'M'
    0x7F, 0x04, 0x08, 0x10, 0x7F, // 0x4E 'N'
    0x3E, 0x41, 0x41, 0x41, 0x3E, // 0x4F 'O'
    0x7F, 0x09, 0x09, 0x09, 0x06, // 0x50 'P'
    0x3E, 0x41, 0x51, 0x21, 0x5E, // 0x51 'Q'
    0x7F, 0x09, 0x19, 0x29, 0x46, // 0x52 'R'
    0x46, 0x49, 0x49, 0x49, 0x31, // 0x53 'S'
    0x01, 0x01, 0x7F, 0x01, 0x01, // 0x54 'T'
    0x3F, 0x40, 0x40, 0x40, 0x3F, // 0x55 'U'
    0x1F, 0x20, 0x40, 0x20, 0x1F, // 0x56 'V'
    0x3F, 0x40, 0x38, 0x40, 0x3F, // 0x57 'W'
    0x63, 0x14, 0x08, 0x14, 0x63, // 0x58 'X'
    0x07, 0x08, 0x70, 0x08, 0x07, // 0x59 'Y'
    0x61, 0x51, 0x49, 0x45, 0x43, // 0x5A 'Z'
    0x00, 0x7F, 0x41, 0x41, 0x00, // 0x5B '['
    0x02, 0x04, 0x08, 0x10, 0x20, // 0x5C '\\'
    0x00, 0x41, 0x41, 0x7F, 0x00, // 0x5D ']'
    0x00, 0x00, 0x77, 0x00, 0x00  // 0x7C '|'
};

#ifdef ARDUINO
static void oled_cmd(uint8_t cmd) {
    Wire.beginTransmission(0x3C);
    Wire.write(0x00);
    Wire.write(cmd);
    Wire.endTransmission();
}

static void oled_init() {
    Wire.begin();
    Wire.setClock(400000L);
    delay(50);
    oled_cmd(0xAE); // Display off
    oled_cmd(0xD5); oled_cmd(0x80);
    oled_cmd(0xA8); oled_cmd(0x3F);
    oled_cmd(0xD3); oled_cmd(0x00);
    oled_cmd(0x40);
    oled_cmd(0x8D); oled_cmd(0x14); // Enable charge pump
    oled_cmd(0x20); oled_cmd(0x00); // Horizontal mode
    oled_cmd(0xA1); // Seg remap
    oled_cmd(0xC8); // Com scan dec
    oled_cmd(0xDA); oled_cmd(0x12);
    oled_cmd(0x81); oled_cmd(0xCF);
    oled_cmd(0xD9); oled_cmd(0xF1);
    oled_cmd(0xDB); oled_cmd(0x40);
    oled_cmd(0xA4); // Output follows RAM
    oled_cmd(0xA6); // Normal display
    oled_cmd(0xAF); // Display on
}

static void oled_set_cursor(uint8_t col, uint8_t row) {
    oled_cmd(0xB0 + (row & 0x07));
    oled_cmd(0x00 + (col & 0x0F));
    oled_cmd(0x10 + ((col >> 4) & 0x0F));
}

static void oled_clear() {
    for (uint8_t row = 0; row < 8; row++) {
        oled_set_cursor(0, row);
        for (uint8_t chunk = 0; chunk < 8; chunk++) {
            Wire.beginTransmission(0x3C);
            Wire.write(0x40);
            for (uint8_t col = 0; col < 16; col++) {
                Wire.write(0x00);
            }
            Wire.endTransmission();
        }
    }
}

static void oled_invert(bool inv) {
    oled_cmd(inv ? 0xA7 : 0xA6);
}

static void oled_write_char(char c, uint8_t col, uint8_t row) {
    oled_set_cursor(col, row);
    char u = c;
    if (u >= 'a' && u <= 'z') u = u - 'a' + 'A';
    uint16_t idx = 0;
    if (u >= 0x20 && u <= 0x5D) idx = (u - 0x20) * 5;
    else if (u == '|') idx = (0x5E - 0x20) * 5;

    Wire.beginTransmission(0x3C);
    Wire.write(0x40);
    for (uint8_t i = 0; i < 5; i++) {
        Wire.write(font5x7_basic[idx + i]);
    }
    Wire.write(0x00); // 1px space between characters
    Wire.endTransmission();
}

static void oled_write_str(const char* str, uint8_t col, uint8_t row) {
    uint8_t curr_col = col;
    while (*str && curr_col < 122) {
        oled_write_char(*str, curr_col, row);
        curr_col += 6;
        str++;
    }
}

static void oled_write_val1(float val, uint8_t col, uint8_t row) {
    int v = (int)(val * 10.0f);
    if (v < 0) {
        oled_write_char('-', col, row);
        col += 6;
        v = -v;
    }
    int whole = v / 10;
    int frac = v % 10;
    char buf[10];
    int idx = 0;
    if (whole == 0) {
        buf[idx++] = '0';
    } else {
        char tmp[8];
        int t_idx = 0;
        while (whole > 0) {
            tmp[t_idx++] = '0' + (whole % 10);
            whole /= 10;
        }
        while (t_idx > 0) {
            buf[idx++] = tmp[--t_idx];
        }
    }
    buf[idx++] = '.';
    buf[idx++] = '0' + frac;
    buf[idx] = '\0';
    oled_write_str(buf, col, row);
}

static void oled_write_anom(float val, uint8_t col, uint8_t row) {
    int v = (int)(val * 100.0f);
    if (v < 0) v = 0;
    int whole = v / 100;
    int frac = v % 100;
    char buf[8];
    buf[0] = '0' + (whole % 10);
    buf[1] = '.';
    buf[2] = '0' + (frac / 10);
    buf[3] = '0' + (frac % 10);
    buf[4] = '\0';
    oled_write_str(buf, col, row);
}
#else
static void oled_init() {}
static void oled_clear() {}
static void oled_invert(bool inv) { (void)inv; }
static void oled_write_str(const char* str, uint8_t col, uint8_t row) { (void)str; (void)col; (void)row; }
static void oled_write_val1(float val, uint8_t col, uint8_t row) { (void)val; (void)col; (void)row; }
static void oled_write_anom(float val, uint8_t col, uint8_t row) { (void)val; (void)col; (void)row; }
#endif

// Zero-RAM algorithmic CAN signal generator
inline float get_can_sample(uint8_t mode, size_t idx) {
    if (mode == 1) { return 95.0f + (float)((idx * 3) % 5); }
    else if (mode == 2) { return 87.0f + (float)((idx * 2) % 4); }
    else if (mode == 3) { return 85.0f + (float)((idx * 17) % 31); }
    else { return 99.0f - (float)((idx * 5) % 9); }
}

const char* get_mode_name(uint8_t mode) {
    if (mode == 1) return "NORMAL";
    if (mode == 2) return "DOS";
    if (mode == 3) return "FUZZY";
    return "IMPERSONATION";
}

// Compact float formatting helper
inline void printVal1(float f) {
    int v = (int)(f * 10.0f);
    if (v < 0) { Serial.print("-"); v = -v; }
    Serial.print(v / 10);
    Serial.print(".");
    Serial.print(v % 10);
}

inline void printAnom3(float f) {
    int v = (int)(f * 1000.0f);
    if (v < 0) { Serial.print("-"); v = -v; }
    Serial.print(v / 1000);
    Serial.print(".");
    int rem = v % 1000;
    if (rem < 100) Serial.print("0");
    if (rem < 10) Serial.print("0");
    Serial.print(rem);
}

// Callback required by Edge Impulse SDK
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    for (size_t i = 0; i < length; ++i) {
        out_ptr[i] = get_can_sample(g_mode, g_global_offset + offset + i);
    }
    return 0;
}

// Zero-timer hardware siren pulse
static void buzzer_pulse(uint16_t freq_hz, uint16_t duration_ms) {
#ifdef ARDUINO
    if (freq_hz == 0) {
        digitalWrite(PIN_BUZZER_ALARM, LOW);
        return;
    }
    uint32_t period_us = 1000000UL / (uint32_t)freq_hz;
    uint32_t half_period = period_us / 2;
    uint32_t cycles = ((uint32_t)duration_ms * 1000UL) / period_us;
    for (uint32_t i = 0; i < cycles; i++) {
        digitalWrite(PIN_BUZZER_ALARM, HIGH);
        delayMicroseconds(half_period);
        digitalWrite(PIN_BUZZER_ALARM, LOW);
        delayMicroseconds(half_period);
    }
#else
    (void)freq_hz;
    (void)duration_ms;
#endif
}

// Update 10-LED CAN Bus Saturation Bar Graph
static void update_bus_bar_graph(uint8_t mode) {
    uint8_t lit_bars = 3; // Default 30% load for Normal
    if (mode == 2) lit_bars = 10;      // DoS Attack: 100% Saturation (10 bars)
    else if (mode == 3) lit_bars = 7;  // Fuzzy Attack: 70% Load (7 bars)
    else if (mode == 4) lit_bars = 5;  // Impersonation: 50% Load (5 bars)

    for (uint8_t i = 0; i < 10; i++) {
        digitalWrite(g_bar_pins[i], (i < lit_bars) ? HIGH : LOW);
    }
}

void setup() {
    Serial.begin(115200);
    
    pinMode(PIN_BTN_NORMAL, INPUT_PULLUP);
    pinMode(PIN_BTN_DOS, INPUT_PULLUP);
    pinMode(PIN_BTN_FUZZY, INPUT_PULLUP);
    pinMode(PIN_BTN_IMPERSONATE, INPUT_PULLUP);
    
    pinMode(PIN_BUZZER_ALARM, OUTPUT);
    digitalWrite(PIN_BUZZER_ALARM, LOW);
    pinMode(PIN_RELAY_GATEWAY, OUTPUT);
    
    // Configure Indicator LEDs
    pinMode(PIN_LED_ALERT_RED, OUTPUT);
    pinMode(PIN_LED_SAFE_GREEN, OUTPUT);
    digitalWrite(PIN_LED_ALERT_RED, LOW);
    digitalWrite(PIN_LED_SAFE_GREEN, HIGH); // Green Safe ON by default
    
    // Configure 10-LED Bar Graph Pins (D6, D10-D12, A0-A5)
    for (uint8_t i = 0; i < 10; i++) {
        pinMode(g_bar_pins[i], OUTPUT);
    }
    update_bus_bar_graph(1);
    
    digitalWrite(PIN_RELAY_GATEWAY, HIGH);
    
    oled_init();
    oled_clear();
    oled_write_str("CAN-IDS FAILSAFE", 0, 0);
    oled_write_str("TINYML ACTIVE", 0, 2);
    delay(1000);
    
    g_mode = 1;
    g_global_offset = 0;
    
    Serial.println("System Initialized");
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        if (cmd == '1' || cmd == 'N' || cmd == 'n') { g_mode = 1; g_global_offset = 0; }
        else if (cmd == '2' || cmd == 'D' || cmd == 'd') { g_mode = 2; g_global_offset = 0; }
        else if (cmd == '3' || cmd == 'F' || cmd == 'f') { g_mode = 3; g_global_offset = 0; }
        else if (cmd == '4' || cmd == 'I' || cmd == 'i') { g_mode = 4; g_global_offset = 0; }
    }

    if (digitalRead(PIN_BTN_NORMAL) == LOW) { g_mode = 1; g_global_offset = 0; delay(200); }
    else if (digitalRead(PIN_BTN_DOS) == LOW) { g_mode = 2; g_global_offset = 0; delay(200); }
    else if (digitalRead(PIN_BTN_FUZZY) == LOW) { g_mode = 3; g_global_offset = 0; delay(200); }
    else if (digitalRead(PIN_BTN_IMPERSONATE) == LOW) { g_mode = 4; g_global_offset = 0; delay(200); }

    signal_t features_signal;
    features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    features_signal.get_data = &raw_feature_get_data;
    
    ei_impulse_result_t result = { 0 };
    run_classifier(&features_signal, &result, false);
    
    float current_val = get_can_sample(g_mode, g_global_offset + EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 1);
    bool is_intrusion = (result.anomaly > 0.30f);

    // Update 10-LED Bus Saturation Bar Graph
    update_bus_bar_graph(g_mode);

    if (is_intrusion) {
        digitalWrite(PIN_RELAY_GATEWAY, LOW); // Cut Relay
        digitalWrite(PIN_LED_ALERT_RED, HIGH);
        digitalWrite(PIN_LED_SAFE_GREEN, LOW);

        if (g_mode == 2) buzzer_pulse(1200, 60);
        else if (g_mode == 3) buzzer_pulse(2000, 60);
        else buzzer_pulse(1600, 60);
    } else {
        digitalWrite(PIN_RELAY_GATEWAY, HIGH); // Restore Relay
        digitalWrite(PIN_LED_SAFE_GREEN, HIGH);
        digitalWrite(PIN_LED_ALERT_RED, LOW);
        digitalWrite(PIN_BUZZER_ALARM, LOW);
    }

    oled_invert(is_intrusion && ((g_global_offset % 2) == 0));
    oled_clear();
    
    oled_write_str("STREAM:", 0, 0);
    oled_write_str(get_mode_name(g_mode), 48, 0);
    
    oled_write_str("VAL:", 0, 2);
    oled_write_val1(current_val, 30, 2);
    
    oled_write_str("ANOM:", 66, 2);
    oled_write_anom(result.anomaly, 96, 2);
    
    oled_write_str("STATUS:", 0, 4);
    oled_write_str(is_intrusion ? "ALERT LOCKOUT" : "NORMAL DRIVE", 48, 4);
    
    oled_write_str("BUS:", 0, 5);
    oled_write_str(is_intrusion ? "ISOLATED" : "CONNECTED", 30, 5);
    
    oled_write_str(is_intrusion ? "! INTRUSION DETECTED !" : "STATUS: SECURE [OK]", 0, 7);
    
    // Output Structured JSON Telemetry Packet over Serial (For Web Dashboard / Wokwi Bridge)
    uint8_t load_pct = (g_mode == 2) ? 100 : ((g_mode == 3) ? 70 : ((g_mode == 4) ? 50 : 30));
    Serial.print("TELEMETRY_JSON:{\"stream\":\"");
    Serial.print(get_mode_name(g_mode));
    Serial.print("\",\"offset\":");
    Serial.print((int)g_global_offset);
    Serial.print(",\"val\":");
    printVal1(current_val);
    Serial.print(",\"anomaly\":");
    printAnom3(result.anomaly);
    Serial.print(",\"load\":");
    Serial.print((int)load_pct);
    Serial.print(",\"actuator\":");
    Serial.print(is_intrusion ? 0 : 90);
    Serial.print(",\"relay\":");
    Serial.print(is_intrusion ? 0 : 1);
    Serial.print(",\"status\":\"");
    Serial.print(is_intrusion ? "ALERT" : "SECURE");
    Serial.println("\"}");

    g_global_offset++;
    delay(is_intrusion ? 440 : 500);
}

#ifndef ARDUINO
int main() {
    setup();
    for (int i = 0; i < 20; ++i) {
        loop();
    }
    return 0;
}
#endif
