#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "utils/biquad_filter.hpp"
#include "utils/sample_data.hpp"
#include <vector>

// Var
extern SampleData sample_data;
extern BiquadFilter bpFilterR;
extern BiquadFilter bpFilterL;
extern std::vector<float> windowL;
extern std::vector<float> windowR;

// Fonction
void init_i2s();
void i2s_task(void* arg);
void register_samples(float left, float right);
void register_angle(float angle);
