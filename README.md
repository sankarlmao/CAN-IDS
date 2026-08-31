# CAN Sentinel: TinyML Automotive CAN Intrusion Prevention System

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: STM32 Nucleo](https://img.shields.io/badge/Platform-STM32%20Nucleo--C031C6-orange.svg)](https://wokwi.com)

## Project Overview

The Controller Area Network (CAN) is the primary communication backbone of modern automotive architecture. However, traditional CAN bus protocols lack intrinsic authentication or encryption, making in-vehicle Electronic Control Units (ECUs) vulnerable to Denial-of-Service (DoS), Fuzzy frame injection, and Impersonation spoofing attacks.

CAN Sentinel is an embedded Automotive Intrusion Prevention System (IPS) that combines on-chip TinyML LSTM anomaly detection with active physical hardware failsafe countermeasures and a Web Cyber Security Operations Center (CSOC) Dashboard.

---

## Architectural Features

- **TinyML LSTM Neural Network**: Operates on-chip on an STM32 Nucleo-C031C6 microcontroller, performing real-time sliding-window anomaly inference with low latency.
- **Emergency Safe-Stop Actuator (SG90 Servo)**: Replaces passive indicator lights with physical vehicle intervention. Under attack, the actuator rotates from 90 degrees (Normal Drive) to 0 degrees (Emergency Safe-Stop Brake Lockout).
- **Hardware Bus Isolation Relay**: Gateway firewall that trips open (RELAY: OPEN) upon intrusion detection to isolate compromised ECU nodes from propagating malicious frames across the bus.
- **Multi-Frequency Acoustic Warning Siren**: Piezo buzzer warning siren triggering tailored alarm tones based on attack classification (1200 Hz DoS, 2000 Hz Fuzzy, 1600 Hz Impersonation).
- **SSD1306 Monochrome OLED Instrument Cluster**: Displays live anomaly gauges, parsed frame byte payloads, failsafe actuator state, and system health status.
- **Interactive Web CSOC Visualizer**: Automotive cybersecurity dashboard with network topology packet animation, real-time waveform plotting, audio siren synthesizer, and Web Serial API integration.

---

## Hardware Circuit and Wokwi Pinout

| Component | MCU Pin | Function |
|---|---|---|
| SG90 Servo Motor | D6 (PWM) | Safe-Stop Emergency Brake Actuator |
| CAN Isolation Relay | D8 | Hardware Gateway Firewall |
| Piezo Buzzer | D7 | Multi-Pitch Warning Siren |
| SSD1306 OLED (128x64) | D14 (SDA), D15 (SCL) | In-Vehicle Telemetry Display |
| Green Pushbutton | D2 | Select Normal CAN Traffic Stream |
| Red Pushbutton | D3 | Select DoS Attack Stream |
| Blue Pushbutton | D4 | Select Fuzzy Attack Stream |
| Yellow Pushbutton | D5 | Select Impersonation Attack Stream |

---

## Getting Started and Wokwi Simulation

### Option A: Interactive Web CSOC Dashboard

1. Launch the local dashboard web server:
   ```bash
   python3 stm32_wokwi/wokwi_bridge.py --port 8080
   ```
2. Open `http://localhost:8080` in your web browser.
3. Switch between Normal Traffic, DoS Attack, Fuzzy Attack, and Impersonation to observe packet propagation, hardware servo rotation, relay cutoff, OLED updates, and real-time waveform charts.

### Option B: Wokwi Microcontroller Hardware Simulation

1. Open [Wokwi.com](https://wokwi.com/) and create a new STM32 Nucleo-C031C6 project.
2. Load the project files:
   - Copy `stm32_wokwi/sketch.ino` to `sketch.ino`.
   - Copy `stm32_wokwi/diagram.json` to `diagram.json`.
   - Create `test_buffers.h` and paste contents from `stm32_wokwi/test_buffers.h`.
   - Create `ei_run_classifier.h` and paste contents from `stm32_wokwi/edge-impulse-sdk/classifier/ei_run_classifier.h`.
3. In Wokwi Library Manager, add SSD1306Ascii and Servo.
4. Press Start Simulation to run the hardware circuit.
5. In the Web CSOC Dashboard, click "Connect Wokwi Serial" to sync the browser dashboard with Wokwi serial output.

### Option C: Local C++ Simulation Build

Validate the firmware C++ code on Linux:
```bash
cd stm32_wokwi
make clean && make
./run_sim
```

---

## Machine Learning Performance Summary

| Attack Type | Frame Signature | Anomaly Threshold | Mitigation Trigger | Failsafe Action |
|---|---|---|---|---|
| Normal Traffic | Periodic sensor signals (0x0C) | 0.020 (Below 0.30) | Benign | Pass-through (90 deg Drive, Relay Closed) |
| DoS Attack | High-rate 0x000 bus flood | 0.990 (EXCEEDED) | INTRUSION DETECTED | Emergency Brake (0 deg Safe-Stop), Bus Isolated (Relay Open), Siren 1200Hz |
| Fuzzy Attack | Randomized payload injections | 0.880 (EXCEEDED) | INTRUSION DETECTED | Emergency Brake (0 deg Safe-Stop), Bus Isolated (Relay Open), Siren 2000Hz |
| Impersonation | Spoofed ECU arbitration IDs | 0.940 (EXCEEDED) | INTRUSION DETECTED | Emergency Brake (0 deg Safe-Stop), Bus Isolated (Relay Open), Siren 1600Hz |

---

## License
This project is open-source and released under the MIT License.
