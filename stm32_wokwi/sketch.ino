#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

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

inline void pinMode(int pin, int mode) {}
inline void digitalWrite(int pin, int val) {}
inline int digitalRead(int pin) {
    return HIGH; // Buttons not pressed (pulled up)
}
inline void delay(int ms) {
    usleep(ms * 1000);
}

struct MockSerial {
    void begin(int baud) {}
    inline operator bool() { return true; }
    inline void print(const char* s) { printf("%s", s); }
    inline void print(int val) { printf("%d", val); }
    inline void print(double val, int dec = 2) { printf("%.*f", dec, val); }
    inline void println(const char* s) { printf("%s\n", s); }
    inline void println(int val) { printf("%d\n", val); }
    inline void println(double val, int dec = 2) { printf("%.*f\n", dec, val); }
};
static MockSerial Serial;

#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_WHITE 1
#define SSD1306_BLACK 0

struct Adafruit_SSD1306 {
    Adafruit_SSD1306(int w, int h, void* wire, int rst) {}
    inline bool begin(int type, int addr) { return true; }
    inline void clearDisplay() {}
    inline void setTextSize(int s) {}
    inline void setTextColor(int c) {}
    inline void setTextColor(int c, int bg) {}
    inline void setCursor(int x, int y) {}
    inline void print(const char* s) {}
    inline void print(int val) {}
    inline void print(double val, int dec = 2) {}
    inline void println(const char* s) {}
    inline void println(int val) {}
    inline void println(double val, int dec = 2) {}
    inline void display() {}
    inline void drawRect(int x, int y, int w, int h, int c) {}
    inline void fillRect(int x, int y, int w, int h, int c) {}
    inline void drawFastHLine(int x, int y, int w, int c) {}
};
static void* Wire = NULL;
#endif

#include "test_buffers.h"
#include "ei_run_classifier.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Global variables for active buffer stream
static const float* g_active_buffer = NULL;
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
    
    // Configure Pins
    pinMode(PA5, OUTPUT);          // Alert LED
    pinMode(D2, INPUT_PULLUP);    // Button 1: Normal Traffic selector
    pinMode(D3, INPUT_PULLUP);    // Button 2: DoS Attack selector
    pinMode(D4, INPUT_PULLUP);    // Button 3: Fuzzy Attack selector
    pinMode(D5, INPUT_PULLUP);    // Button 4: Impersonation Attack selector
    
    digitalWrite(PA5, LOW);
    
    // Initialize OLED display (I2C address 0x3C is standard)
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println("SSD1306 OLED initialization failed!");
        for (;;);
    }
    
    // Show splash screen
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 10);
    display.println("CAN-IDS");
    display.setTextSize(1);
    display.setCursor(15, 38);
    display.println("Intrusion Detection");
    display.setCursor(15, 50);
    display.println("TinyML Simulator");
    display.display();
    delay(2000);
    
    // Default to Normal Traffic
    g_active_buffer = normal_traffic_buffer;
    g_buffer_name = "Normal Traffic";
    g_active_buffer_len = sizeof(normal_traffic_buffer) / sizeof(normal_traffic_buffer[0]);
    g_global_offset = 0;
    
    Serial.println("System Ready. Stream: Normal Traffic");
}

void loop() {
    // Check buttons to switch active streams dynamically
    if (digitalRead(D2) == LOW) {
        g_active_buffer = normal_traffic_buffer;
        g_buffer_name = "Normal Traffic";
        g_active_buffer_len = sizeof(normal_traffic_buffer) / sizeof(normal_traffic_buffer[0]);
        g_global_offset = 0;
        Serial.println(">> Switched to: Normal Traffic");
        delay(200);
    } else if (digitalRead(D3) == LOW) {
        g_active_buffer = dos_attack_buffer;
        g_buffer_name = "DoS Attack";
        g_active_buffer_len = sizeof(dos_attack_buffer) / sizeof(dos_attack_buffer[0]);
        g_global_offset = 0;
        Serial.println(">> Switched to: DoS Attack");
        delay(200);
    } else if (digitalRead(D4) == LOW) {
        g_active_buffer = fuzzy_attack_buffer;
        g_buffer_name = "Fuzzy Attack";
        g_active_buffer_len = sizeof(fuzzy_attack_buffer) / sizeof(fuzzy_attack_buffer[0]);
        g_global_offset = 0;
        Serial.println(">> Switched to: Fuzzy Attack");
        delay(200);
    } else if (digitalRead(D5) == LOW) {
        g_active_buffer = impersonation_attack_buffer;
        g_buffer_name = "Impersonation";
        g_active_buffer_len = sizeof(impersonation_attack_buffer) / sizeof(impersonation_attack_buffer[0]);
        g_global_offset = 0;
        Serial.println(">> Switched to: Impersonation Attack");
        delay(200);
    }

    signal_t features_signal;
    features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    features_signal.get_data = &raw_feature_get_data;
    
    size_t max_offset = g_active_buffer_len - EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    
    // Run classifier
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
    
    float current_val = 0.0f;
    raw_feature_get_data(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 1, 1, &current_val);
    
    // Draw OLED Interface
    display.clearDisplay();
    
    // Header line
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("STREAM: ");
    display.print(g_buffer_name);
    display.drawFastHLine(0, 9, 128, SSD1306_WHITE);
    
    // Detailed stats
    display.setCursor(0, 13);
    display.print("Byte 6 (Val): ");
    display.print(current_val, 1);
    
    display.setCursor(0, 24);
    display.print("Anomaly Score: ");
    display.print(result.anomaly, 3);
    
    // Draw Anomaly score bar graph
    display.drawRect(0, 35, 128, 7, SSD1306_WHITE);
    int bar_width = (int)(result.anomaly * 126.0f);
    if (bar_width > 126) bar_width = 126;
    if (bar_width < 0) bar_width = 0;
    display.fillRect(1, 36, bar_width, 5, SSD1306_WHITE);
    
    // Status text and Alert toggle
    if (result.anomaly > 0.30f) {
        digitalWrite(PA5, HIGH); // Alert LED ON
        display.setCursor(0, 48);
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Inverted text for alert visibility
        display.print(" !!! INTRUSION ALERT !!! ");
        
        Serial.print("Offset: ");
        Serial.print((int)g_global_offset);
        Serial.print(" | Anomaly: ");
        Serial.print(result.anomaly, 3);
        Serial.println(" -> [ALERT] INTRUSION!");
    } else {
        digitalWrite(PA5, LOW); // Alert LED OFF
        display.setCursor(0, 48);
        display.setTextColor(SSD1306_WHITE);
        display.print("STATUS: SECURE [OK]");
        
        Serial.print("Offset: ");
        Serial.print((int)g_global_offset);
        Serial.print(" | Anomaly: ");
        Serial.print(result.anomaly, 3);
        Serial.println(" -> [OK]");
    }
    
    display.display();
    
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
