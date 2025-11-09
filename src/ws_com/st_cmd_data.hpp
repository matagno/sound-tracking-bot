#pragma once
#include <array>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


// Structure
struct CmdData {
    // Mutex
    SemaphoreHandle_t mutexAll;

    // Data
    bool xAuto;
    bool xManu;
    bool xTeleop;
    bool xTeleop_run;
    bool xTeleop_turn;
    std::array<float, 12> qTarget_manual;
    std::array<bool, 12> qActive_manual;

    // Init
    void init_value() {
        mutexAll = xSemaphoreCreateMutex();
        xAuto = false;
        xManu = false;
        xTeleop = false;
        xTeleop_run = false;
        xTeleop_turn = false;
        qTarget_manual.fill(0.0f);
        qActive_manual.fill(false);
    }
};