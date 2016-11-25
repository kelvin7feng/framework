//
//  data_manager.hpp
//  server
//
//  Created by 冯文斌 on 16/11/14.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef data_manager_hpp
#define data_manager_hpp

#include <stdio.h>

#include "redis_client.hpp"
#include "mysql_client.h"
#include "Statement.h"

using namespace RedisWrap;
using namespace MySQLWrap;

class DataManager
{
public:
    DataManager();
    ~DataManager();
    DataManager(const DataManager& dm);
    static DataManager* GetInstance();
    
    bool IsUserCreated(unsigned int user_id);
private:
    RedisClient _redis_client;
    MySQLClient _mysql_client;
};

#endif /* data_manager_hpp */
