#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "BiquadFilter.hpp"
#include <vector>

extern std::vector<float> bufferL;
extern std::vector<float> bufferR;
extern SemaphoreHandle_t bufferMutex;
extern BiquadFilter bpFilterR;
extern BiquadFilter bpFilterL;

void i2s_task(void* arg);
