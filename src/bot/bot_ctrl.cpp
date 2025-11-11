#include "bot_ctrl.hpp"
#include <algorithm>

BotCtrl::BotCtrl(CmdData& cmd_data, SampleData& sample_data, PCA9685& driver) 
    : adrCmd_data(cmd_data), adrSample_data(sample_data), adrPCA(driver) {}


void BotCtrl::init_value() {
    fAngle = 9999.0f;
    xAuto = false;
    xManu = false;
    xTeleop = false;
    xTeleop_run = false;
    xTeleop_turn = false;
    qTarget.fill(0.0f);
    qActive.fill(false);
    mutexData_transmit = xSemaphoreCreateMutex();
}


/////// Move Methods ///////

void BotCtrl::update_move_mode() {
    //if (xSemaphoreTake(adrCmd_data.mutexAll, portMAX_DELAY) == pdTRUE) {
        xAuto = adrCmd_data.xAuto;
        xManu = adrCmd_data.xManu;
        xTeleop = adrCmd_data.xTeleop;

        if (xManu && !xAuto && !xTeleop) {
            qActive = adrCmd_data.qActive_manual;
            qTarget = adrCmd_data.qTarget_manual;
        }else{
            qActive.fill(true);
        }
        if (xTeleop && !xManu && !xAuto) {
            xTeleop_run = adrCmd_data.xTeleop_run;
            xTeleop_turn = adrCmd_data.xTeleop_turn;
            adrCmd_data.xTeleop_turn = false;
        }

        //xSemaphoreGive(adrCmd_data.mutexAll);
    //}
}


/////// Motors Methods ///////

