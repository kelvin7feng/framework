//
//  main.cpp
//  server
//
//  Created by 冯文斌 on 16/10/11.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "document.h"
#include "file_util.h"
#include "game_logic_server.hpp"

using namespace std;
using namespace rapidjson;

int main() {
    
    string sz_config;
    FileUtil* file_util = FileUtil::GetInstance();
    bool is_ok = file_util->ReadFile("game_logic.json", sz_config);
    if(!is_ok)
    {
        cout << "read config failed." << endl;
        exit(1);
    }
    
    Document json_doc;
    json_doc.Parse(sz_config.c_str());
    
    const Value& server_config = json_doc["listen"];
    
    assert(server_config.HasMember("ip"));
    assert(server_config["ip"].IsString());
    
    assert(server_config.HasMember("port"));
    assert(server_config["port"].IsInt());
    
    string sz_ip = server_config["ip"].GetString();
    int port = server_config["port"].GetInt();
    
    uv_loop_t *loop = uv_default_loop();
    GameLogicServer *server = GameLogicServer::get_instance();
    server->init(loop, sz_ip.c_str(), port);
    
    return uv_run(loop, UV_RUN_DEFAULT);
}