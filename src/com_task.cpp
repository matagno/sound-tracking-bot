#include "com_task.hpp"

#include "esp_log.h"
#include <cstdio>
#include <vector>

// Nombre d'échantillons
const size_t BLOCK_SIZE = 1;

void com_task(void* arg) {
    std::vector<float> localBufferR;
    std::vector<float> localBufferL;

    for(;;) {
        // Copier un bloc du buffer partagé dans un buffer local
        if (xSemaphoreTake(bufferMutex, portMAX_DELAY) == pdTRUE) {
            if (bufferL.size() >= BLOCK_SIZE) {
                localBufferL.assign(bufferL.begin(), bufferL.begin() + BLOCK_SIZE);
                bufferL.erase(bufferL.begin(), bufferL.begin() + BLOCK_SIZE);
            }
            if (bufferR.size() >= BLOCK_SIZE) {
                localBufferR.assign(bufferR.begin(), bufferR.begin() + BLOCK_SIZE);
                bufferR.erase(bufferR.begin(), bufferR.begin() + BLOCK_SIZE);
            }
            xSemaphoreGive(bufferMutex);
        }

        // Envoyer le bloc sur UART
        if (!localBufferL.empty()) {
            ESP_LOGI("COM_TASK", "New Data");
            for (size_t i = 0; i < localBufferL.size(); i++) {
                ESP_LOGI("COM_TASK", "Sample Right: %f", localBufferR[i]);
                ESP_LOGI("COM_TASK", "Sample Left: %f", localBufferL[i]);
            }

            localBufferR.clear();
            localBufferL.clear();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

