# Building a TinyML Prototype for CAN Intrusion Detection

Transitioning from a static mathematical threshold to a TinyML model allows the STM32 to mathematically map the baseline of normal CAN traffic and identify deviations, such as DoS, Fuzzy, or Impersonation attacks, without rigid rules. 

Here is a comprehensive guide to building this software prototype using Edge Impulse and Wokwi.

## 1. Local Data Preparation
To train a TinyML model, the dataset must be structured as time-series data. 
Preparing the dataset locally on Arch Linux involves using Python or shell utilities to separate the normal CAN traffic from the attack traffic. 

Edge Impulse expects CSV or JSON files. Focus strictly on the numerical values derived from the CAN 'Data' fields (the Y-axis from the graphs). 

**Example CSV Structure:**
```csv
timestamp,data_value
0,94
100,96
200,95
300,92
```
*Note: Create separate CSV files for normal traffic (for training) and attack traffic (for testing).*

## 2. Cloud Training via Edge Impulse
Edge Impulse is ideal for generating embedded machine learning models that run on resource-constrained microcontrollers.

1. **Project Setup:** Create a free account on [Edge Impulse](https://studio.edgeimpulse.com) and start a new project.
2. **Data Ingestion:** Upload your formatted CSV files using the web interface or the Edge Impulse CLI.
3. **Impulse Design:**
   * **Time Series Data:** Set this as your input block.
   * **Processing Block:** Add a **Flatten** block. This extracts statistical features (like mean, standard deviation, and RMS) from the raw CAN data, which is highly effective for detecting anomalies in fluctuating signals.
   * **Learning Block:** Add an **Anomaly Detection (K-Means)** block.
4. **Training:** Train the model using *only* the normal data. The algorithm will create clusters representing standard network behavior. Any data point falling outside these clusters during testing will be assigned a high "anomaly score."

## 3. Exporting the C++ Library
Once the model is trained and verified against your DoS and Fuzzy datasets in the browser, it needs to be packaged for the microcontroller.

1. Navigate to the **Deployment** tab.
2. Select **C++ Library**. Do not select a specific hardware board. This generates a generic, highly optimized C++ package containing the neural network.
3. Download the `.zip` file. It contains hardware-agnostic code (including `ei_run_classifier.h` and the `model-parameters` directory), making it universally compatible with the Wokwi simulator.

## 4. Wokwi STM32 Deployment & Inference
With a solid background in C and low-level programming, integrating the generated library into an STM32 environment is straightforward.

1. **Wokwi Setup:** Open a new STM32 C++ project in Wokwi (e.g., STM32 Nucleo-C031C6).
2. **Import Library:** Upload the unzipped Edge Impulse library files into your Wokwi project workspace.
3. **Simulate Data:** Create a C-style array in your `main.cpp` containing a slice of your raw CAN data to act as a virtual sensor feed.

**Example Inference Logic (`main.cpp`):**
```cpp
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

// Simulated CAN data buffer
float can_data_buffer[] = {94.0, 96.0, 95.0, 92.0, /* ... */};
int buffer_index = 0;

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, can_data_buffer + offset, length * sizeof(float));
    return 0;
}

void loop() {
    signal_t features_signal;
    features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    features_signal.get_data = &raw_feature_get_data;

    ei_impulse_result_t result = { 0 };
    
    // Run the classifier
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
    
    if (res == EI_IMPULSE_OK) {
        printf("Anomaly Score: %.3f\n", result.anomaly);
        
        // Trigger alert if the anomaly score exceeds the threshold
        if (result.anomaly > 0.3) {
            printf("ALERT: CAN Intrusion Detected!\n");
            // Turn on a virtual LED wired to a GPIO pin here
        }
    }
    
    // Simulate timing of CAN messages
    for (volatile int i = 0; i < 100000; i++); 
}
```

By following this workflow, the complete TinyML pipeline is built and validated in software before committing to physical STM32 hardware.