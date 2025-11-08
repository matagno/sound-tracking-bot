#include "servo_ctrl.hpp"
#include <algorithm>

servo_ctrl::servo_ctrl(PCA9685& driver) 
    : ptrPCA(driver) {}


void servo_ctrl::updateServos(const std::array<float, 12>& q_output, const std::array<bool, 12>& q_output_active) {
    for (int i = 0; i < 12; ++i) {
        if (q_output_active[i]) {
            setServoAngle(i, q_output[i]);
        }
    }
}

//////// Private methods ///////

void servo_ctrl::setServoAngle(uint8_t id, float angle) {
    uint16_t ticks = angleToTicks(angle);
    ptrPCA.setPWM(id, 0, ticks);
}

uint16_t servo_ctrl::angleToTicks(float angle) {
    angle = std::clamp(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    float ratio = (angle - SERVO_MIN_ANGLE) / (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE);
    return SERVO_MIN_TICK + static_cast<uint16_t>((SERVO_MAX_TICK - SERVO_MIN_TICK) * ratio);
}

