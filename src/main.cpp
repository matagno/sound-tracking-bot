#include <cstdio>
#include <cinttypes>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "driver/i2s.h"

#include "BiquadFilter.hpp"

#define SAMPLE_RATE 44100
#define I2S_NUM I2S_NUM_0


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

    printf("Test INMP441 I2S\n");

    /////////////////////////////////   START I2S   //////////////////////////////////////////
    // Configuration I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
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

    // Installer I2S
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, nullptr);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    // Fréquence d'échantillonnage
    i2s_set_sample_rates(I2S_NUM, SAMPLE_RATE);
    /////////////////////////////////////////////////////////////////////////////////////////

    /////////////       Passe Bande         //////////////
    BiquadFilter bpFilter;
    bpFilter.setupBandpass(100.0f, 500.0f, 44100.0f);


    ///////////////////////// CYCLE ////////////////////////////

    for(;;) {
        int32_t sample;
        size_t bytes_read;
        i2s_read(I2S_NUM_0, &sample, sizeof(sample), &bytes_read, portMAX_DELAY);

        float norm = float(sample >> 8) / 8388608.0f;
        float filtered = bpFilter.process(norm);

        // Afficher 10 échantillon par seconde
        static int counter = 0;
        if (++counter >= SAMPLE_RATE/10) {  
            printf("Sample: %f\n", filtered);
            counter = 0;
        }
    }
    // vTaskDelay(100 / portTICK_PERIOD_MS); 
    // esp_restart();
}


