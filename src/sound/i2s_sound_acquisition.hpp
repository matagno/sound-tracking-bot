#pragma once
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "utils/biquad_filter.hpp"
#include "sound/st_sample_data.hpp"

// I2S
#define I2S_NUM I2S_NUM_0
// For cross-correlation 
#define WINDOW_SIZE 441

class I2sSoundAcquisition {
public:
    I2sSoundAcquisition(BiquadFilter &biquad_filterR, BiquadFilter &biquad_filterL, SampleData &sample_data);

    // Init functions
    void init_i2s();
    // Main function
    void i2s_acquisition();

private:
    // Ptr
    BiquadFilter &refBiquad_filterR;
    BiquadFilter &refBiquad_filterL;
    SampleData &refSample_data;

    // Attributes
    std::vector<float> windowL;
    std::vector<float> windowR;
};


