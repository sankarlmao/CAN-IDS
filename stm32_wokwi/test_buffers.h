#ifndef TEST_BUFFERS_H
#define TEST_BUFFERS_H

#include <stdint.h>

// Simulated normal CAN traffic slice (25 samples)
static const uint8_t normal_traffic_buffer[25] = {
    95, 95, 97, 97, 97, 99, 99, 99, 99, 97, 97, 95, 95, 97, 97, 97, 99, 99, 97, 97, 95, 95, 97, 97, 95
};

// Simulated DoS attack traffic slice (25 samples)
static const uint8_t dos_attack_buffer[25] = {
    91, 91, 91, 91, 91, 91, 93, 93, 99, 99, 91, 91, 91, 89, 89, 85, 85, 87, 87, 89, 89, 93, 93, 91, 91
};

// Simulated Fuzzy attack traffic slice (25 samples)
static const uint8_t fuzzy_attack_buffer[25] = {
    87, 89, 93, 95, 97, 91, 95, 93, 97, 95, 93, 91, 95, 97, 101, 105, 111, 103, 97, 93, 109, 107, 99, 93, 81
};

// Simulated Impersonation attack traffic slice (25 samples)
static const uint8_t impersonation_attack_buffer[25] = {
    99, 99, 99, 99, 97, 95, 93, 95, 95, 99, 97, 101, 99, 95, 93, 91, 93, 91, 93, 97, 97, 97, 97, 97, 97
};

#endif // TEST_BUFFERS_H
