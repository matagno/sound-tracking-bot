#include "com_task.hpp"
#include <cstring>
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "com_task.hpp"
#include "esp_log.h"
#include "esp_wifi.h"


WebSocketServer::WebSocketServer(int p) 
    : port(p), server(nullptr) {}


//////////////////////////////// PUBLIC ///////////////////////////


// Init SoftAP
void WebSocketServer::init_wifi_softap(const std::string& ssid, const std::string& pass) {
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
void WebSocketServer::init_ws() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    httpd_start(&server, &config);

    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = [](httpd_req_t* req) -> esp_err_t {
            WebSocketServer* obj = (WebSocketServer*)req->user_ctx;
            return obj->wsHandler(req);
        },
        .user_ctx = this,
        .is_websocket = true
    };
    httpd_register_uri_handler(server, &ws_uri);

    ESP_LOGI("WS_Server", "WebSocket server started on port %d", port);
}


// Loop
void WebSocketServer::sendTask(void* param) {
    WebSocketServer* obj = (WebSocketServer*)param;
    while (true) {
        obj->sendMessage("Hello Client!"); 
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}






/////////////////////////// PRIVATE ///////////////////////////



esp_err_t WebSocketServer::wsHandler(httpd_req_t* req) {

    // Connect
    if (req->method == HTTP_GET) {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.push_back(req);
        ESP_LOGI("WS_Server", "Client connected");
        return ESP_OK;
    }

    // Recieve
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(ws_pkt));
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 1024);
    if (ret == ESP_OK && ws_pkt.len > 0) {
        std::string msg((char*)ws_pkt.payload, ws_pkt.len);
        onMessageReceived(msg);
    } 
    
    // Disconnect
    if (ret != ESP_OK) {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), req), clients.end());
        ESP_LOGI("WS_Server", "Client disconnected");
    }

    return ESP_OK;
}

void WebSocketServer::onMessageReceived(const std::string& msg) {
    ESP_LOGI("WS_Server", "Message received: %s", msg.c_str());
}

void WebSocketServer::sendMessage(const std::string& msg) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto* client : clients) {
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(ws_pkt));
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;
        ws_pkt.payload = (uint8_t*)msg.c_str();
        ws_pkt.len = msg.size();
        httpd_ws_send_frame(client, &ws_pkt);
    }
}



