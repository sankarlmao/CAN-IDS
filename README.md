# CAN Sentinel: TinyML Automotive CAN Intrusion Prevention & Failsafe System

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: STM32 Nucleo](https://img.shields.io/badge/Platform-STM32%20Nucleo--C031C6-orange.svg)](https://wokwi.com)
[![TinyML: Edge Impulse](https://img.shields.io/badge/TinyML-LSTM%20Anomaly%20Detection-green.svg)](#)

## 📌 Project Overview

The **Controller Area Network (CAN)** is the primary communication backbone of modern automotive architecture. However, traditional CAN bus protocols lack intrinsic authentication or encryption, making in-vehicle Electronic Control Units (ECUs) vulnerable to Denial-of-Service (DoS), Fuzzy frame injection, and Impersonation spoofing attacks.

**CAN Sentinel** is an advanced **Automotive Intrusion Prevention System (IPS)** that combines embedded **TinyML LSTM anomaly detection** with **active physical hardware failsafe countermeasures** and an interactive **Cyber Security Operations Center (CSOC) Web Dashboard**.

---

## ⚡ Key Architectural Features

- **TinyML LSTM Neural Network**: Operates on-chip on an **STM32 Nucleo-C031C6** micro-controller, performing real-time sliding-window anomaly inference with <15ms latency.
- **Emergency Safe-Stop Actuator (SG90 Servo)**: Replaces passive LED indicators with real physical vehicle intervention. Under attack, the actuator sweeps from **90° (Normal Drive)** to **0° (Emergency Safe-Stop Brake Lockout)**.
- **Hardware Bus Isolation Relay**: Gateway firewall that physically trips open (`RELAY: OPEN`) upon intrusion detection to isolate compromised ECU nodes from propagating malicious frames across the bus.
- **Multi-Frequency Acoustic Warning Siren**: Piezo buzzer warning siren triggering tailored alarm tones based on attack classification (1200 Hz DoS, 2000 Hz Fuzzy, 1600 Hz Impersonation).
- **SSD1306 Monochrome OLED Instrument Cluster**: Renders live anomaly gauges, parsed frame byte payloads, failsafe actuator state, and system health status.
- **Interactive HTML5 CSOC Visualizer**: High-tech automotive cyber-security dashboard with live network topology packet animation, real-time waveform plotting, audio siren synthesizer, and 1-click **Wokwi Web Serial API** integration.

---

## 🛠 Hardware Circuit & Wokwi Pinout

| Component | MCU Pin | Function |
|---|---|---|
| **SG90 Servo Motor** | `D6` (PWM) | Safe-Stop Emergency Brake Actuator |
| **CAN Isolation Relay** | `D8` | Hardware Gateway Firewall |
| **Piezo Buzzer** | `D7` | Multi-Pitch Warning Siren |
| **SSD1306 OLED (128x64)** | `D14` (SDA), `D15` (SCL) | In-Vehicle Telemetry Display |
| **Green Push-button** | `D2` | Select Normal CAN Traffic Stream |
| **Red Push-button** | `D3` | Select DoS Attack Stream |
| **Blue Push-button** | `D4` | Select Fuzzy Attack Stream |
| **Yellow Push-button** | `D5` | Select Impersonation Attack Stream |

---

## 💻 Getting Started & Wokwi Simulation

### Option A: Interactive Web CSOC Dashboard (Recommended)

1. Launch the local dashboard web server:
   ```bash
   python3 stm32_wokwi/wokwi_bridge.py --port 8080
   ```
2. Open `http://localhost:8080` in your web browser.
3. Switch between **Normal Traffic**, **DoS Attack**, **Fuzzy Attack**, and **Impersonation** to observe live packet propagation, hardware servo rotation, relay cutoff, OLED updates, and real-time waveform charts!

### Option B: Wokwi Microcontroller Hardware Simulation

1. Open [Wokwi.com](https://wokwi.com/) and create a new **STM32 Nucleo-C031C6** project.
2. Load the project files:
   - Copy [`stm32_wokwi/sketch.ino`](stm32_wokwi/sketch.ino) to `sketch.ino`.
   - Copy [`stm32_wokwi/diagram.json`](stm32_wokwi/diagram.json) to `diagram.json`.
   - Create `test_buffers.h` and paste contents from [`stm32_wokwi/test_buffers.h`](stm32_wokwi/test_buffers.h).
   - Create `ei_run_classifier.h` and paste contents from [`stm32_wokwi/edge-impulse-sdk/classifier/ei_run_classifier.h`](stm32_wokwi/edge-impulse-sdk/classifier/ei_run_classifier.h).
3. In Wokwi Library Manager, add **SSD1306Ascii** and **Servo**.
4. Press **Start Simulation** to run the hardware circuit.
5. In the Web CSOC Dashboard, click **🔌 Connect Wokwi Serial** to sync the browser dashboard 1:1 with Wokwi's live serial output!

### Option C: Local C++ Simulation Build

Validate the firmware C++ code on Linux:
```bash
cd stm32_wokwi
make
./run_sim
```

---

## 📊 Machine Learning Performance Summary

| Attack Type | Frame Signature | Anomaly Threshold | Mitigation Trigger | Failsafe Action |
|---|---|---|---|---|
| **Normal Traffic** | Periodic sensor signals (0x0C) | `0.020` (Below 0.30) | Benign | Pass-through (`90° Drive`, `Relay Closed`) |
| **DoS Attack** | High-rate 0x000 bus flood | `0.990` (EXCEEDED) | INTRUSION DETECTED | Emergency Brake (`0° Safe-Stop`), Bus Isolated (`Relay Open`), Siren 1200Hz |
| **Fuzzy Attack** | Randomized payload injections | `0.880` (EXCEEDED) | INTRUSION DETECTED | Emergency Brake (`0° Safe-Stop`), Bus Isolated (`Relay Open`), Siren 2000Hz |
| **Impersonation** | Spoofed ECU arbitration IDs | `0.940` (EXCEEDED) | INTRUSION DETECTED | Emergency Brake (`0° Safe-Stop`), Bus Isolated (`Relay Open`), Siren 1600Hz |

---

## 📄 License
This project is open-source and released under the MIT License.
