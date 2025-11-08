#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "biquad_filter.hpp"
#include "sample_data.hpp"
#include <vector>

// Var extern
extern BiquadFilter bpFilterR;
extern BiquadFilter bpFilterL;
extern SampleData sample_data;

// Var I2S
extern std::vector<float> windowL;
extern std::vector<float> windowR;

// Fonction
void init_i2s();
void i2s_task(void* arg);
