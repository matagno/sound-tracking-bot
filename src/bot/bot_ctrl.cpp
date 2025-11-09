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
}


/////// Move Methods ///////

void BotCtrl::update_move_mode() {
    if (xSemaphoreTake(adrCmd_data.mutexAll, portMAX_DELAY) == pdTRUE) {
        xAuto = adrCmd_data.xAuto;
        xManu = adrCmd_data.xManu;
        xTeleop = adrCmd_data.xTeleop;

        if (xManu && !xAuto && !xTeleop) {
            qActive = adrCmd_data.qActive_manual;
            qTarget = adrCmd_data.qTarget_manual;
            adrCmd_data.qActive_manual.fill(false);
        }
        if (xTeleop && !xManu && !xAuto) {
            xTeleop_run = adrCmd_data.xTeleop_run;
            xTeleop_turn = adrCmd_data.xTeleop_turn;
            adrCmd_data.xTeleop_turn = false;
        }

        xSemaphoreGive(adrCmd_data.mutexAll);
    }
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

    if (xSemaphoreTake(adrSample_data.mutexAll, 0) == pdTRUE) {
        localL = adrSample_data.vecSamplesL;
        localR = adrSample_data.vecSamplesR;
        xSemaphoreGive(adrSample_data.mutexAll);
    }

    // Calcul
    if (!localL.empty() && !localR.empty()) {
        fAngle = calculate_angle(localL, localR);
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
    if (maxCorr < 0.0001f) {
        return 9999; 
    }


    // Calculate angle in degrees
    float timeDelay = float(bestLag) / SAMPLE_RATE;
    float angle = asin(timeDelay * 343.0f / MIC_DISTANCE) * (180.0f / M_PI);
    //ESP_LOGI("I2S_TASK", "Angle: %.2f degrees", angle);

    return angle;
}


