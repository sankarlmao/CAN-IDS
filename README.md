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

*(Add your specific installation and execution instructions below)*

### Prerequisites

* Python 3.8+
* [List your libraries, e.g., TensorFlow/PyTorch, Scikit-Learn, Pandas, NumPy]

### Installation

1. Clone the repository:
   ```bash
   git clone [https://github.com/yourusername/your-repo-name.git](https://github.com/yourusername/your-repo-name.git)
