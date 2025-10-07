#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "sample_data.hpp"
#include <vector>

// Var
extern SampleData sample_data;

// Fonction
void com_task(void* arg);

