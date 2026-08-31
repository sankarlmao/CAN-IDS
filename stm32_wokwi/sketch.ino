#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
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

#define PA5 5
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define D8 8

inline void pinMode(int pin, int mode) { (void)pin; (void)mode; }
inline void digitalWrite(int pin, int val) { (void)pin; (void)val; }
inline int digitalRead(int pin) { (void)pin; return HIGH; }
inline void delay(int ms) { usleep(ms * 1000); }
inline void tone(int pin, unsigned int freq, unsigned long duration = 0) { (void)pin; (void)freq; (void)duration; }
inline void noTone(int pin) { (void)pin; }

struct Servo {
    inline void attach(int pin) { (void)pin; }
    inline void write(int angle) { (void)angle; }
};

struct MockSerial {
    void begin(int baud) { (void)baud; }
    inline operator bool() { return true; }
    inline int available() { return 0; }
    inline char read() { return 0; }
    inline void print(const char* s) { printf("%s", s); }
    inline void print(int val) { printf("%d", val); }
    inline void print(double val, int dec = 2) { printf("%.*f", dec, val); }
    inline void println(const char* s) { printf("%s\n", s); }
    inline void println(int val) { printf("%d\n", val); }
    inline void println(double val, int dec = 2) { printf("%.*f\n", dec, val); }
};
static MockSerial Serial;

struct SSD1306AsciiWire {
    inline void begin(const void* dev, int addr) { (void)dev; (void)addr; }
    inline void setFont(const void* font) { (void)font; }
    inline void clear() {}
    inline void setCursor(int col, int row) { (void)col; (void)row; }
    inline void print(const char* s) { (void)s; }
    inline void print(int val) { (void)val; }
    inline void print(double val, int dec = 2) { (void)val; (void)dec; }
    inline void println(const char* s) { (void)s; }
    inline void println(int val) { (void)val; }
    inline void println(double val, int dec = 2) { (void)val; (void)dec; }
};
static SSD1306AsciiWire display;
static const void* Adafruit128x64 = NULL;
static const void* System5x7 = NULL;
#endif

#include "test_buffers.h"
#include "ei_run_classifier.h"

// Hardware Pin Assignments
#define PIN_BTN_NORMAL       D2
#define PIN_BTN_DOS          D3
#define PIN_BTN_FUZZY        D4
#define PIN_BTN_IMPERSONATE  D5
#define PIN_SERVO_BRAKE      D6   // Emergency Safe-Stop Actuator
#define PIN_BUZZER_ALARM     D7   // Multi-Frequency Cyber Warning Siren
#define PIN_RELAY_GATEWAY    D8   // Hardware CAN Bus Isolation Switch

#ifdef ARDUINO
static SSD1306AsciiWire display;
#endif

static Servo brakeServo;

// Global variables for active buffer stream
static const uint8_t* g_active_buffer = NULL;
static size_t g_active_buffer_len = 0;
static size_t g_global_offset = 0;
static const char* g_buffer_name = "";

// Callback required by Edge Impulse SDK
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    size_t actual_offset = g_global_offset + offset;
    for (size_t i = 0; i < length; ++i) {
        size_t idx = actual_offset + i;
        if (idx < g_active_buffer_len) {
            out_ptr[i] = g_active_buffer[idx];
        } else {
            out_ptr[i] = 0.0f; // Padding
        }
    }
    return 0;
}

void setup() {
    Serial.begin(115200);
    
    // Configure Input Pin Selectors
    pinMode(PIN_BTN_NORMAL, INPUT_PULLUP);
    pinMode(PIN_BTN_DOS, INPUT_PULLUP);
    pinMode(PIN_BTN_FUZZY, INPUT_PULLUP);
    pinMode(PIN_BTN_IMPERSONATE, INPUT_PULLUP);
    
    // Configure Actuator Output Pins
    pinMode(PIN_BUZZER_ALARM, OUTPUT);
    pinMode(PIN_RELAY_GATEWAY, OUTPUT);
    
    // Attach and set Failsafe Servo to Normal Throttle position (90 degrees)
    brakeServo.attach(PIN_SERVO_BRAKE);
    brakeServo.write(90);
    
    // Set CAN Bus Gateway Relay to Connected state (HIGH = Closed Circuit)
    digitalWrite(PIN_RELAY_GATEWAY, HIGH);
    noTone(PIN_BUZZER_ALARM);
    
    // Initialize OLED display using SSD1306Ascii
#ifdef ARDUINO
    Wire.begin();
    Wire.setClock(400000L);
    display.begin(&Adafruit128x64, 0x3C);
    display.setFont(System5x7);
#endif
    
    display.clear();
    display.println("=====================");
    display.println(" CAN-IDS FAILSAFE ");
    display.println("=====================");
    display.println("TinyML Sentinel Active");
    display.println("Actuators Calibrated");
    delay(1500);
    
    // Default to Normal Traffic
    g_active_buffer = normal_traffic_buffer;
    g_buffer_name = "Normal Traffic";
    g_active_buffer_len = sizeof(normal_traffic_buffer) / sizeof(normal_traffic_buffer[0]);
    g_global_offset = 0;
    
    Serial.println("System Initialized. Active Stream: Normal Traffic");
}

