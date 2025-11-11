#pragma once
#include <array>
#include <math.h>
#include "esp_log.h"

#include "utils/pca9685.hpp"
#include "ws_com/st_cmd_data.hpp"
#include "sound/st_sample_data.hpp"

#include "utils/ik_calcul.hpp"



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
    void autonomous_move(float t, float angle_target, bool run_cmd, bool turn_cmd);

    
    // Ptr hardware
    PCA9685& adrPCA;

private:
    // Constants
    static constexpr float SERVO_MIN_ANGLE = 0.0f;
    static constexpr float SERVO_MAX_ANGLE = 180.0f;
    static constexpr uint16_t SERVO_MIN_TICK = 102;
    static constexpr uint16_t SERVO_MAX_TICK = 491;


    // Attributes (turn track)
    bool turn_last_cycle;
    float t0_rot;
    float angle_to_turn;
    
    // Methods 
    void set_servo_angle(uint8_t id, float angle);
    int calculate_angle(const std::vector<float>& sigL, const std::vector<float>& sigR);

    std::tuple<float,float,float> foot_traj(float t, float step_length, float step_height, float T=2.0f);
    std::tuple<float,float,float> rot_traj(float t, int phase_id, std::array<float,3> xyz_start_in_bot,
                                          float angle, float step_height, float T=2.0f);
};

