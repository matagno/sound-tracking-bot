#include "i2s_task.hpp"

#include "esp_log.h"
#include "driver/i2s.h"
#include "BiquadFilter.hpp"
#include <vector>
#include <cstdio>

#define SAMPLE_RATE 44100
#define MAX_BUFFER 441

#define WINDOW_SIZE 512
#define MIC_DISTANCE 0.10
std::vector<float> windowL;
std::vector<float> windowR;

void i2s_task(void* arg) {
    for(;;) {
        int32_t samples[2];  // [0] = gauche, [1] = droite
        size_t bytes_read;
        i2s_read(I2S_NUM_0, samples, sizeof(samples), &bytes_read, portMAX_DELAY);

        if (bytes_read == sizeof(samples)) {
            float left  = float(samples[0] >> 8) / 8388608.0f;
            float right = float(samples[1] >> 8) / 8388608.0f;
            
            float filtered_left = bpFilterL.process(left);
            float filtered_right = bpFilterR.process(right);

            // CALCUL ANGLE
            windowL.push_back(filtered_left);
            windowR.push_back(filtered_right);
            if (windowL.size() >= WINDOW_SIZE && windowR.size() >= WINDOW_SIZE)
            {
                // Cross-correlation
                int maxLag = 50; 
                int bestLag = 0;
                float maxCorr = -1e9;

                for (int lag = -maxLag; lag <= maxLag; lag++) {
                    float corr = 0;
                    for (size_t i = 0; i < WINDOW_SIZE; i++) {
                        int j = i + lag;
                        if (j >= 0 && j < WINDOW_SIZE) {
                            corr += windowL[i] * windowR[j];
                        }
                    }
                    if (corr > maxCorr) {
                        maxCorr = corr;
                        bestLag = lag;
                    }
                }

                // Calcul de l'angle en degrés
                float timeDelay = float(bestLag) / SAMPLE_RATE; // en secondes
                float angle = asin(timeDelay * 343.0f / MIC_DISTANCE) * (180.0f / M_PI); // en degrés
                ESP_LOGI("I2S_TASK", "Angle: %.2f degrees", angle);

                windowL.clear();
                windowR.clear();
            }

            // COM
            static int counter = 0;
            if(xSemaphoreTake(bufferMutex, portMAX_DELAY) == pdTRUE) {
                if (++counter >= SAMPLE_RATE/1) {  
                    bufferR.push_back(filtered_right);
                    bufferL.push_back(filtered_left);
                    counter = 0;
                }
                // Securité overflow
                if (bufferR.size() > MAX_BUFFER || bufferL.size() > MAX_BUFFER) {
                    bufferR.clear();
                    bufferL.clear();
                    ESP_LOGW("I2S_TASK", "BufferL overflow, trimming buffer");
                }
                xSemaphoreGive(bufferMutex);
            }
        }
    }
}

