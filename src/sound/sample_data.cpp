#include "sample_data.hpp"
#include "esp_log.h"

void SampleData::init_value(bool wMutex) {
    if (wMutex){
        mutex_buffer = xSemaphoreCreateMutex();
        if (mutex_buffer == NULL) {
            ESP_LOGE("INIT", "Erreur: échec de création du mutex_buffer !");
            abort();
        }
        mutex_angle = xSemaphoreCreateMutex();
        if (mutex_angle == NULL) {
            ESP_LOGE("INIT", "Erreur: échec de création du mutex_angle !");
            abort();
        }
    }
    angle = 0.0f;
    bufferL.clear();
    bufferR.clear();
}


