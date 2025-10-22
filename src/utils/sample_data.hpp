#pragma once
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Structure
struct SampleData {
    SemaphoreHandle_t mutex_buffer;
    std::vector<float> bufferL;
    std::vector<float> bufferR;
    SemaphoreHandle_t mutex_angle;
    float angle;
    void init_value(bool wMutex = true);
};
