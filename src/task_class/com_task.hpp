#pragma once
#include "esp_http_server.h"
#include <string>
#include <vector>
#include <mutex>

class WebSocketServer {
public:
    WebSocketServer(int port = 80);
    // Wifi
    void init_wifi_softap(const std::string& ssid, const std::string& pass);
    // WS
    void init_ws();
    // Loop
    static void sendTask(void* param);

private:
    int port;
    httpd_handle_t server;
    std::vector<httpd_req_t*> clients;
    std::mutex clientsMutex;    

    // Callback 
    void onMessageReceived(const std::string& msg);

    // Send
    void sendMessage(const std::string& msg);

    // Handler
    esp_err_t wsHandler(httpd_req_t* req);
};

