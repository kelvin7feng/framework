//
//  config_cache.hpp
//  server
//
//  Created by 冯文斌 on 17/1/5.
//  Copyright © 2017年 kelvin. All rights reserved.
//

#ifndef config_cache_hpp
#define config_cache_hpp

#include <iostream>
#include <string>

#include "document.h"

class ConfigCache
{
public:
    ConfigCache();
    ~ConfigCache();
    
    bool LoadConfig();

    std::string GetListenIp();
    int GetListenPort();
    
private:
    rapidjson::Document json_doc;
    std::string m_sz_config;
};

extern ConfigCache* g_pConfigCache;

#endif /* config_cache_hpp */
