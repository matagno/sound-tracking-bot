#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <vector>

extern std::vector<float> buffer;
extern SemaphoreHandle_t bufferMutex;

void com_task(void* arg);

