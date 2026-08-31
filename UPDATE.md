# Session Update Summary: CAN Sentinel v2.0

## 📌 Executive Summary
In this session, the **CAN Intrusion Detection System (CAN-IDS)** was upgraded from a basic LED indicator into a full-scale **Automotive CAN Intrusion Prevention System (IPS)** with active hardware failsafe actuators, real-time Wokwi microcontroller integration, and a high-tech Web Cyber Security Operations Center (CSOC) Dashboard.

---

## 🛠 Key Changes & System Enhancements

### 1. Hardware Actuators & Active Defense Integration (`stm32_wokwi/sketch.ino` & `diagram.json`)
- **SG90 Servo Motor Actuator (`D6`)**: Added as a physical **Vehicle Safe-Stop Emergency Brake Actuator**. Rotates from **90° (Normal Drive)** to **0° (Emergency Safe-Stop)** when TinyML anomaly score exceeds threshold (>0.30).
- **CAN Gateway Isolation Relay (`D8`)**: Added as a **Hardware Bus Firewall Switch**. Closed (`HIGH`) under normal traffic, opening (`LOW`) upon threat detection to isolate compromised ECU nodes from propagating malicious frames.
- **Multi-Frequency Acoustic Warning Siren (`D7`)**: Added a piezo sounder emitting dynamic alarm frequencies based on attack classification (1200 Hz DoS, 2000 Hz Fuzzy, 1600 Hz Impersonation).
- **Enhanced OLED Telemetry Interface (`SSD1306`)**: Upgraded display layout to show real-time signal byte value, anomaly score, progress gauge, active actuator angle, and bus connection status.
- **Serial Remote Command Listener & JSON Telemetry**:
  - Transmits structured `TELEMETRY_JSON:{"stream":..., "val":..., "anomaly":..., "actuator":..., "relay":...}` packets over 115200 baud UART.
  - Listens for remote serial commands (`1` Normal, `2` DoS, `3` Fuzzy, `4` Impersonation) for 1:1 hardware synchronization.

### 2. Wokwi Circuit Diagram Upgrade (`stm32_wokwi/diagram.json`)
- Replaced basic LED with:
  - `wokwi-servo` (Brake actuator connected to pin `D6`)
  - `wokwi-relay-module` (Gateway firewall connected to pin `D8`)
  - `wokwi-buzzer` (Acoustic alarm connected to pin `D7`)
  - `board-ssd1306` (OLED screen connected to `D14`/`D15` I2C)
  - 4 Push-buttons (`D2`, `D3`, `D4`, `D5`) for hardware traffic injection.

### 3. Web CSOC Dashboard & Telemetry Visualizer (`web_dashboard/`)
Created a standalone web application featuring:
- **`index.html`**: Structured layout for stream selection, CAN network topology, hardware actuators, OLED mirror, waveform charts, and forensic table.
- **`styles.css`**: Automotive dark theme with glassmorphism, glowing status indicators, and responsive grid system.
- **`app.js`**: Real-time canvas waveform plotting, animated CAN packet propagation on CAN_HIGH/CAN_LOW differential lines, Web Audio API acoustic siren synthesizer, CSV forensic log export, and **Web Serial API (`navigator.serial`) driver**.

### 4. Python Wokwi Gateway Bridge (`stm32_wokwi/wokwi_bridge.py`)
- Created Python bridge script serving the CSOC web dashboard static assets and relaying serial telemetry between Wokwi / physical microcontrollers and browser clients.

### 5. Updated Project Documentation (`README.md` & `stm32_wokwi/README.md`)
- Comprehensive setup guides for running the Web CSOC Visualizer, Wokwi hardware circuit simulation, and local C++ firmware builds.

---

## 📁 Modified & Created Files

| File Path | Status | Purpose |
|---|---|---|
| [`UPDATE.md`](UPDATE.md) | Created | Summary log of all session changes |
| [`README.md`](README.md) | Updated | Core project documentation & setup guide |
| [`stm32_wokwi/sketch.ino`](stm32_wokwi/sketch.ino) | Updated | Firmware with Servo, Relay, Siren, OLED & JSON telemetry |
| [`stm32_wokwi/diagram.json`](stm32_wokwi/diagram.json) | Updated | Wokwi hardware wiring diagram |
| [`stm32_wokwi/README.md`](stm32_wokwi/README.md) | Updated | Wokwi simulation configuration guide |
| [`stm32_wokwi/wokwi_bridge.py`](stm32_wokwi/wokwi_bridge.py) | Created | Python Web/Serial gateway bridge |
| [`web_dashboard/index.html`](web_dashboard/index.html) | Created | CSOC Web Dashboard HTML structure |
| [`web_dashboard/styles.css`](web_dashboard/styles.css) | Created | CSOC Web Dashboard CSS design system |
| [`web_dashboard/app.js`](web_dashboard/app.js) | Created | Web telemetry visualizer & Web Serial driver |

---

## 🚀 Quick Execution Commands

### Launch Web CSOC Visualizer & Server
```bash
python3 stm32_wokwi/wokwi_bridge.py --port 8080
```
*Open `http://localhost:8080` in your browser.*

### Build & Run Local Host C++ Simulator
```bash
cd stm32_wokwi
make clean && make
./run_sim
```
