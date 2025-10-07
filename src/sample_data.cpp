#include "sample_data.hpp"
#include "esp_log.h"

void SampleData::init_value(bool wMutex) {
    if (wMutex){
        mutex = xSemaphoreCreateMutex();
        if (mutex == NULL) {
            ESP_LOGE("INIT", "Erreur: échec de création du data_mutex !");
            abort();
        }
    }
    angle = 0.0f;
    bufferL.clear();
    bufferR.clear();
}
