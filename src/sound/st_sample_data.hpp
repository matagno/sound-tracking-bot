#pragma once
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Sound parameters
#define SAMPLE_RATE 44100
#define MIC_DISTANCE 0.10

struct SampleData {
    // Mutex
    SemaphoreHandle_t mutexAll;

    // Data
    std::vector<float> vecSamplesL;
    std::vector<float> vecSamplesR;

    // Init
    void init_value() {
        mutexAll = xSemaphoreCreateMutex();
        vecSamplesL.clear();
        vecSamplesR.clear();
    }
};