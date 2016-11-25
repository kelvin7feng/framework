//
//  data_manager.cpp
//  server
//
//  Created by 冯文斌 on 16/11/14.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include "DatabaseException.h"
#include "data_manager.hpp"
#include "Nullable.h"
#include "document.h"
#include "file_util.h"

using namespace rapidjson;
DataManager::DataManager()
{
    string sz_config;
    FileUtil* file_util = FileUtil::GetInstance();
    bool is_ok = file_util->ReadFile("database.json", sz_config);
    if(!is_ok)
    {
        cout << "read config failed." << endl;
        exit(1);
    }
    
    Document json_doc;
    json_doc.Parse(sz_config.c_str());
    
    const Value& redis_config = json_doc["game_logic_redis"];
    string redis_ip = redis_config["ip"].GetString();
    int redis_port = redis_config["port"].GetInt();
    
    _redis_client = *new RedisClient(redis_ip,redis_port);
    int result = _redis_client.Connect();
    if(result == REDIS_OK)
    {
        std::cout << "connect redis succeed..." << std::endl;
    } else {
        std::cout << "connect redis failed..." << std::endl;
        exit(1);
    }
    
    const Value& mysql_config = json_doc["game_logic_mysql"];
    string mysql_ip = mysql_config["ip"].GetString();
    int mysql_port = mysql_config["port"].GetInt();
    string mysql_user = mysql_config["user"].GetString();
    string mysql_pass = mysql_config["password"].GetString();
    string mysql_scheme = mysql_config["scheme"].GetString();
    
    _mysql_client = *new MySQLClient(mysql_ip, mysql_user, mysql_pass, mysql_scheme, mysql_port, NULL, 0);
    _mysql_client.Connect();
    
    if(_mysql_client.IsConnected())
    {
        std::cout << "connect mysql succeed..." << std::endl;
    }
    else
    {
        exit(1);
    }
    
}

DataManager::~DataManager()
{
    
}

DataManager::DataManager(const DataManager& dm)
{
    
}

DataManager* DataManager::GetInstance()
{
    static DataManager instance;
    return &instance;
}

bool DataManager::IsUserCreated(unsigned int user_id)
{
    bool is_created = false;
    
    redisReply r;
    int is_ok = _redis_client.HGet("account", user_id, r);
    if(_redis_client.IsQuerySucceed(is_ok))
    {
        if(!_redis_client.IsReplyNull(r)){
            Statement statement(_mysql_client, "select * from player_account where id = ?");
            statement << Nullable<unsigned int>(user_id) << execute;
            if(!statement.Eof())
            {
                is_created = true;
            }
        }
    }
    
    return is_created;
}