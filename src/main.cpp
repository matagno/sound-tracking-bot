#include <cstdio>
#include <cinttypes>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "driver/i2s.h"

#include "BiquadFilter.hpp"
#include "i2s_task.hpp"
#include "com_task.hpp"



/////////////// VARIABLE I2S ///////////////

#define SAMPLE_RATE 44100
#define I2S_NUM I2S_NUM_0

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

////////////////////////////////////////////

///////////// VARIABLE Biquad //////////////
BiquadFilter bpFilterL;
BiquadFilter bpFilterR;
////////////////////////////////////////////

///////////// VARIABLE Buffer //////////////
std::vector<float> bufferL;
std::vector<float> bufferR;
SemaphoreHandle_t bufferMutex;
////////////////////////////////////////////



extern "C" void app_main(void) {

    /////////////////////////////////   INFO CHIP   //////////////////////////////////////////
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s)\n", CONFIG_IDF_TARGET, chip_info.cores);

    if(esp_flash_get_size(nullptr, &flash_size) == ESP_OK) {
        printf("%" PRIu32 " MB flash\n", flash_size / (1024 * 1024));
    }
    /////////////////////////////////////////////////////////////////////////////////////////   

    ///////////////////////// CYCLE ////////////////////////////

    // Create Mutex 
    bufferMutex = xSemaphoreCreateMutex();
    if (bufferMutex == NULL) {
        printf("Erreur: échec de création du bufferMutex !\n");
        abort();
    }
    
    // Set config I2S
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, nullptr);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_sample_rates(I2S_NUM, SAMPLE_RATE);
    // Set filtre
    bpFilterL.setupBandpass(500.0f, 1000.0f, 44100.0f);
    bpFilterR.setupBandpass(500.0f, 1000.0f, 44100.0f);

    // TASK
    xTaskCreate(i2s_task, "I2S_Task", 4096, NULL, 5, NULL);
    xTaskCreate(com_task, "Com_Task", 4096, NULL, 1, NULL);

    vTaskDelete(NULL);

    /////////////////////////////////////////////////////////////

    // vTaskDelay(100 / portTICK_PERIOD_MS); 
    // esp_restart();
    
}


