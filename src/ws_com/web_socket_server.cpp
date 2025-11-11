#include "web_socket_server.hpp"
#include <cstring>
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

WebSocketServer::WebSocketServer(BotCtrl &bot_ctrl)
    : adrBot_ctrl(bot_ctrl), adrCmd_Data(bot_ctrl.adrCmd_data), _server(nullptr){}


//////////////////////////////// PUBLIC ///////////////////////////


// Init SoftAP
void WebSocketServer::init_wifi_softap(const std::string& ssid, const std::string& pass) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, ssid.c_str(), sizeof(wifi_config.ap.ssid));
    strncpy((char*)wifi_config.ap.password, pass.c_str(), sizeof(wifi_config.ap.password));
    wifi_config.ap.ssid_len = ssid.length();
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = pass.empty() ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("WS_Server", "SoftAP lancé : SSID=%s PASS=%s", ssid.c_str(), pass.c_str());
}


// Init WS
void WebSocketServer::init_ws(int port) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    httpd_start(&_server, &config);

    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = [](httpd_req_t* req) -> esp_err_t {
            WebSocketServer* obj = (WebSocketServer*)req->user_ctx;
            return obj->ws_handler(req);
        },
        .user_ctx = this,
        .is_websocket = true
    };
    httpd_register_uri_handler(_server, &ws_uri);

    ESP_LOGI("WS_Server", "WebSocket server started on port %d", port);
}




/////////////////////////// PRIVATE ///////////////////////////



esp_err_t WebSocketServer::ws_handler(httpd_req_t* req) {

    // Connect
    if (req->method == HTTP_GET) {
        ESP_LOGI("WS_Server", "Client connected");
        return ESP_OK;
    }

    // Receive
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(ws_pkt));
    ws_pkt.payload = (uint8_t*) bufRead_ws;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 1024);
    if (ret == ESP_OK && ws_pkt.len > 0) {
        char msg[128];
        size_t len = ws_pkt.len;
        if (len >= sizeof(msg)) len = sizeof(msg) - 1;
        memcpy(msg, ws_pkt.payload, len);
        msg[len] = '\0';

        on_message_received(msg, req);
    }
    
    // Disconnect
    if (ret != ESP_OK) {
        ESP_LOGI("WS_Server", "Client disconnected or error");
        return ESP_OK;
    }

    return ESP_OK;
}

void WebSocketServer::on_message_received(const char* msg, httpd_req_t* req) {

    if (strcmp(msg, "ping") == 0) {
        send_message("pong", req); 
    }

    // Send Info
    if (strcmp(msg, "get_angle") == 0) {
        char strAngle[32];
        if (xSemaphoreTake(adrBot_ctrl.mutexData_transmit, 0) == pdTRUE) {
            snprintf(strAngle, sizeof(strAngle), "%.2f", adrBot_ctrl.fAngle);
            xSemaphoreGive(adrBot_ctrl.mutexData_transmit);
        }
        ESP_LOGI("WS_Server", "Ask angle, respond : %s", strAngle);
        send_message(strAngle, req); 
    }

    // Recieve Info
    if (xSemaphoreTake(adrCmd_Data.mutexAll, portMAX_DELAY) == pdTRUE) {
        setBoolCommand(msg, "set_auto-", adrCmd_Data.xAuto);
        setBoolCommand(msg, "set_manual-", adrCmd_Data.xManu);
        setBoolCommand(msg, "set_teleop-", adrCmd_Data.xTeleop);
        setBoolCommand(msg, "set_teleop_run-", adrCmd_Data.xTeleop_run);
        setBoolCommand(msg, "set_teleop_turn-", adrCmd_Data.xTeleop_turn);
        xSemaphoreGive(adrCmd_Data.mutexAll);
    }

    if (strncmp(msg, "set_teleop_angle-", 17) == 0) {
        // set_teleop_angle-<valeur>
        float value = 0.0f;
        if (sscanf(msg + 17, "%f", &value) == 1) {
            if (xSemaphoreTake(adrCmd_Data.mutexAll, portMAX_DELAY) == pdTRUE) {
                adrCmd_Data.fTeleop_angle = value;
                ESP_LOGI("WS_Server", "Cmd received: set_teleop_angle %f", value);
                xSemaphoreGive(adrCmd_Data.mutexAll);
            }
        }
    }

    if (strncmp(msg, "set_qTarget-", 12) == 0) {
        // set_qTarget-<valeur>-<index>
        float value = 0.0f;
        int index = -1;
        if (sscanf(msg + 12, "%f-%d", &value, &index) == 2) {
            if (index >= 0 && index < 12) {
                if (xSemaphoreTake(adrCmd_Data.mutexAll, portMAX_DELAY) == pdTRUE) {
                    adrCmd_Data.qTarget_manual[index] = value;
                    ESP_LOGI("WS_Server", "Cmd received: set_qTarget %f at index %d", value, index);
                    xSemaphoreGive(adrCmd_Data.mutexAll);
                }
            }
        }
    }
    if (strncmp(msg, "set_qActive-", 12) == 0) {
        // set_qActive-<true/false>-<index>
        char state[8];
        int index = -1;
        if (sscanf(msg + 12, "%7[^-]-%d", state, &index) == 2) {
            for (int i = 0; state[i]; ++i)
                state[i] = tolower((unsigned char)state[i]);

            bool value = (strcmp(state, "true") == 0);
            if (index >= 0 && index < 12) {
                if (xSemaphoreTake(adrCmd_Data.mutexAll, portMAX_DELAY) == pdTRUE) {
                    adrCmd_Data.qActive_manual[index] = value;
                    ESP_LOGI("WS_Server", "Cmd received: index %d", index);
                    xSemaphoreGive(adrCmd_Data.mutexAll);
                }
            }
        }
    }
}


void WebSocketServer::setBoolCommand(const char* msg, const char* prefix, bool& target) {
    size_t len = strlen(prefix);
    if (strncmp(msg, prefix, len) == 0) {
        const char* val = msg + len;
        if (strcmp(val, "true") == 0) target = true;
        else if (strcmp(val, "false") == 0) target = false;
        ESP_LOGI("WS_Server", "Cmd received: %s = %s", prefix, target ? "true" : "false");
    }
}



void WebSocketServer::send_message(const char* msg, httpd_req_t* client) {
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(ws_pkt));
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;
        ws_pkt.payload = (uint8_t*)msg;
        ws_pkt.len = strlen(msg);
        httpd_ws_send_frame(client, &ws_pkt);
}



