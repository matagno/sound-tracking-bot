#pragma once
#include "esp_http_server.h"
#include "freertos/semphr.h"
#include "sound/sample_data.hpp"
#include <string>
#include <vector>


extern SampleData sample_data;

class WebSocketServer {
public:
    WebSocketServer(int port = 80);
    // Add Attributes for HMI variables

    // Init functions
    void init_wifi_softap(const std::string& ssid, const std::string& pass);
    void init_ws();

private:
    // Attributes
    int port;
    httpd_handle_t server;
    uint8_t readBufWS[1024];

    // Callback 
    void onMessageReceived(const std::string& msg);
    // Send
    void sendMessage(const std::string& msg, httpd_req_t* client);
    // Handler
    esp_err_t wsHandler(httpd_req_t* req);
};

