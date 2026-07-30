#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
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

struct SSD1306AsciiWire {
    inline void begin(const void* dev, int addr) {}
    inline void setFont(const void* font) {}
    inline void clear() {}
    inline void setCursor(int col, int row) {}
    inline void print(const char* s) {}
    inline void print(int val) {}
    inline void print(double val, int dec = 2) {}
    inline void println(const char* s) {}
    inline void println(int val) {}
    inline void println(double val, int dec = 2) {}
};
static SSD1306AsciiWire display;
static const void* Adafruit128x64 = NULL;
static const void* System5x7 = NULL;
#endif

#include "test_buffers.h"
#include "ei_run_classifier.h"

#ifdef ARDUINO
static SSD1306AsciiWire display;
#endif

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
    
    // Initialize OLED display using SSD1306Ascii (very light)
#ifdef ARDUINO
    Wire.begin();
    Wire.setClock(400000L);
    display.begin(&Adafruit128x64, 0x3C);
    display.setFont(System5x7);
#endif
    
    display.clear();
    display.println("=====================");
    display.println("    CAN-IDS DEMO     ");
    display.println("=====================");
    display.println("TinyML Anomaly Det.");
    display.println("OLED Monitor Active");
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
    run_classifier(&features_signal, &result, false);
    
    float current_val = 0.0f;
    raw_feature_get_data(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 1, 1, &current_val);
    
    // Draw OLED Interface
    display.clear();
    
    display.setCursor(0, 0);
    display.print("STREAM: ");
    display.println(g_buffer_name);
    display.println("---------------------");
    
    display.print("Byte 6 (Val): ");
    display.println(current_val, 1);
    
    display.print("Anomaly Score: ");
    display.println(result.anomaly, 3);
    
    // Draw Textual Anomaly progress bar
    display.print("[");
    int progress_chars = (int)(result.anomaly * 16.0f);
    if (progress_chars > 16) progress_chars = 16;
    if (progress_chars < 0) progress_chars = 0;
    for (int i = 0; i < 16; ++i) {
        if (i < progress_chars) {
            display.print("=");
        } else {
            display.print(" ");
        }
    }
    display.println("]");
    display.println("---------------------");
    
    // Status text and Alert toggle
    display.setCursor(0, 7);
    if (result.anomaly > 0.30f) {
        digitalWrite(PA5, HIGH); // Alert LED ON
        display.print("!!! INTRUSION ALERT !!!");
        
        Serial.print("Offset: ");
        Serial.print((int)g_global_offset);
        Serial.print(" | Anomaly: ");
        Serial.print(result.anomaly, 3);
        Serial.println(" -> [ALERT] INTRUSION!");
    } else {
        digitalWrite(PA5, LOW); // Alert LED OFF
        display.print("STATUS: SECURE [OK]");
        
        Serial.print("Offset: ");
        Serial.print((int)g_global_offset);
        Serial.print(" | Anomaly: ");
        Serial.print(result.anomaly, 3);
        Serial.println(" -> [OK]");
    }
    
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