void BotCtrl::set_servo_angle(uint8_t id, float angle) {
    angle = std::clamp(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    float ratio = (angle - SERVO_MIN_ANGLE) / (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE);
    uint16_t ticks = SERVO_MIN_TICK + static_cast<uint16_t>((SERVO_MAX_TICK - SERVO_MIN_TICK) * ratio);

    adrPCA.set_PWM(id, 0, ticks);
}

void BotCtrl::update_servos() {
    for (int i = 0; i < 12; ++i) {
        if (qActive[i]) {
            set_servo_angle(i, qTarget[i]);
        }
    }
}







//////// Sound processing Methods ///////

// Update angle
void BotCtrl::update_sound_angle() {
    static std::vector<float> localL;
    static std::vector<float> localR;

    if (xSemaphoreTake(adrSample_data.mutexAll, portMAX_DELAY) == pdTRUE) {
        localL = adrSample_data.vecSamplesL;
        localR = adrSample_data.vecSamplesR;
        xSemaphoreGive(adrSample_data.mutexAll);
    }

    // Calcul
    if (!localL.empty() && !localR.empty()) {
        fAngle = calculate_angle(localL, localR);
        fAngle = (fAngle > 180.0f || fAngle < -180.0f) ? 9999.0f : fAngle;
    }
}


// Cross-correlation and angle calculation
int BotCtrl::calculate_angle(const std::vector<float>& sigL, const std::vector<float>& sigR) {
    int maxLag = 88; 
    int bestLag = 0;
    float maxCorr = 0;
    

    for (int lag = -maxLag; lag <= maxLag; lag++) {
        float corr = 0;
        for (size_t i = 0; i < sigL.size(); i++) {
            int j = i + lag;
            if (j >= 0 && j < sigR.size()) {
                corr += sigL[i] * sigR[j];
            }
        }
        if (corr > maxCorr) {
            maxCorr = corr;
            bestLag = lag;
        }
    }

    // Check treshold, only consider significant sound detected
    if (maxCorr < 0.00001f) {
        return 9999; 
    }


    // Calculate angle in degrees
    float timeDelay = float(bestLag) / SAMPLE_RATE;
    float angle = asin(timeDelay * 343.0f / MIC_DISTANCE) * (180.0f / M_PI);
    ESP_LOGI("I2S_TASK", "Angle: %.2f degrees, Lag %f, Time %f", angle, float(bestLag), timeDelay);

    return angle;
}






void BotCtrl::autonomous_move(float t, float angle_target, bool run_cmd, bool turn_cmd) {
    bool run = run_cmd;
    bool turn = turn_cmd;

    const float step_length = 130.0f;
    const float step_height = 70.0f;
    const float step_height_rot = 45.0f;
    const float T = 2; //0.25f;
    const float T_rot = 0.5f;

    float phase_AVD_ARG = t;
    float phase_AVG_ARD = t + T / 2.0f;

    float x_std = 110.0f, y_std = 110.0f, z_std = -80.0f;

    float dx=0.0f, dy=0.0f, dz=0.0f;
    float x_leg_offset_std = 42.57f;
    float y_leg_offset_std = 48.75f;
    float t_rot = 0.0f;
    float angle_rot = 0.0f;

    if (turn) {
        if (!turn_last_cycle) {
            t0_rot = t;
            angle_to_turn = angle_target;
            turn_in_progress = true;
        }
        t_rot = t - t0_rot;
        float angle_rot_past = (angle_target > 0.0f) ? fminf(angle_to_turn, M_PI/8.0f)
                                                     : fmaxf(angle_to_turn, -M_PI/8.0f);
        z_std = -60.0f;
        if (t_rot >= T_rot) {
            angle_to_turn -= angle_rot_past;
            if (fabsf(angle_to_turn) > 0.001f) {
                t0_rot = t;
            }else{
                turn_in_progress = false;
            }
        }
        angle_rot = (angle_target > 0.0f) ? fminf(angle_to_turn, M_PI/8.0f)
                                          : fmaxf(angle_to_turn, -M_PI/8.0f);
    }

    // ---------------------------
    // AVD
    float x = x_std, y = y_std, z = z_std;
    if (run) {
        std::tie(dx, dy, dz) = foot_traj(phase_AVD_ARG, step_length, step_height, T);
        float z_eq = 20.0f;
        x += dx; y += dy; z += dz + z_eq;
    }
    if (turn) {
        float x_start_in_bot = x + x_leg_offset_std;
        float y_start_in_bot = y + y_leg_offset_std;
        std::tie(x, y, z) = rot_traj(t_rot, 1, {x_start_in_bot, y_start_in_bot, z}, angle_rot, step_height_rot, T_rot);
        x -= x_leg_offset_std;
        y -= y_leg_offset_std;
    }
    auto hip_knee_foot = ik_leg({x, y, z}, 0.0);
    qTarget[6] = static_cast<float>((hip_knee_foot[0]* (180.0 / M_PI))+90.0);
    qTarget[7] = static_cast<float>(90-(hip_knee_foot[1]* (180.0 / M_PI)));
    qTarget[8] = static_cast<float>(-(hip_knee_foot[2]* (180.0 / M_PI)));    

    // ARG
    x = x_std; y = y_std; z = z_std;
    if (run) {
        std::tie(dx, dy, dz) = foot_traj(phase_AVD_ARG, step_length, step_height, T);
        float z_eq = -30.0f;
        x += dx; y += dy; z += dz + z_eq;
    }
    if (turn) {
        float x_start_in_bot = x - x_leg_offset_std;
        float y_start_in_bot = y - y_leg_offset_std;
        std::tie(x, y, z) = rot_traj(t_rot, 2, {x_start_in_bot, y_start_in_bot, z}, angle_rot, step_height_rot, T_rot);
        x += x_leg_offset_std;
        y += y_leg_offset_std;
    }
    hip_knee_foot = ik_leg({x, y, z}, 0.0);
    qTarget[9] =  static_cast<float>((hip_knee_foot[0]* (180.0 / M_PI))+90.0);
    qTarget[10] = static_cast<float>(90-(hip_knee_foot[1]* (180.0 / M_PI)));
    qTarget[11] = static_cast<float>(-(hip_knee_foot[2]* (180.0 / M_PI)));    

    // AVG
    x = x_std; y = y_std; z = z_std;
    if (run) {
        std::tie(dx, dy, dz) = foot_traj(phase_AVG_ARD, step_length, step_height, T);
        float z_eq = 20.0f;
        x += dx; y += dy; z += dz + z_eq;
    }
    if (turn) {
        float x_start_in_bot = x + x_leg_offset_std;
        float y_start_in_bot = y - y_leg_offset_std;
        std::tie(x, y, z) = rot_traj(t_rot, 3, {x_start_in_bot, y_start_in_bot, z}, angle_rot, step_height_rot, T_rot);
        x -= x_leg_offset_std;
        y += y_leg_offset_std;
    }
    hip_knee_foot = ik_leg({x, y, z}, 0.0);
    qTarget[3] =  static_cast<float>(-(hip_knee_foot[0]* (180.0 / M_PI))+90.0);
    qTarget[4] =  static_cast<float>(90-(hip_knee_foot[1]* (180.0 / M_PI)));
    qTarget[5] =  static_cast<float>(-(hip_knee_foot[2]* (180.0 / M_PI)));    

    // ARD
    x = x_std; y = y_std; z = z_std;
    if (run) {
        std::tie(dx, dy, dz) = foot_traj(phase_AVG_ARD, step_length, step_height, T);
        float z_eq = -30.0f;
        x += dx; y += dy; z += dz + z_eq;
    }
    if (turn) {
        float x_start_in_bot = x - x_leg_offset_std;
        float y_start_in_bot = y + y_leg_offset_std;
        std::tie(x, y, z) = rot_traj(t_rot, 4, {x_start_in_bot, y_start_in_bot, z}, angle_rot, step_height_rot, T_rot);
        x += x_leg_offset_std;
        y -= y_leg_offset_std;
    }
    hip_knee_foot = ik_leg({x, y, z}, 0.0);
    qTarget[0] =  static_cast<float>(-(hip_knee_foot[0]* (180.0 / M_PI))+90.0);
    qTarget[1] =  static_cast<float>(90-(hip_knee_foot[1]* (180.0 / M_PI)));
    qTarget[2] =  static_cast<float>(-(hip_knee_foot[2]* (180.0 / M_PI)));    

    turn_last_cycle = turn;
}







std::tuple<float,float,float> BotCtrl::foot_traj(float t, float step_length, float step_height, float T) {
    float phase = fmodf(t, T) / T;
    float x=0.0f, y=0.0f, z=0.0f;
    if (phase < 0.5f) {
        x = - step_length * (phase - 0.25f);
        z = 0.0f;
    } else {
        x = step_length * (phase - 0.75f);
        z = step_height * sinf((phase - 0.5f) * M_PI * 2.0f);
    }
    return {x, 0.0f, z};
}





std::tuple<float,float,float> BotCtrl::rot_traj(float t, int phase_id, std::array<float,3> xyz_start_in_bot,
                                               float angle, float step_height, float T) {
    float phase = fminf(fmaxf(t / T, 0.0f), 1.0f);
    float start = (phase_id-1) / 4.0f;
    float end   = phase_id / 4.0f;
    float x=0.0f, y=0.0f, z=0.0f;

    if (phase < start) {
        x = xyz_start_in_bot[0];
        y = xyz_start_in_bot[1];
        z = xyz_start_in_bot[2];
    } else if (phase < end) {
        float local_phase = (phase - start) / (end - start);
        x = xyz_start_in_bot[0] * cosf(angle*local_phase) - xyz_start_in_bot[1] * sinf(angle*local_phase);
        y = xyz_start_in_bot[0] * sinf(angle*local_phase) + xyz_start_in_bot[1] * cosf(angle*local_phase);
        z = step_height * sinf(local_phase * M_PI);
    } else {
        x = xyz_start_in_bot[0] * cosf(angle) - xyz_start_in_bot[1] * sinf(angle);
        y = xyz_start_in_bot[0] * sinf(angle) + xyz_start_in_bot[1] * cosf(angle);
        z = xyz_start_in_bot[2];
    }
    return {x, y, z};
}



