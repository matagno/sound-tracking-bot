#pragma once
#include <vector>
#include <cmath>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

// Sound parameters
#define SAMPLE_RATE 44100
#define MIC_DISTANCE 0.10


class SampleData {
public:
    SampleData();

    // Attributes
    SemaphoreHandle_t mutex_buffer; 
    std::vector<float> bufferL;
    std::vector<float> bufferR;

    // Functions
    void registerAngle();
    float getAngle();

private:
    // Private Attributes
    SemaphoreHandle_t mutex_angle;
    float angle;

    // Private Functions
    int calculateAngle(const std::vector<float>& sigL, const std::vector<float>& sigR);
    void setAngle(float newAngle);
    
};
