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

#include "biquad_filter.hpp"
#include "i2s_task.hpp"
#include "com_task.hpp"
#include "sample_data.hpp"



/////////////// VARIABLE I2S ///////////////
// Init window for cross-correlation
std::vector<float> windowL;
std::vector<float> windowR;
////////////////////////////////////////////

///////////// VARIABLE BIQUAD //////////////
BiquadFilter bpFilterL;
BiquadFilter bpFilterR;
////////////////////////////////////////////

///////// VARIABLE COMMUNICATION ////////////
WebSocketServer wsServer(80);
SampleData sample_data; 
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

    // Init Data 
    sample_data.init_value();

    // Init Wifi + WS
    wsServer.init_wifi_softap("ESP_Spider", "12345678");
    wsServer.init_ws();
    
    // Init I2S
    init_i2s();

    // Set filtre
    bpFilterL.setupBandpass(500.0f, 1000.0f, 44100.0f);
    bpFilterR.setupBandpass(500.0f, 1000.0f, 44100.0f);

    // Task
    xTaskCreate(WebSocketServer::sendTask, "ws_send_task", 4096, &wsServer, 5, nullptr);
    xTaskCreate(i2s_task, "I2S_Task", 4096, NULL, 5, NULL);
    //xTaskCreate(com_task, "Com_Task", 4096, NULL, 1, NULL);

    vTaskDelete(NULL);

    /////////////////////////////////////////////////////////////
    
}


