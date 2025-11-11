#pragma once
#include <cmath>
#include <array>
#include "esp_log.h"

// Clamp function inline
inline double clamp(double x, double a=-1.0, double b=1.0) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

// Inverse kinematics for one leg
std::array<double,3> ik_leg(const std::array<double,3>& target,
                            double leg_rotation = 0.0,
                            const std::array<double,3>& leg_base = {0.0,0.0,0.0},
                            double coxa = 60.0,
                            double tl = 76.84,
                            double fl = 128.05,
                            double offset_foot_angle = 0.15708);
