#include "sample_data.hpp"


SampleData::SampleData() 
    : angle(0.0f) {
        mutex_buffer = xSemaphoreCreateMutex();
        if (mutex_buffer == NULL) {
            ESP_LOGE("SampleData", "Erreur: échec création mutex_buffer !");
            abort();
        }

        mutex_angle = xSemaphoreCreateMutex();
        if (mutex_angle == NULL) {
            ESP_LOGE("SampleData", "Erreur: échec création mutex_angle !");
            abort();
        }

        bufferL.clear();
        bufferR.clear();
}

float SampleData::getAngle() {
    float val = 0.0f;
    if (mutex_angle && xSemaphoreTake(mutex_angle, portMAX_DELAY) == pdTRUE) {
        val = angle;
        xSemaphoreGive(mutex_angle);
    }
    return val;
}

void SampleData::registerAngle() {
    std::vector<float> localL, localR;

    if (mutex_buffer && xSemaphoreTake(mutex_buffer, portMAX_DELAY) == pdTRUE) {
        localL = bufferL;
        localR = bufferR;
        xSemaphoreGive(mutex_buffer);
    }

    if (!localL.empty() && !localR.empty()) {
        int angleCalc = calculateAngle(localL, localR);
        setAngle(angleCalc);
    }
}



///////////////// Private ////////////////////


// Cross-correlation and angle calculation
int SampleData::calculateAngle(const std::vector<float>& sigL, const std::vector<float>& sigR) {
    const int maxLag = 88; 
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

    ESP_LOGI("I2S_TASK", "Angle: %.2f degrees", angle);
    return angle;
}


void SampleData::setAngle(float newAngle) {
    if (mutex_angle && xSemaphoreTake(mutex_angle, portMAX_DELAY) == pdTRUE) {
        angle = newAngle;
        xSemaphoreGive(mutex_angle);
    }
}





