//
//  main.cpp
//  server
//
//  Created by 冯文斌 on 16/10/10.
//  Copyright © 2016年 kelvin. All rights reserved.
//


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "document.h"
#include "file_util.h"
#include "gateway_server.hpp"
#include "gateway_client.hpp"

using namespace std;
using namespace rapidjson;

int main() {
    string sz_config;
    FileUtil* file_util = FileUtil::GetInstance();
    bool is_ok = file_util->ReadFile("gateway.json", sz_config);
    if(!is_ok)
    {
        cout << "read config failed." << endl;
        exit(1);
    }
    
    Document json_doc;
    json_doc.Parse(sz_config.c_str());
    
    //初始化网关
    const Value& server_config = json_doc["listen"];
    string gateway_ip = server_config["ip"].GetString();
    int gateway_port = server_config["port"].GetInt();
    
    uv_loop_t *loop = uv_default_loop();
    GatewayServer *server = GatewayServer::GetInstance();
    server->Init(loop, gateway_ip.c_str(), gateway_port);
    
    //连接逻辑服
    const Value& game_logic_config = json_doc["game_logic"];
    string game_loic_ip = game_logic_config["ip"].GetString();
    int game_logic_port = game_logic_config["port"].GetInt();
    
    GatewayClient* client = GatewayClient::GetInstance();
    client->Init(loop, game_loic_ip.c_str(), game_logic_port);
    
    return uv_run(loop, UV_RUN_DEFAULT);
}