void loop() {
    // 1. Process Serial Commands (Remote Web Bridge Control)
    if (Serial.available()) {
        char cmd = Serial.read();
        if (cmd == '1' || cmd == 'N' || cmd == 'n') {
            g_active_buffer = normal_traffic_buffer;
            g_buffer_name = "Normal Traffic";
            g_active_buffer_len = sizeof(normal_traffic_buffer) / sizeof(normal_traffic_buffer[0]);
            g_global_offset = 0;
        } else if (cmd == '2' || cmd == 'D' || cmd == 'd') {
            g_active_buffer = dos_attack_buffer;
            g_buffer_name = "DoS Attack";
            g_active_buffer_len = sizeof(dos_attack_buffer) / sizeof(dos_attack_buffer[0]);
            g_global_offset = 0;
        } else if (cmd == '3' || cmd == 'F' || cmd == 'f') {
            g_active_buffer = fuzzy_attack_buffer;
            g_buffer_name = "Fuzzy Attack";
            g_active_buffer_len = sizeof(fuzzy_attack_buffer) / sizeof(fuzzy_attack_buffer[0]);
            g_global_offset = 0;
        } else if (cmd == '4' || cmd == 'I' || cmd == 'i') {
            g_active_buffer = impersonation_attack_buffer;
            g_buffer_name = "Impersonation";
            g_active_buffer_len = sizeof(impersonation_attack_buffer) / sizeof(impersonation_attack_buffer[0]);
            g_global_offset = 0;
        }
    }

    // 2. Process Physical Button Selectors
    if (digitalRead(PIN_BTN_NORMAL) == LOW) {
        g_active_buffer = normal_traffic_buffer;
        g_buffer_name = "Normal Traffic";
        g_active_buffer_len = sizeof(normal_traffic_buffer) / sizeof(normal_traffic_buffer[0]);
        g_global_offset = 0;
        delay(200);
    } else if (digitalRead(PIN_BTN_DOS) == LOW) {
        g_active_buffer = dos_attack_buffer;
        g_buffer_name = "DoS Attack";
        g_active_buffer_len = sizeof(dos_attack_buffer) / sizeof(dos_attack_buffer[0]);
        g_global_offset = 0;
        delay(200);
    } else if (digitalRead(PIN_BTN_FUZZY) == LOW) {
        g_active_buffer = fuzzy_attack_buffer;
        g_buffer_name = "Fuzzy Attack";
        g_active_buffer_len = sizeof(fuzzy_attack_buffer) / sizeof(fuzzy_attack_buffer[0]);
        g_global_offset = 0;
        delay(200);
    } else if (digitalRead(PIN_BTN_IMPERSONATE) == LOW) {
        g_active_buffer = impersonation_attack_buffer;
        g_buffer_name = "Impersonation";
        g_active_buffer_len = sizeof(impersonation_attack_buffer) / sizeof(impersonation_attack_buffer[0]);
        g_global_offset = 0;
        delay(200);
    }

    signal_t features_signal;
    features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    features_signal.get_data = &raw_feature_get_data;
    
    size_t max_offset = g_active_buffer_len - EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    
    // Run classifier
    ei_impulse_result_t result = { 0 };
    run_classifier(&features_signal, &result, false);
    
    float current_val = 0.0f;
    raw_feature_get_data(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 1, 1, &current_val);
    
    bool is_intrusion = (result.anomaly > 0.30f);

    // Apply Active Hardware Mitigation Controls
    if (is_intrusion) {
        // 1. Emergency Safe-Stop Actuator: Rotate Servo to 0 degrees (Brake Lockout)
        brakeServo.write(0);
        
        // 2. Hardware Gateway Firewall: Open Relay to Isolate Compromised CAN Bus
        digitalWrite(PIN_RELAY_GATEWAY, LOW);
        
        // 3. Acoustic Warning Siren: Trigger alarm tone depending on attack type
        if (strcmp(g_buffer_name, "DoS Attack") == 0) {
            tone(PIN_BUZZER_ALARM, 1200); // Urgent 1.2kHz tone
        } else if (strcmp(g_buffer_name, "Fuzzy Attack") == 0) {
            tone(PIN_BUZZER_ALARM, 2000); // Rapid 2.0kHz tone
        } else {
            tone(PIN_BUZZER_ALARM, 1600); // 1.6kHz warning tone
        }
    } else {
        // 1. Throttle / Safe Operation: Servo at 90 degrees (Normal position)
        brakeServo.write(90);
        
        // 2. CAN Gateway: Closed Relay (Normal CAN frame pass-through)
        digitalWrite(PIN_RELAY_GATEWAY, HIGH);
        
        // 3. Siren Muted
        noTone(PIN_BUZZER_ALARM);
    }

    // Draw Dynamic OLED Telemetry Interface
    display.clear();
    
    display.setCursor(0, 0);
    display.print("STREAM: ");
    display.println(g_buffer_name);
    display.println("---------------------");
    
    display.print("Val: ");
    display.print(current_val, 1);
    display.print(" | Anom: ");
    display.println(result.anomaly, 2);
    
    // Draw Progress bar
    display.print("[");
    int progress_chars = (int)(result.anomaly * 16.0f);
    if (progress_chars > 16) progress_chars = 16;
    if (progress_chars < 0) progress_chars = 0;
    for (int i = 0; i < 16; ++i) {
        if (i < progress_chars) display.print("=");
        else display.print(" ");
    }
    display.println("]");
    
    display.print("ACT: ");
    display.println(is_intrusion ? "SAFE-STOP (0deg)" : "NORMAL (90deg)");
    
    display.print("BUS: ");
    display.println(is_intrusion ? "ISOLATED [CUT]" : "CONNECTED [OK]");
    
    display.setCursor(0, 7);
    if (is_intrusion) {
        display.print("! INTRUSION DETECTED !");
        
        Serial.print("Offset: ");
        Serial.print((int)g_global_offset);
        Serial.print(" | Anomaly: ");
        Serial.print(result.anomaly, 3);
        Serial.println(" -> [ALERT] Emergency Brake & Bus Isolated!");
    } else {
        display.print("STATUS: SECURE [OK]");
        
        Serial.print("Offset: ");
        Serial.print((int)g_global_offset);
        Serial.print(" | Anomaly: ");
        Serial.print(result.anomaly, 3);
        Serial.println(" -> [OK] Normal Drive Operations");
    }
    
    // Output Structured JSON Telemetry Packet over Serial (For Web Dashboard / Wokwi Bridge)
    Serial.print("TELEMETRY_JSON:{\"stream\":\"");
    Serial.print(g_buffer_name);
    Serial.print("\",\"offset\":");
    Serial.print((int)g_global_offset);
    Serial.print(",\"val\":");
    Serial.print(current_val, 1);
    Serial.print(",\"anomaly\":");
    Serial.print(result.anomaly, 3);
    Serial.print(",\"actuator\":");
    Serial.print(is_intrusion ? 0 : 90);
    Serial.print(",\"relay\":");
    Serial.print(is_intrusion ? 0 : 1);
    Serial.print(",\"status\":\"");
    Serial.print(is_intrusion ? "ALERT" : "SECURE");
    Serial.println("\"}");

    // Slide window
    g_global_offset++;
    if (g_global_offset > max_offset) {
        g_global_offset = 0;
    }
    
    delay(500); // 500 ms sampling step
}

#ifndef ARDUINO
// Main entry point for local Linux PC execution
int main() {
    printf("==================================================\n");
    printf("TinyML CAN Intrusion Detection Simulator (Host PC)\n");
    printf("==================================================\n\n");
    
    setup();
    
    // Simulate a few steps for each test dataset
    // Case 1: Normal
    printf("\n--- Simulating Stream: Normal Traffic ---\n");
    for (int i = 0; i < 10; ++i) {
        loop();
    }
    
    // Case 2: DoS
    g_active_buffer = dos_attack_buffer;
    g_buffer_name = "DoS Attack";
    g_active_buffer_len = sizeof(dos_attack_buffer) / sizeof(dos_attack_buffer[0]);
    g_global_offset = 0;
    printf("\n--- Simulating Stream: DoS Attack ---\n");
    for (int i = 0; i < 10; ++i) {
        loop();
    }
    
    return 0;
}
#endif
