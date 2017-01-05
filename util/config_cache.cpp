//
//  config_cache.cpp
//  server
//
//  Created by 冯文斌 on 17/1/5.
//  Copyright © 2017年 kelvin. All rights reserved.
//

#include <iostream>
#include <string>
#include "file_util.h"
#include "config_cache.hpp"

using namespace std;
using namespace rapidjson;

ConfigCache* g_pConfigCache = NULL;

ConfigCache::ConfigCache()
{
    
}

bool ConfigCache::LoadConfig()
{
    bool is_ok = g_pFileUtil->ReadFile("game_logic.json", m_sz_config);
    if(!is_ok)
    {
        std::cout << "read config failed." << std::endl;
        goto Exit0;
    }
    
    json_doc.Parse(m_sz_config.c_str());
    
Exit0:
    return is_ok;
}

std::string ConfigCache::GetListenIp()
{
    const Value& server_config = json_doc["listen"];
    std::string sz_ip = server_config["ip"].GetString();
    return sz_ip;
}
