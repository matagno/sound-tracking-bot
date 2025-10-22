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

#include "task_class/i2s_task.hpp"
#include "task_class/com_task.hpp"
#include "utils/biquad_filter.hpp"
#include "utils/sample_data.hpp"



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





void angle_task(void* arg) {
    for(;;) {
        register_angle();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




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
    xTaskCreate(i2s_task, "I2S_Task", 4096, NULL, 5, NULL);
    xTaskCreate(angle_task, "Angle_Task", 4096, NULL, 4, NULL);

    vTaskDelete(NULL);

    /////////////////////////////////////////////////////////////
    
}



/*      Note        */
/*
Client websocket :
With wscat : wscat -c ws://192.168.4.1:80/ws
ping > pong
angle > angle value
*/
/*
Angle calculation :
Rsultat entre -90 et 90 degres
9999 = pas de son detecte
*/
/*
Regle mouvement robot :
Si angle entre -20 et 20 degres : avancer tout droit
Si angle entre 20 et 90 degres : tourner a droite de angle degres puis reverifier angle
Si angle entre -20 et -90 degres : tourner a gauche de angle degres puis reverifier angle
Pas de probleme de direction car on reverifie l'angle apres chaque rotation
9999 = pas de son detecte
*/

