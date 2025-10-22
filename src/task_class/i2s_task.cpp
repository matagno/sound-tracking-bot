#include "i2s_task.hpp"

#include "esp_log.h"
#include "driver/i2s.h"
#include <vector>
#include <cstdio>

// I2S
#define SAMPLE_RATE 44100
#define I2S_NUM I2S_NUM_0
// For cross-correlation and angle calculation
#define WINDOW_SIZE 441
#define MIC_DISTANCE 0.10
// For com
#define MAX_BUFFER 100



void init_i2s() {
    // Configuration I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64
    };

    // Pins I2S
    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,    // Bit Clock
        .ws_io_num = 25,     // Word Select
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 22    // Data
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, nullptr);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_sample_rates(I2S_NUM, SAMPLE_RATE);
}




// Main I2S task
void i2s_task(void* arg) {
    for(;;) {
        int32_t samples[2];  // [0] = left, [1] = right
        size_t bytes_read;
        i2s_read(I2S_NUM_0, samples, sizeof(samples), &bytes_read, portMAX_DELAY);

        if (bytes_read == sizeof(samples)) {
            // Samples
            float left  = float(samples[0] >> 8) / 8388608.0f;
            float right = float(samples[1] >> 8) / 8388608.0f;
            
            // Filtred angle
            float filtered_left = bpFilterL.process(left);
            float filtered_right = bpFilterR.process(right);

            // Cross-correlation
            if (windowL.size() < WINDOW_SIZE && windowR.size() < WINDOW_SIZE) {
                windowL.push_back(filtered_left);
                windowR.push_back(filtered_right);
            } else {
                // Register      
                if(xSemaphoreTake(sample_data.mutex_buffer, portMAX_DELAY) == pdTRUE) {
                    sample_data.bufferR = windowR;
                    sample_data.bufferL = windowL;

                    xSemaphoreGive(sample_data.mutex_buffer);
                }

                // Clear
                windowL.clear();
                windowR.clear();
            }
        }
        taskYIELD();
    }
}


// Cross-correlation and angle calculation
int calculate_angle(const std::vector<float>& sigL, const std::vector<float>& sigR) {
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

    ESP_LOGI("I2S_TASK", "Angle: %.2f degrees", angle);
    return angle;
}



// Register angle
void register_angle() {
    static std::vector<float> localL;
    static std::vector<float> localR;

    if (xSemaphoreTake(sample_data.mutex_buffer, portMAX_DELAY) == pdTRUE) {
        localL = sample_data.bufferL;
        localR = sample_data.bufferR;
        xSemaphoreGive(sample_data.mutex_buffer);
    }

    // Calcul
    if (!localL.empty() && !localR.empty()) {
        int angle = calculate_angle(localL, localR);
        if (xSemaphoreTake(sample_data.mutex_angle, portMAX_DELAY) == pdTRUE) {
            sample_data.angle = angle;
            xSemaphoreGive(sample_data.mutex_angle);
        }
    }
}




