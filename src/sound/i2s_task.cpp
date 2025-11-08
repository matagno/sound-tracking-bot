#include "i2s_task.hpp"

#include "esp_log.h"
#include "driver/i2s.h"
#include <vector>
#include <cstdio>

// I2S
#define I2S_NUM I2S_NUM_0
// For cross-correlation 
#define WINDOW_SIZE 441


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



