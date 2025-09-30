#include "com_task.hpp"

#include <cstdio>
#include <vector>

// Nombre d'échantillons
const size_t BLOCK_SIZE = 1;

void com_task(void* arg) {
    std::vector<float> localBuffer;

    for(;;) {
        // Copier un bloc du buffer partagé dans un buffer local
        if (xSemaphoreTake(bufferMutex, portMAX_DELAY) == pdTRUE) {
            if (buffer.size() >= BLOCK_SIZE) {
                localBuffer.assign(buffer.begin(), buffer.begin() + BLOCK_SIZE);
                buffer.erase(buffer.begin(), buffer.begin() + BLOCK_SIZE);
            }
            xSemaphoreGive(bufferMutex);
        }

        // Envoyer le bloc sur UART
        if (!localBuffer.empty()) {
            printf("Block: \n");
            for (size_t i = 0; i < localBuffer.size(); i++) {
                printf("Sample Right: %f\n", localBuffer[i]);
                //printf("Sample Left: %f\n", localBuffer[i]);
            }
            printf("\n");

            localBuffer.clear();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

