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

#include "file_util.h"
#include "db_client_manager.hpp"
#include "game_logic_server.hpp"

int main() {
    
    g_pFileUtil = new FileUtil;
    g_pDBClientMgr = new KDBClientMgr;
    
    uv_loop_t *loop = uv_default_loop();
    GameLogicServer *server = GameLogicServer::GetInstance();
    server->Init(loop);
    
    return uv_run(loop, UV_RUN_DEFAULT);
}