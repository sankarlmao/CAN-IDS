# TinyML CAN Intrusion Detection - Interactive STM32 Wokwi Simulation

This directory contains the STM32 Arduino firmware, Wokwi hardware config, and local verification tools for real-time CAN bus intrusion detection. 

The workspace has been updated to include an **SSD1306 OLED Display** for real-time data visualization and **4 colored selector buttons** to dynamically switch between traffic streams.

---

## 1. Hardware Diagram Configuration
*   **Nucleo-C031C6 Board**: STM32 microcontroller simulating TinyML inference.
*   **Red LED (PA5)**: Visual intrusion indicator (LED turns ON when an anomaly is detected).
*   **SSD1306 OLED Display (128x64)**: Shows current traffic stream name, parsed C-CAN byte 6 signal value, real-time anomaly score, anomaly level bar graph, and alert banners. Connected via default I2C pins (`A4` SDA, `A5` SCL).
*   **4 Interactive Push-buttons** (configured with pull-ups):
    *   **Green Button (Pin D2)**: Selects **Normal Traffic** stream.
    *   **Red Button (Pin D3)**: Selects **DoS Attack** stream.
    *   **Blue Button (Pin D4)**: Selects **Fuzzy Attack** stream.
    *   **Yellow Button (Pin D5)**: Selects **Impersonation Attack** stream.

---

## 2. Resolving Wokwi Compilation Errors
In Wokwi Arduino projects, all files must be placed flatly in the root directory. To avoid the `fatal error: edge-impulse-sdk/classifier/ei_run_classifier.h: No such file or directory` compiler error:
1.  In your Wokwi editor, copy the contents of `edge-impulse-sdk/classifier/ei_run_classifier.h` and write them to a new file in the root directory named **`ei_run_classifier.h`**.
2.  Your Wokwi files panel should look exactly like this:
    *   `sketch.ino` (firmware)
    *   `test_buffers.h` (CAN data slices)
    *   `ei_run_classifier.h` (mock classifier, or replaced with real SDK contents)
    *   `diagram.json` (hardware layout)

---

## 3. Step-by-Step Wokwi Setup
1.  Open [Wokwi](https://wokwi.com/) in your browser.
2.  Start a new project using the **STM32 Nucleo-C031C6** template.
3.  **Setup Files**:
    *   Copy the contents of `sketch.ino` into Wokwi's `sketch.ino`.
    *   Create `test_buffers.h` in Wokwi and copy the contents of your local `test_buffers.h` into it.
    *   Create `ei_run_classifier.h` in Wokwi and copy the contents of your local `edge-impulse-sdk/classifier/ei_run_classifier.h` into it.
    *   Replace Wokwi's `diagram.json` contents with the contents of your local `diagram.json`.
4.  **Library Manager**:
    *   Click on Wokwi's **Library Manager** (the book icon on the left sidebar).
    *   Search and add **Adafruit SSD1306** and **Adafruit GFX Library**.
5.  **Run & Interact**:
    *   Click the green **Start Simulation** button.
    *   Press the buttons (Green, Red, Blue, Yellow) to switch between different CAN streams.
    *   Watch the OLED render the anomaly bar graph and see the alert LED light up instantly on DoS and Fuzzy attacks!

---

## 4. Local Simulator Verification
To test the Arduino code logic directly on your local Arch Linux host:
1.  Run `make` to compile:
    ```bash
    make
    ```
2.  Execute the simulator binary:
    ```bash
    ./run_sim
    ```
3.  This runs a sequential loop simulating 10 steps of Normal Traffic followed by 10 steps of DoS Attack, logging outputs directly to the console.
