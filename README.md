# CAN Security and Prediction Using Machine Learning

## Overview

The Controller Area Network (CAN) is an effective and robust communication network essential for integrating self-driving automobiles into modern transportation systems. As the complexity and data flow within CAN systems grow, traditional approaches struggle to maintain security, reliability, and efficiency. 

This project leverages Machine Learning (ML) models to address these modern challenges, offering advanced solutions for real-time data prediction, anomaly detection, fault diagnosis, and intrusion detection to secure automotive networks.

## Key Features

* **Anomaly Detection:** Identifies unusual patterns and events within the network.
* **Fault Diagnosis:** Preemptively diagnoses system failures to ensure vehicle reliability.
* **Real-Time Data Prediction:** Forecasts future CAN signals based on historical data sequences.
* **Intrusion Detection:** Enhances overall network security by triggering alerts regarding potential security lapses and unauthorized access.

## Machine Learning Architecture

We implemented and evaluated three distinct models to process CAN data and identify threats. 

| Model | Primary Function | Advantage |
|---|---|---|
| **ARIMA** (Auto Regressive Integrated Moving Average) | Signal Forecasting | Forecasts patterns in CAN signals to allow for preventive measures against system failures. |
| **MA** (Moving Average) | Trend Analysis | Reduces volatility in CAN data and highlights long-term patterns to improve data interpretation. |
| **LSTM** (Long Short-Term Memory) | Anomaly & Intrusion Detection | Excels at analyzing sequential data to predict unusual events and act as an intrusion alert system. |

## Results & Performance

The integration of these ML models significantly enhances both the prediction accuracy and the security of the CAN system. 

Based on our statistical evaluations, the **LSTM network outperformed the other models**, demonstrating superior capabilities in anomaly detection and intrusion alerting.

**Evaluation Metrics Used:**
* **MAPE** (Mean Absolute Percentage Error)
* **RMSE** (Root Mean Square Error)
* **MAE** (Mean Absolute Error)

---

## Getting Started

### Prerequisites

* Python 3.8+
* `pandas`, `numpy`, `scikit-learn` (for local preprocessing)
* C++ compiler (g++) (for local verification of the firmware)

### 1. Data Preprocessing (Optional)

If you wish to preprocess raw CAN traffic datasets:
1. Clone the repository:
   ```bash
   git clone https://github.com/sankarlmao/CAN-IDS.git
   cd CAN-IDS
   ```
2. Run the preprocessor:
   ```bash
   python preprocess.py
   ```

### 2. Running the Interactive Wokwi Simulation (Step-by-Step)

The project includes an interactive hardware simulation environment built on [Wokwi](https://wokwi.com/) simulating an **STM32 Nucleo-C031C6** running a TinyML anomaly detection classifier.

#### Simulation Features
* **Nucleo-C031C6 Board**: Runs the TinyML model.
* **Red LED (PA5)**: Visual alert that turns ON when an intrusion anomaly is detected.
* **SSD1306 OLED Display**: Renders real-time CAN traffic stream name, byte 6 signal value, anomaly score, and status banners.
* **4 Colored Selector Buttons**:
  * **Green (Pin D2)**: Switch to **Normal Traffic** stream.
  * **Red (Pin D3)**: Switch to **DoS Attack** stream.
  * **Blue (Pin D4)**: Switch to **Fuzzy Attack** stream.
  * **Yellow (Pin D5)**: Switch to **Impersonation Attack** stream.

#### Setup Instructions
1. Open [Wokwi](https://wokwi.com/) in your browser.
2. Create a new project and select the **STM32 Nucleo-C031C6** template.
3. Set up the virtual environment files in Wokwi:
   * **`sketch.ino`**: Copy the contents of [`stm32_wokwi/sketch.ino`](stm32_wokwi/sketch.ino) into the main `sketch.ino` tab.
   * **`test_buffers.h`**: Create a new file in Wokwi named `test_buffers.h` and copy the contents of [`stm32_wokwi/test_buffers.h`](stm32_wokwi/test_buffers.h) into it.
   * **`ei_run_classifier.h`**: Create a new file in Wokwi named `ei_run_classifier.h` and copy the contents of [`stm32_wokwi/edge-impulse-sdk/classifier/ei_run_classifier.h`](stm32_wokwi/edge-impulse-sdk/classifier/ei_run_classifier.h) into it.
   * **`diagram.json`**: Replace the contents of Wokwi's `diagram.json` with the contents of [`stm32_wokwi/diagram.json`](stm32_wokwi/diagram.json).
4. Install the required libraries in Wokwi:
   * Click on the **Library Manager** tab (book icon on the left toolbar).
   * Search for and install **SSD1306Ascii**.
5. Run the Simulation:
   * Click the green **Start Simulation** button (play icon).
   * Press the colored buttons (Green, Red, Blue, Yellow) to switch between different CAN traffic streams.
   * Observe the OLED display rendering the anomaly bar graph and see the red **Alert LED** light up instantly on DoS and Fuzzy attacks!

### 3. Local Firmware Simulation Verification

You can test the firmware logic directly on your local Linux machine without a microcontroller:
1. Navigate to the `stm32_wokwi` directory:
   ```bash
   cd stm32_wokwi
   ```
2. Compile the simulator:
   ```bash
   make
   ```
3. Run the compiled binary:
   ```bash
   ./run_sim
   ```
   This will run 10 steps of Normal Traffic and 10 steps of DoS Attack, logging outputs directly to the console.
