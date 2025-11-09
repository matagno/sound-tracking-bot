#include <cstdio>
#include <cinttypes>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"


#include "bot/utils/pca9685.hpp"
#include "bot/bot_ctrl.hpp"

#include "sound/i2s_sound_acquisition.hpp"
#include "sound/utils/biquad_filter.hpp"
#include "sound/st_sample_data.hpp"

#include "ws_com/web_socket_server.hpp"
#include "ws_com/st_cmd_data.hpp"



//////////////////////////////////

/*          Data In             */
// Sample data
SampleData stSample_data; 
// HMI data
CmdData stCmd_data;

/*          Robot Object        */
PCA9685 objPca;
BotCtrl objBot_ctrl(stCmd_data, stSample_data, objPca);

/*          Sound Object        */
// Biquad filter
BiquadFilter objBiquad_filterL;
BiquadFilter objBiquad_filterR;
// I2s handling
I2sSoundAcquisition objI2s_sound_scquisition(objBiquad_filterR, objBiquad_filterL, stSample_data);

/*     Communication Object     */
WebSocketServer objWS_server(objBot_ctrl);

//////////////////////////////////





/*          Sound Task              */
void sound_task(void* arg) {
    for(;;) {
        objI2s_sound_scquisition.i2s_acquisition();
        taskYIELD();
    }
}


/*          Main Cycle Task         */  

void cycle_task(void* arg) {
    const int freq_cycle = 10; // per sec
    int cycle_count = 0;

    for(;;) {
        /*      Input Data processing       */
        if(xSemaphoreTake(objBot_ctrl.mutexData_transmit, portMAX_DELAY) == pdTRUE) {
            if (cycle_count == freq_cycle * 1){
                objBot_ctrl.update_move_mode();
            }
            if (cycle_count == freq_cycle * 5){
                objBot_ctrl.update_sound_angle();
            }
            xSemaphoreGive(objBot_ctrl.mutexData_transmit);
        }

        /*          Move Choice            */
        if (objBot_ctrl.xAuto && !objBot_ctrl.xManu && !objBot_ctrl.xTeleop) {
            if (objBot_ctrl.fAngle != 9999.0f) {
                // If angle between -20 and 20 degres : go forward
                // Else function to set qTarget based on fAngle
            }
        }
        if (objBot_ctrl.xTeleop && !objBot_ctrl.xAuto && !objBot_ctrl.xManu) {
            if (objBot_ctrl.xTeleop_run || objBot_ctrl.xTeleop_turn) {
                objBot_ctrl.qActive.fill(true);
            }
        }

        /*      Output Update       */
        objBot_ctrl.update_servos();

        // Reset count every minute
        cycle_count = (cycle_count + 1) % (freq_cycle * 60 + 1);
        vTaskDelay(pdMS_TO_TICKS(1000 / freq_cycle));
    }
}











extern "C" void app_main(void) {

    /////////////////////////   INFO CHIP   /////////////////////////
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s)\n", CONFIG_IDF_TARGET, chip_info.cores);

    if(esp_flash_get_size(nullptr, &flash_size) == ESP_OK) {
        printf("%" PRIu32 " MB flash\n", flash_size / (1024 * 1024));
    }

    /////////////////////////  START CYCLE  /////////////////////////

    // Init Comunication
    objWS_server.init_wifi_softap("ESP_Spider", "12345678");
    objWS_server.init_ws(80);
    
    // Init Sound
    objBiquad_filterL.setup_bandpass(500.0f, 1000.0f, 44100.0f);
    objBiquad_filterR.setup_bandpass(500.0f, 1000.0f, 44100.0f);
    objI2s_sound_scquisition.init_i2s();

    // Init Bot ctrl
    objPca.init_pca();
    objBot_ctrl.init_value();

    // Task
    xTaskCreate(sound_task, "I2S_Task", 4096, NULL, 5, NULL);
    xTaskCreate(cycle_task, "Cycle_Task", 4096, NULL, 4, NULL);

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
Resultat entre -90 et 90 degres
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

