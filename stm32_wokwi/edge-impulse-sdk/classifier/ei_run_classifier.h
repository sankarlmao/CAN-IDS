#ifndef EI_RUN_CLASSIFIER_H
#define EI_RUN_CLASSIFIER_H

#include <stddef.h>
#include <stdint.h>

// Ultra-fast, zero-dependency Newton-Raphson square root to avoid linking libm.a
static inline float fast_sqrtf(float val) {
    if (val <= 0.0f) return 0.0f;
    float x = val;
    for (int i = 0; i < 6; ++i) {
        x = 0.5f * (x + val / x);
    }
    return x;
}

#define EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE 10

// Signal interface required by Edge Impulse SDK
typedef struct {
    size_t total_length;
    int (*get_data)(size_t offset, size_t length, float *out_ptr);
} signal_t;

// Result structure containing anomaly score
typedef struct {
    float anomaly;
} ei_impulse_result_t;

// Error codes
typedef enum {
    EI_IMPULSE_OK = 0,
    EI_IMPULSE_ERROR_SHAPE_MISMATCH = -1,
    EI_IMPULSE_ERROR_FAILED = -2
} EI_IMPULSE_ERROR;

// Mock K-Means classifier function using a Normal Behavior Bounding Box
inline EI_IMPULSE_ERROR run_classifier(signal_t *signal, ei_impulse_result_t *result, bool debug = false) {
    (void)debug;
    if (!signal || !signal->get_data || !result) {
        return EI_IMPULSE_ERROR_FAILED;
    }
    
    if (signal->total_length < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        return EI_IMPULSE_ERROR_SHAPE_MISMATCH;
    }
    
    // Read the window of features
    float window[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
    int res = signal->get_data(0, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, window);
    if (res != 0) {
        return EI_IMPULSE_ERROR_FAILED;
    }
    
    // Calculate Mean and Standard Deviation of the current window
    float sum = 0.0f;
    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ++i) {
        sum += window[i];
    }
    float mean = sum / EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    
    float variance_sum = 0.0f;
    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ++i) {
        variance_sum += (window[i] - mean) * (window[i] - mean);
    }
    float std_dev = fast_sqrtf(variance_sum / (float)EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    
    // Check deviation from Normal Bounding Box
    // In normal Kia Soul CAN traffic, the steering/engine values stay within [93.0, 99.5]
    // and standard deviation stays within [0.0, 2.0] (constant value is normal)
    float diff_mean = 0.0f;
    if (mean < 93.0f) {
        diff_mean = 93.0f - mean;
    } else if (mean > 99.5f) {
        diff_mean = mean - 99.5f;
    }
    
    float diff_std = 0.0f;
    if (std_dev > 2.0f) {
        diff_std = std_dev - 2.0f;
    }
    
    // Weighted Euclidean distance from normal behavior space
    float distance = fast_sqrtf((diff_mean * diff_mean * 1.5f) + (diff_std * diff_std * 3.0f));
    
    // Determine anomaly score (threshold is 0.30)
    float anomaly_score = 0.0f;
    if (distance > 0.2f) {
        // Distance outside normal space -> scales anomaly above 0.30
        anomaly_score = 0.30f + (distance / 4.0f);
        if (anomaly_score > 0.99f) anomaly_score = 0.99f;
    } else {
        // Inside or very close to normal space -> low anomaly score
        anomaly_score = 0.02f + (distance * 0.8f);
    }
    
    result->anomaly = anomaly_score;
    
    return EI_IMPULSE_OK;
}

#endif // EI_RUN_CLASSIFIER_H
