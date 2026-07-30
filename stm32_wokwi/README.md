# TinyML CAN Intrusion Detection - STM32 Firmware & Simulation Guide

This directory contains the STM32 C++ firmware, hardware configuration, and simulation files for real-time CAN bus intrusion detection. It includes a mock Edge Impulse SDK to compile and run the project immediately on a local host or inside the Wokwi simulator.

---

## 1. Project Directory Structure
*   `main.cpp`: Main firmware file containing the simulation loop, UART outputs, and LED alerting logic. Supports dual-compilation for host PCs (using GCC) and STM32 boards.
*   `diagram.json`: Hardware definition file for the Wokwi simulator, specifying the `STM32 Nucleo-C031C6` board, an alert LED, and a 220-ohm resistor connected to pin `PA5`.
*   `test_buffers.h`: Contains C-style float arrays representing slices of CAN traffic from the preprocessed HCRL datasets (`normal_traffic_buffer`, `dos_attack_buffer`, `fuzzy_attack_buffer`, `impersonation_attack_buffer`).
*   `Makefile`: Build file to compile the firmware locally on your host computer for quick testing and logic verification.
*   `edge-impulse-sdk/classifier/ei_run_classifier.h`: Mock header file implementing `run_classifier(...)` using a bounding box cluster heuristic. This simulates Edge Impulse's K-Means Anomaly Detection block before you export the real model.

---

## 2. Local Simulation & Verification
You can compile and run the project on your Arch Linux system immediately:
1.  Open a terminal in this directory.
2.  Run `make` to compile:
    ```bash
    make
    ```
3.  Execute the simulator:
    ```bash
    ./run_sim
    ```
4.  Observe the output logs showing how the normal traffic registers `[OK]` (low anomaly score, LED OFF), while DoS and Fuzzy attack traffic registers `[ALERT] INTRUSION!` (high anomaly score, LED ON).

---

## 3. Training the Real Model in Edge Impulse Studio
To replace the mock code with a real TinyML model:

### Step 1: Create a Project
1.  Go to [Edge Impulse Studio](https://studio.edgeimpulse.com/) and create a free account.
2.  Create a new project named **CAN-Intrusion-IDS**.

### Step 2: Upload Preprocessed Data
1.  Navigate to **Data Acquisition** -> **Upload existing data**.
2.  Upload `normal_traffic.csv` (located in the `processed_data/` directory) and select **Training** as the category.
3.  Upload `dos_attack.csv`, `fuzzy_attack.csv`, and `impersonation_attack.csv` and select **Testing** as the category.

### Step 3: Configure the Impulse
1.  Go to **Create Impulse**.
2.  Add a **Time Series Data** block. Set the window size to `100` (representing 10 samples of 10ms intervals) and window increase to `10`.
3.  Add a **Flatten** processing block (extracts statistical features like mean, std, min, max, rms, etc.).
4.  Add an **Anomaly Detection (K-Means)** learning block.
5.  Click **Save Impulse**.

### Step 4: Feature Extraction & Training
1.  Go to the **Flatten** tab, click **Save parameters**, then click **Generate features**.
2.  Go to the **Anomaly Detection** tab.
3.  Select the training features, set the number of clusters (e.g. `32`), and click **Start training**.
4.  Once trained, check the Anomaly Score threshold (default `0.30`) and verify classification performance.

### Step 5: Export C++ Library
1.  Go to the **Deployment** tab.
2.  Under "Search deployment options", select **C++ Library** (do not select a specific board).
3.  Click **Build** to download a `.zip` package.

---

## 4. Deploying & Simulating in Wokwi (STM32)
1.  Open [Wokwi](https://wokwi.com/) in your browser.
2.  Select **STM32 Nucleo-C031C6** (or create a new C++ project).
3.  **Import Files:**
    *   Copy the contents of `main.cpp` into Wokwi's `main.cpp`.
    *   Copy the contents of `diagram.json` into Wokwi's `diagram.json`.
    *   Create a new file in Wokwi named `test_buffers.h` and copy the contents of `test_buffers.h` into it.
4.  **Integration of Real Model:**
    *   Unzip the C++ library download from Edge Impulse.
    *   Upload the contents of the `edge-impulse-sdk` folder and the `model-parameters` folder directly into the Wokwi project directory.
    *   Delete the mock `edge-impulse-sdk` files.
5.  **Simulation & Alerting:**
    *   Click **Start Simulation** in Wokwi.
    *   Open the serial monitor to see the output logs.
    *   To test different attacks, modify the active buffer selector lines in Wokwi's `main.cpp` (uncomment the attack buffer you want to feed to the network).
    *   Watch the red external LED light up when an intrusion is detected!
