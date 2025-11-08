#pragma once
#include "pca9685.hpp"
#include <array>

class servo_ctrl {
public:
    servo_ctrl(PCA9685& driver);
    void updateServos(const std::array<float, 12>& q_output, const std::array<bool, 12>& q_output_active);

private:
    PCA9685& ptrPCA;
    static constexpr float SERVO_MIN_ANGLE = 0.0f;
    static constexpr float SERVO_MAX_ANGLE = 180.0f;
    static constexpr uint16_t SERVO_MIN_TICK = 205;
    static constexpr uint16_t SERVO_MAX_TICK = 410;

    void setServoAngle(uint8_t id, float angle);
    uint16_t angleToTicks(float angle);
};

