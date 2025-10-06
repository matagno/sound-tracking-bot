#include "com_task.hpp"

#include "esp_log.h"
#include <cstdio>
#include <vector>



void com_task(void* arg) {
    const size_t BLOCK_SIZE = 10;
    float last_reported_angle = 0.0f;
    SampleData local_copy;

    for(;;) {
        local_copy.bufferR.clear();
        local_copy.bufferL.clear();
        local_copy.angle = 999.0f;

        ////////////////////////////// COPY DATA //////////////////////////////
        if (xSemaphoreTake(sample_data.mutex, portMAX_DELAY) == pdTRUE) {
            // Angle copy
            if (last_reported_angle != sample_data.angle) {
                local_copy.angle = sample_data.angle;
                last_reported_angle = sample_data.angle;
            }

            // Buffer copy
            if (sample_data.bufferL.size() >= BLOCK_SIZE) {
                local_copy.bufferL.assign(sample_data.bufferL.begin(), sample_data.bufferL.begin() + BLOCK_SIZE);
                sample_data.bufferL.erase(sample_data.bufferL.begin(), sample_data.bufferL.begin() + BLOCK_SIZE);
            }
            if (sample_data.bufferR.size() >= BLOCK_SIZE) {
                local_copy.bufferR.assign(sample_data.bufferR.begin(), sample_data.bufferR.begin() + BLOCK_SIZE);
                sample_data.bufferR.erase(sample_data.bufferR.begin(), sample_data.bufferR.begin() + BLOCK_SIZE);
            }

            xSemaphoreGive(sample_data.mutex);
        }

        ///////////////////////////// PRINT DATA //////////////////////////////
        // Print buffer 
        if (!local_copy.bufferL.empty() && !local_copy.bufferR.empty()) {
            ESP_LOGI("COM_TASK", "New Data");
            for (size_t i = 0; i < local_copy.bufferL.size(); i++) {
                ESP_LOGI("COM_TASK", "Sample Right: %f", local_copy.bufferR[i]);
                ESP_LOGI("COM_TASK", "Sample Left: %f", local_copy.bufferL[i]);
            }
        }
        // Print angle 
        if (local_copy.angle != 999.0f) {
            ESP_LOGI("COM_TASK", "New Angle: %f degrees", local_copy.angle);
        }


        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

