#pragma once
#include <vector>
#include <string>
#include "esp_http_server.h"
#include "freertos/semphr.h"
// Data Receives
#include "st_cmd_data.hpp"
// Data Sends
#include "bot/bot_ctrl.hpp"
//

class WebSocketServer {
public:
    WebSocketServer(BotCtrl &bot_ctrl);

    // Init functions
    void init_wifi_softap(const std::string& ssid, const std::string& pass);
    void init_ws(int port = 80);

private:
    // Ptr
    BotCtrl &adrBot_ctrl;
    CmdData &adrCmd_Data;

    // Attributes
    httpd_handle_t _server;
    uint8_t bufRead_ws[1024];

    // Methods 
    void on_message_received(const char* msg, httpd_req_t* req);
    void send_message(const char* msg, httpd_req_t* client);
    esp_err_t ws_handler(httpd_req_t* req);

    void setBoolCommand(const char* msg, const char* prefix, bool& target);
};

