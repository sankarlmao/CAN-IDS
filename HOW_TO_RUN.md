# Execution and Setup Guide: CAN Sentinel IPS

This document provides a walkthrough explaining how to run and demonstrate the CAN Sentinel Automotive Intrusion Prevention System (IPS).

---

## System Requirements and Prerequisites

- **Operating System**: Linux / macOS / Windows
- **Python**: Python 3.8 or higher
- **Web Browser**: Google Chrome, Microsoft Edge, Brave, or Opera (Required for Web Serial API support)
- **C++ Compiler**: `g++` (Required only if testing the local CLI firmware simulation)
- **Hardware Simulator**: Web browser access to [Wokwi.com](https://wokwi.com/)

---

## Method 1: Running the Interactive Web CSOC Dashboard

The Web Cyber Security Operations Center (CSOC) Dashboard provides a real-time visualization of the vehicle network topology, active hardware failsafe actuators, OLED screen mirror, waveform charts, and acoustic sirens.

### Step 1: Start the Gateway Server
Open your terminal in the project root directory and execute:
```bash
python3 stm32_wokwi/wokwi_bridge.py --port 8080
```

### Step 2: Open the Visualizer in Browser
Open your browser and navigate to:
```
http://localhost:8080
```

### Step 3: Interacting with the Visualizer
1. **Traffic Stream Selectors**: Click the 4 scenario buttons on the left panel:
   - **Normal Traffic**: Simulates clean telemetry. Anomaly score remains low (~0.02), vehicle throttle is open (90 deg), gateway relay is connected (CLOSED).
   - **DoS Attack**: Simulates a high-rate 0x000 arbitration ID packet flood. Anomaly score spikes (>0.79), triggering emergency safe-stop (0 deg), relay trip (OPEN), and acoustic sirens.
   - **Fuzzy Attack**: Simulates randomized payload byte injections. Anomaly score spikes (>0.88), triggering defense countermeasures.
   - **Impersonation Attack**: Simulates spoofed node arbitration IDs. Anomaly score spikes (>0.94), isolating compromised bus segments.
2. **Acoustic Warning Siren**: Click "Enable Sound" in the hardware panel to synthesize piezo buzzer alarm tones in real time.
3. **CAN Topology Diagram**: Observe cyan data packets flowing smoothly during normal traffic, vs. red malicious packet floods and relay lockouts under cyber-attack.
4. **Forensic Log Export**: Click "Export Forensic CSV" to download a formatted security incident log with timestamps, payload bytes, and anomaly scores.

---

## Method 2: Running the Microcontroller Circuit Simulation in Wokwi

Demonstrate the firmware running on virtual STM32 Nucleo-C031C6 hardware with physical components.

### Step 1: Create Wokwi Project
1. Go to [Wokwi.com](https://wokwi.com/) and create a new project.
2. Select STM32 Nucleo-C031C6 template.

### Step 2: Load Project Files into Wokwi
Copy and paste the repository files into Wokwi:

| Wokwi Tab Name | Local Repository File Source |
|---|---|
| `sketch.ino` | Copy contents from `stm32_wokwi/sketch.ino` |
| `diagram.json` | Replace Wokwi `diagram.json` with contents from `stm32_wokwi/diagram.json` |
| `test_buffers.h` | Create new file `test_buffers.h` and paste from `stm32_wokwi/test_buffers.h` |
| `ei_run_classifier.h` | Create new file `ei_run_classifier.h` and paste from `stm32_wokwi/edge-impulse-sdk/classifier/ei_run_classifier.h` |

### Step 3: Libraries Configuration
Zero external library dependencies are required! `sketch.ino` includes an ultra-compact direct I2C OLED driver and zero-timer buzzer pulse routine to fit comfortably within the 32 KB STM32 Flash boundary. Leave `libraries.txt` empty.

### Step 4: Run the Hardware Simulation
1. Click the green Start Simulation play button.
2. Press the hardware colored pushbuttons in Wokwi:
   - Green (D2): Switch to Normal Traffic.
   - Red (D3): Inject DoS Attack.
   - Blue (D4): Inject Fuzzy Attack.
   - Yellow (D5): Inject Impersonation Attack.
3. Watch the physical components react in real time:
   - SG90 Servo: Rotates from 90 deg to 0 deg on attack.
   - Relay Module: Trips open on attack.
   - Piezo Buzzer: Emits audible warning sirens.
   - OLED Display: Updates telemetry text and progress bars.

### Step 5: Connect Wokwi Live Serial to Web CSOC Dashboard
1. Keep Wokwi running in one browser tab.
2. Open `http://localhost:8080` in another browser tab.
3. Click the "Connect Wokwi Serial" button in the header.
4. Select the serial/COM port assigned to your Wokwi session to stream live hardware telemetry into the web dashboard.

---

## Method 3: Running the C++ Firmware Simulator Locally on Linux PC

Test and verify the C++ firmware logic directly on your local terminal without needing a browser or microcontroller.

### Step 1: Navigate to the Simulation Directory
```bash
cd stm32_wokwi
```

### Step 2: Compile the Binary
```bash
make clean && make
```

### Step 3: Execute the Local Simulator
```bash
./run_sim
```

### Expected Output
The binary executes 10 iterations of Normal Traffic followed by 10 iterations of DoS Attack, logging formatted terminal output:
```text
==================================================
TinyML CAN Intrusion Detection Simulator (Host PC)
==================================================

System Initialized. Active Stream: Normal Traffic

--- Simulating Stream: Normal Traffic ---
Offset: 0 | Anomaly: 0.020 -> [OK] Normal Drive Operations
TELEMETRY_JSON:{"stream":"Normal Traffic","offset":0,"val":12.0,"anomaly":0.020,"actuator":90,"relay":1,"status":"SECURE"}

--- Simulating Stream: DoS Attack ---
Offset: 0 | Anomaly: 0.790 -> [ALERT] Emergency Brake & Bus Isolated!
TELEMETRY_JSON:{"stream":"DoS Attack","offset":0,"val":255.0,"anomaly":0.790,"actuator":0,"relay":0,"status":"ALERT"}
```

---

## Method 4: Data Preprocessing Pipeline (Optional)

If you wish to re-process raw CSV dataset files from the `10) CAN-Intrusion Dataset` folder into feature windows:

```bash
python3 preprocess.py
```
This generates cleaned, normalized numpy array feature vectors used for model inference.

---

## Troubleshooting and FAQs

### Port 8080 is already in use
If port 8080 is busy, specify a custom port:
```bash
python3 stm32_wokwi/wokwi_bridge.py --port 8085
```
Then navigate to `http://localhost:8085`.

### Wokwi throws `fatal error: ei_run_classifier.h`
Ensure you created the file `ei_run_classifier.h` in Wokwi's root tab panel and pasted the contents from `stm32_wokwi/edge-impulse-sdk/classifier/ei_run_classifier.h`.

### Web Serial API button does not open a prompt
Ensure you are using Google Chrome, Microsoft Edge, or Brave browser. Firefox currently does not support the Web Serial API standard natively.
