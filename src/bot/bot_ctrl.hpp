#pragma once
#include <array>
#include <math.h>
#include "esp_log.h"

#include "utils/pca9685.hpp"
#include "ws_com/st_cmd_data.hpp"
#include "sound/st_sample_data.hpp"



class BotCtrl {
public:
    BotCtrl(CmdData& cmd_data, SampleData& sample_data, PCA9685& driver);
    // Ptr data
    CmdData& adrCmd_data;
    SampleData& adrSample_data;

    // Mutex
    SemaphoreHandle_t mutexData_transmit;

    // Attributes
    float fAngle;
    bool xAuto;
    bool xManu;
    bool xTeleop;
    bool xTeleop_run;
    bool xTeleop_turn;
    std::array<float, 12> qTarget;
    std::array<bool, 12> qActive;


    // Methods 
    void init_value();
    void update_servos();
    void update_sound_angle();
    void update_move_mode();

private:
    // Constants
    static constexpr float SERVO_MIN_ANGLE = 0.0f;
    static constexpr float SERVO_MAX_ANGLE = 180.0f;
    static constexpr uint16_t SERVO_MIN_TICK = 205;
    static constexpr uint16_t SERVO_MAX_TICK = 410;

    // Ptr hardware
    PCA9685& adrPCA;
    
    // Methods 
    void set_servo_angle(uint8_t id, float angle);
    int calculate_angle(const std::vector<float>& sigL, const std::vector<float>& sigR);

};

