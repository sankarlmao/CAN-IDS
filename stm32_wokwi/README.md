# TinyML Automotive CAN Intrusion Prevention System (IPS) - Wokwi Simulation

This directory contains the STM32 firmware, hardware actuators, Wokwi circuit diagram, and real-time Web Serial Gateway for an end-to-end **Automotive CAN Intrusion Prevention System (IPS)**.

---

## 🏎️ Hardware Actuators & Protection Architecture

Instead of basic alerts, this system implements active automotive cyber-defense countermeasures:

| Hardware Component | Pin | Purpose & Failsafe Function |
|---|---|---|
| **SG90 Servo Actuator** | `D6` | **Vehicle Safe-Stop System**: Serves as throttle/brake lockout. Rotates from **90° (Normal Drive)** to **0° (Emergency Safe-Stop)** upon intrusion detection. |
| **CAN Gateway Relay Module** | `D8` | **Bus Isolation Firewall**: Closed circuit (`HIGH`) under normal traffic. Instantly trips open (`LOW`) to physically cut off compromised CAN bus segments. |
| **Acoustic Warning Siren** | `D7` | **Multi-Pitch Piezo Alarm**: Triggers distinct cyber warning sirens based on attack classification (1200 Hz DoS, 2000 Hz Fuzzy, 1600 Hz Impersonation). |
| **SSD1306 OLED Screen** | `D14`/`D15` | **Instrument Cluster Telemetry**: Renders real-time CAN byte values, anomaly score gauge, actuator status, and isolation state. |
| **4 Tactical Push-buttons** | `D2`–`D5` | Hardware traffic selectors for Normal Traffic, DoS Attack, Fuzzy Attack, and Impersonation. |

---

## 🌐 Wokwi-to-Web CSOC Real-Time Integration

The system supports **1:1 Live Telemetry Synchronization** with the HTML5 Web CSOC Dashboard:

1. **Structured Serial Telemetry**:
   The MCU streams JSON packets over 115200 baud UART:
   ```json
   TELEMETRY_JSON:{"stream":"DoS Attack","offset":3,"val":255.0,"anomaly":0.944,"actuator":0,"relay":0,"status":"ALERT"}
   ```
2. **Direct Web Serial API**:
   Open the Web CSOC Dashboard (`http://localhost:8080`), click **🔌 Connect Wokwi Serial**, and select the Wokwi/STM32 COM port for instant 1:1 telemetry plotting & remote stream control!
3. **Python Bridge Server (`wokwi_bridge.py`)**:
   Run the gateway bridge:
   ```bash
   python3 wokwi_bridge.py --port 8080
   ```

---

## 🚀 Wokwi Simulation Setup Instructions

1. Open [Wokwi.com](https://wokwi.com/) and create a new **STM32 Nucleo-C031C6** project.
2. Replace `diagram.json` with the contents of [`stm32_wokwi/diagram.json`](diagram.json).
3. Copy `sketch.ino` into Wokwi's `sketch.ino`.
4. Create `test_buffers.h` in Wokwi and paste the contents from [`test_buffers.h`](test_buffers.h).
5. Create `ei_run_classifier.h` in Wokwi and paste the contents from [`edge-impulse-sdk/classifier/ei_run_classifier.h`](edge-impulse-sdk/classifier/ei_run_classifier.h).
6. In Wokwi **Library Manager** (book icon), search and install **SSD1306Ascii** and **Servo**.
7. Click **Start Simulation**!

---

## 💻 Local Host Verification

Verify the firmware logic on Linux without hardware:
```bash
make
./run_sim
```
