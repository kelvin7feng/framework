//
//  redis_client.hpp
//  redis
//
//  Created by 冯文斌 on 16/11/2.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef redis_client_hpp
#define redis_client_hpp

#include <stdio.h>
#include <iostream>

#include "hiredis.h"

using namespace std;

namespace RedisWrap {
    class RedisClient
    {
        enum CmdType {
            SET = 1,
            GET = 2,
            HSET = 3,
            HGET = 4,
        };
    public:
        RedisClient();
        RedisClient(const string &host, unsigned int port);
        RedisClient(const RedisClient &rc);
        virtual ~RedisClient();
        static RedisClient* GetInstance();
        
        string GetReplyContent(redisReply& r);
        void FreeReply(redisReply *r);
        bool IsReplyNull(redisReply& r);
        bool IsQuerySucceed(int type);
        
        int Connect();
        bool IsConnect();
        
        int Set(const string &key, const string &value);
        int Set(int64_t key, const string &value);
        int Get(const string &key, redisReply& result);
        int Get(int64_t key, redisReply& result);
        
        int HSet(const string &key, const string &filed, const string &value);
        int HSet(int64_t key, int64_t filed, const string &value);
        int HGet(const string &key, const string &filed, redisReply& r);
        int HGet(int64_t key, int64_t filed, redisReply* r);
        int HGet(const string &key, int64_t filed, redisReply& result);
        
    private:
        redisReply* ExcuteCommand(const char* format, ...);
        bool IsExcuteSucceed(int cmd_type, redisReply* r);
        
        string _host;
        unsigned int _port;
        bool _is_connect;
        struct timeval _timeout;
        
        redisContext *_redis_context;
        redisReply *_reply;
    };
}

#endif /* redis_client_hpp */
