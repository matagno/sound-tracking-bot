#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <vector>

extern std::vector<float> bufferL;
extern std::vector<float> bufferR;
extern SemaphoreHandle_t bufferMutex;

void com_task(void* arg);

