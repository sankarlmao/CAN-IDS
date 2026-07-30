#ifdef STM32
#include "stm32c0xx_hal.h"
#else
#include <stdio.h>
#include <unistd.h>
#endif

#include "test_buffers.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

// Global variables for the virtual CAN data stream
const float* g_active_buffer = NULL;
size_t g_active_buffer_len = 0;
size_t g_global_offset = 0;

// Callback function required by Edge Impulse SDK
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    size_t actual_offset = g_global_offset + offset;
    for (size_t i = 0; i < length; ++i) {
        size_t idx = actual_offset + i;
        if (idx < g_active_buffer_len) {
            out_ptr[i] = g_active_buffer[idx];
        } else {
            out_ptr[i] = 0.0f; // Padding
        }
    }
    return 0;
}

#ifdef STM32
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
#endif

int main(void) {
#ifdef STM32
    // Initialize HAL, clock configuration, and GPIO pins
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
#else
    printf("==================================================\n");
    printf("TinyML CAN Intrusion Detection Simulator\n");
    printf("==================================================\n\n");
#endif

    // ==========================================
    // DATASET SELECTOR
    // Un-comment the line below for the dataset you want to simulate.
    // ==========================================
    
#ifdef STM32
    // Test Case Selectors for STM32 board (uncomment one)
    g_active_buffer = normal_traffic_buffer;
    const char* buffer_name = "Normal Traffic";
    g_active_buffer_len = sizeof(normal_traffic_buffer) / sizeof(normal_traffic_buffer[0]);

    // g_active_buffer = dos_attack_buffer;
    // const char* buffer_name = "DoS Attack";
    // g_active_buffer_len = sizeof(dos_attack_buffer) / sizeof(dos_attack_buffer[0]);

    // g_active_buffer = fuzzy_attack_buffer;
    // const char* buffer_name = "Fuzzy Attack";
    // g_active_buffer_len = sizeof(fuzzy_attack_buffer) / sizeof(fuzzy_attack_buffer[0]);

    // g_active_buffer = impersonation_attack_buffer;
    // const char* buffer_name = "Impersonation Attack";
    // g_active_buffer_len = sizeof(impersonation_attack_buffer) / sizeof(impersonation_attack_buffer[0]);

    printf("Simulating data stream: %s (%d CAN data bytes)\n", buffer_name, (int)g_active_buffer_len);
    printf("Window Size (Features): %d\n\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);

    signal_t features_signal;
    features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    features_signal.get_data = &raw_feature_get_data;

    size_t max_offset = g_active_buffer_len - EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    g_global_offset = 0;

    while (1) {
        ei_impulse_result_t result = { 0 };
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);

        if (res == EI_IMPULSE_OK) {
            printf("Offset: %03d | Anomaly Score: %.3f", (int)g_global_offset, result.anomaly);
            if (result.anomaly > 0.30f) {
                printf(" -> [ALERT] INTRUSION DETECTED!\n");
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            } else {
                printf(" -> [OK]\n");
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            }
        }
        g_global_offset++;
        if (g_global_offset > max_offset) {
            g_global_offset = 0;
        }
        HAL_Delay(100);
    }
#else
    // Host PC: Run all four test cases sequentially
    struct TestCase {
        const float* buffer;
        size_t len;
        const char* name;
    } test_cases[] = {
        { normal_traffic_buffer, sizeof(normal_traffic_buffer)/sizeof(float), "Normal Traffic" },
        { dos_attack_buffer, sizeof(dos_attack_buffer)/sizeof(float), "DoS Attack" },
        { fuzzy_attack_buffer, sizeof(fuzzy_attack_buffer)/sizeof(float), "Fuzzy Attack" },
        { impersonation_attack_buffer, sizeof(impersonation_attack_buffer)/sizeof(float), "Impersonation Attack" }
    };

    signal_t features_signal;
    features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    features_signal.get_data = &raw_feature_get_data;

    for (int t = 0; t < 4; ++t) {
        g_active_buffer = test_cases[t].buffer;
        g_active_buffer_len = test_cases[t].len;
        g_global_offset = 0;
        
        printf("--- Simulating Test Case %d: %s (%d elements) ---\n", t + 1, test_cases[t].name, (int)g_active_buffer_len);
        
        size_t max_offset = g_active_buffer_len - EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
        for (int step = 0; step < 15; ++step) {
            ei_impulse_result_t result = { 0 };
            EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);

            if (res == EI_IMPULSE_OK) {
                printf("  Offset: %02d | Anomaly: %.3f", (int)g_global_offset, result.anomaly);
                if (result.anomaly > 0.30f) {
                    printf(" -> [ALERT] INTRUSION!\n");
                } else {
                    printf(" -> [OK]\n");
                }
            }
            g_global_offset++;
            if (g_global_offset > max_offset) break;
        }
        printf("\n");
    }
    printf("Local host simulation complete.\n");
#endif
    return 0;
}

#ifdef STM32
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    /*Configure GPIO pin : PA5 */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
#endif
