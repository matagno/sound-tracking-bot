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

#include "move_ik/pca9685.hpp"
#include "move_ik/servo_ctrl.hpp"
#include "sound/biquad_filter.hpp"
#include "sound/sample_data.hpp"
#include "sound/i2s_task.hpp"
#include "ws_com/com_handle.hpp"



/////////////// VARIABLE Sound /////////////
// Init window for cross-correlation in i2s_task
std::vector<float> windowL;
std::vector<float> windowR;
// Biquad filter
BiquadFilter bpFilterL;
BiquadFilter bpFilterR;
// Sample data
SampleData sample_data; 
////////////////////////////////////////////

///////// VARIABLE COMMUNICATION ///////////
WebSocketServer ws_server(80);
////////////////////////////////////////////

/////////  VARIABLE ROBOT MOVE  ////////////
// Objects
pca9685 pca;
servo_ctrl servo_controller(pca);
// Variables
std::array<float, 12> q_output;        
std::array<bool, 12> q_output_active;
// Remplacer par structure ou mettre dans servo_ctrl ?
// g_stBot_State
// With stCmd, fAngle_sound, qOutput, qOutput_active
// g_stBot_State.stCmd with xAuto, xManu, qManual_cmd, qManual_active
////////////////////////////////////////////















////////////////////// Main Cycle Task //////////////////////

void cycle_task(void* arg) {
    // Number of cycle each second 
    const int freq_cycle = 10; 
    // Count
    int cycle_count = 0;

    for(;;) {
        /////////// Input sound processing  ///////////
        if (cycle_count == freq_cycle * 1){
            // Mutex lock objCom_hmi.stCmd
            // g_stBot_State.stCmd = objCom_hmi.stCmd;
            // Mutex unlock objCom_hmi.stCmd
        }
        if (cycle_count == freq_cycle * 5){
            sample_data.registerAngle();
            // g_stBot_State.fAngle_sound = sample_data.getAngle();
        }

        ///////////         Processing       ///////////
        // Si Auto : Calcul q
        // fct auto_mode
        q_output = {90.0f, 45.0f, 90.0f, 135.0f, 90.0f, 45.0f, 90.0f, 135.0f, 90.0f, 45.0f, 90.0f, 135.0f};
        q_output_active = {true, true, true, true, true, true, true, true, true, true, true, true};
        // Si Manuel : Send q_manual_cmd
        // q = g_stBot_State.q_manual_cmd
        // q_active = g_stBot_State.q_manual_active;

        ///////////     Update Output       ///////////
        servo_controller.updateServos(q_output, q_output_active);


        ///////////         End Cycle      ///////////
        // Reset count every minute
        cycle_count = (cycle_count + 1) % (freq_cycle * 60 + 1);
        vTaskDelay(pdMS_TO_TICKS(1000 / freq_cycle));
    }
}

///////////////////////////////////////////////////////////////













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

    // Init Comunication
    ws_server.init_wifi_softap("ESP_Spider", "12345678");
    ws_server.init_ws();
    
    // Init Sound
    bpFilterL.setup_bandpass(500.0f, 1000.0f, 44100.0f);
    bpFilterR.setup_bandpass(500.0f, 1000.0f, 44100.0f);
    init_i2s();

    // Init Bot ctrl
    pca.init();

    // Task
    xTaskCreate(i2s_task, "I2S_Task", 4096, NULL, 5, NULL);
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

/*
Ameliorations structure code possibles :
- Transformer i2s_task en classe
- Mettre des pointeurs vers les classes BiquadFilter et SampleData dans i2s_task
- Uniformiser nom var et commentaires
*/


// Prochaine fois :
// Transformer servo_ctrl en robot_ctrl avec g_stBot_State dedans
// Ajouter methode mode auto et fonction cinématique
// Ajouter variable de l'ihm dans com_handle et le renommer
// Changer les noms de variables

