#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "BiquadFilter.hpp"
#include <vector>

extern std::vector<float> buffer;
extern SemaphoreHandle_t bufferMutex;
extern BiquadFilter bpFilterR;
extern BiquadFilter bpFilterL;

void i2s_task(void* arg);
