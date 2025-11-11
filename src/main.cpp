#include <cstdio>
#include <cinttypes>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_timer.h"


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
        if (cycle_count % (freq_cycle * 1) == 0){
            objBot_ctrl.update_move_mode();
        }
        if (cycle_count % (freq_cycle * 1) == 0){
            objBot_ctrl.update_sound_angle();
        }


        /*          Move Choice            */
        float t = esp_timer_get_time() / 1000000.0f;
        if (!objBot_ctrl.xAuto && !objBot_ctrl.xManu && !objBot_ctrl.xTeleop) {
            //float x = 0, y =  136.840, z = -128.05;
            float x = 110, y = 110, z = -110;
            
            // ARD
            auto hip_knee_foot_1 = ik_leg({x, y, z}, 0.0);
            objBot_ctrl.qTarget[0] = static_cast<float>(-(hip_knee_foot_1[0]* (180.0 / M_PI))+90.0);
            objBot_ctrl.qTarget[1] = static_cast<float>(90-(hip_knee_foot_1[1]* (180.0 / M_PI)));
            objBot_ctrl.qTarget[2] = static_cast<float>(-(hip_knee_foot_1[2]* (180.0 / M_PI)));
            //ESP_LOGI("CMD", "Leg 1 at %f, %f, %f", hip_knee_foot_1[0], hip_knee_foot_1[1], hip_knee_foot_1[2]);
            // AVG
            auto hip_knee_foot_2 = ik_leg({x, y, z}, 0.0);
            objBot_ctrl.qTarget[3] = static_cast<float>(-(hip_knee_foot_2[0]* (180.0 / M_PI))+90.0);
            objBot_ctrl.qTarget[4] = static_cast<float>(90-(hip_knee_foot_2[1]* (180.0 / M_PI)));
            objBot_ctrl.qTarget[5] = static_cast<float>(-(hip_knee_foot_2[2]* (180.0 / M_PI)));
            //ESP_LOGI("CMD", "Leg 2 at %f, %f, %f", hip_knee_foot_2[0], hip_knee_foot_2[1], hip_knee_foot_2[2]);
            // AVD
            auto hip_knee_foot_3 = ik_leg({x, y, z}, 0.0);
            objBot_ctrl.qTarget[6] = static_cast<float>((hip_knee_foot_3[0]* (180.0 / M_PI))+90.0);
            objBot_ctrl.qTarget[7] = static_cast<float>(90-(hip_knee_foot_3[1]* (180.0 / M_PI)));
            objBot_ctrl.qTarget[8] = static_cast<float>(-(hip_knee_foot_3[2]* (180.0 / M_PI)));
            //ESP_LOGI("CMD", "Leg 3 at %f, %f, %f", hip_knee_foot_3[0], hip_knee_foot_3[1], hip_knee_foot_3[2]);
            // ARG
            auto hip_knee_foot_4 = ik_leg({x, y, z}, 0.0);
            objBot_ctrl.qTarget[9] = static_cast<float>((hip_knee_foot_4[0]* (180.0 / M_PI))+90.0);
            objBot_ctrl.qTarget[10] = static_cast<float>(90-(hip_knee_foot_4[1]* (180.0 / M_PI)));
            objBot_ctrl.qTarget[11] = static_cast<float>(-(hip_knee_foot_4[2]* (180.0 / M_PI)));
            //ESP_LOGI("CMD", "Leg 4 at %f, %f, %f", hip_knee_foot_4[0], hip_knee_foot_4[1], hip_knee_foot_4[2]);
        }
        if (objBot_ctrl.xAuto && !objBot_ctrl.xManu && !objBot_ctrl.xTeleop) {
            if (objBot_ctrl.fAngle != 9999.0f) {
                if (objBot_ctrl.fAngle < 20.0f && objBot_ctrl.fAngle > -20.0f && !objBot_ctrl.turn_in_progress) {
                    objBot_ctrl.autonomous_move(t, 0.0f, true, false);
                }else{
                    objBot_ctrl.autonomous_move(t, objBot_ctrl.fAngle * M_PI / 180, false, true);
                }
            }
        }
        if (objBot_ctrl.xTeleop && !objBot_ctrl.xAuto && !objBot_ctrl.xManu) {
            if (objBot_ctrl.xTeleop_run) {
                objBot_ctrl.autonomous_move(t, 0.0f, true, false);
            }
            else if (objBot_ctrl.xTeleop_turn || objBot_ctrl.turn_in_progress) {
                objBot_ctrl.autonomous_move(t, objBot_ctrl.fTeleop_angle * M_PI / 180, false, true);
                ESP_LOGI("Cmd", "Turn in progress");
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

    vTaskDelay(pdMS_TO_TICKS(10000));

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
    stCmd_data.init_value();
    
    // Init Sound
    objBiquad_filterL.setup_bandpass(1000.0f, 1200.0f, 44100.0f);
    objBiquad_filterR.setup_bandpass(1000.0f, 1200.0f, 44100.0f);
    objI2s_sound_scquisition.init_i2s();
    stSample_data.init_value();

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
Test with wscat : wscat -c ws://192.168.4.1:80/ws
*/
