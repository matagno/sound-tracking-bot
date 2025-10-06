#pragma once
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Structure
struct SampleData {
    SemaphoreHandle_t mutex;
    std::vector<float> bufferL;
    std::vector<float> bufferR;
    float angle;
};

// Var
extern SampleData sample_data;

// Fonction
void init_sample_data();
