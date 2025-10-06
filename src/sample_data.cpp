#include "sample_data.hpp"
#include "esp_log.h"

void init_sample_data() {
    sample_data.mutex = xSemaphoreCreateMutex();
    if (sample_data.mutex == NULL) {
        ESP_LOGE("INIT", "Erreur: échec de création du data_mutex !");
        abort();
    }
    sample_data.angle = 0.0f;
    sample_data.bufferL.clear();
    sample_data.bufferR.clear();
}
