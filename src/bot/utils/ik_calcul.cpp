#include "ik_calcul.hpp"


std::array<double,3> ik_leg(const std::array<double,3>& target,
                            double leg_rotation,
                            const std::array<double,3>& leg_base,
                            double coxa,
                            double tl,
                            double fl,
                            double offset_foot_angle) {
        
    // Compute local target in leg frame
    std::array<double,3> local_target = { target[0]-leg_base[0],
                                          target[1]-leg_base[1],
                                          target[2]-leg_base[2] };

    // Rotation around Z axis
    double cosR = cos(-leg_rotation);
    double sinR = sin(-leg_rotation);
    double x = cosR * local_target[0] - sinR * local_target[1];
    double y = sinR * local_target[0] + cosR * local_target[1];
    double z = local_target[2];
    
    

    // Projection for knee and foot
    double horiz = hypot(x, y);
    double ex = horiz - coxa;
    double ez = z;
    double l = hypot(ex, ez);

    if (l > (tl + fl) || l < fabs(tl - fl)) {
        ESP_LOGW("IK", "Impossible to reach this position!");
        return {0.0, 0.0, 0.0};
    }

    // Hip angle
    double hip = M_PI_2 - atan2(y, x);

    // Foot angle
    double foot = acos(clamp((tl*tl + fl*fl - l*l) / (2.0 * tl * fl))) - M_PI + offset_foot_angle;

    // Knee angle
    double va = -atan2(ez, ex);
    double vb = acos(clamp((l*l + tl*tl - fl*fl) / (2.0 * l * tl)));
    double knee = va - vb;

    return {hip, knee, foot};
}


