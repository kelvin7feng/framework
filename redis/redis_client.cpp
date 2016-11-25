//
//  redis_client.cpp
//  redis
//
//  Created by 冯文斌 on 16/11/2.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include "redis_client.hpp"

namespace RedisWrap {
    RedisClient::RedisClient()
    {
        _host = "";
        _port = 0;
        _is_connect = false;
        _timeout = {2, 0};
        _redis_context = nullptr;
    }

    RedisClient::RedisClient(const string &host, unsigned int port)
    {
        _host = host;
        _port = port;
        _is_connect = false;
        _timeout = {2, 0};
        _redis_context = nullptr;
    }

    RedisClient::RedisClient(const RedisClient &rc)
    {
        _host = rc._host;
        _port = rc._port;
        _is_connect = rc._is_connect;
        _timeout = rc._timeout;
        _redis_context = rc._redis_context;
    }

    RedisClient::~RedisClient()
    {
        redisFree(_redis_context);
    }

    string RedisClient::GetReplyContent(redisReply& r)
    {
        return r.str;
    }
    
    RedisClient* RedisClient::GetInstance()
    {
        static RedisClient instance;
        return &instance;
    }
    
    bool RedisClient::IsReplyNull(redisReply& r)
    {
        if(r.len <=0 && r.elements <=0)
        {
            return true;
        }
        
        return false;
    }
    
    bool RedisClient::IsQuerySucceed(int type)
    {
        if(type == REDIS_OK)
        {
            return true;
        }
        
        return false;
    }
    
    void RedisClient::FreeReply(redisReply *r)
    {
        freeReplyObject(r);
    }

    int RedisClient::Connect()
    {
        _redis_context = (redisContext*)redisConnectWithTimeout(_host.c_str(), _port, _timeout);
        if ( (NULL == _redis_context) || (_redis_context->err) )
        {
            if (_redis_context)
            {
                std::cout << "connect error:" << _redis_context->errstr << std::endl;
            }
            else
            {
                std::cout << "connect error: can't allocate redis context." << std::endl;
            }
            return -1;
        }
        
        _is_connect = true;
        
        return 0;
    }

    bool RedisClient::IsExcuteSucceed(int cmd_type, redisReply* r)
    {
        bool is_ok = false;
        if( NULL == r)
        {
            printf("Execut command1 failure\n");
            return is_ok;
        }
        
        switch (cmd_type) {
                
            case CmdType::SET:
                if(!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK")==0))
                {
                    return is_ok;
                }
                break;
                
            case CmdType::GET:
                if ( r->type != REDIS_REPLY_STRING)
                {
                    return is_ok;
                }
                break;
                
            case CmdType::HSET:
                if(r->type != REDIS_REPLY_INTEGER)
                {
                    return is_ok;
                }
                break;
              
            case CmdType::HGET:
                if(r->type != REDIS_REPLY_STRING)
                {
                    return is_ok;
                }
                
            default:
                break;
        }
        
        is_ok = true;
        
        return is_ok;
    }

    redisReply* RedisClient::ExcuteCommand(const char* format, ... )
    {
        va_list ap;
        redisReply *reply = NULL;
        va_start(ap, format);
        
        reply = (redisReply*)redisvCommand(_redis_context, format, ap);
        va_end(ap);
        
        return reply;
    }

    int RedisClient::Set(const string &key, const string &value)
    {
        int type = REDIS_ERR;
        redisReply *reply = NULL;
        
        reply = (redisReply*)ExcuteCommand("SET %b %b", key.c_str(), key.size(), value.c_str(), value.size());
        
        if (IsExcuteSucceed(CmdType::SET, reply))
        {
            type = REDIS_OK;
        }
        
        freeReplyObject(reply);
        
        return type;
    }

    int RedisClient::Set(int64_t key, const string &value)
    {
        int type = REDIS_ERR;
        redisReply *reply = NULL;
        
        reply = (redisReply*)ExcuteCommand("SET %d %b", key, value.c_str(), value.size());
        
        if (IsExcuteSucceed(CmdType::SET, reply))
        {
            type = REDIS_OK;
        }
        
        freeReplyObject(reply);
        
        return type;
    }

    int RedisClient::Get(const string &key, redisReply& result)
    {
        int type = REDIS_ERR;
        redisReply *reply = NULL;
        
        reply = (redisReply*)ExcuteCommand("GET %b", key.c_str(), key.size());
        
        if (IsExcuteSucceed(CmdType::GET, reply))
        {
            type = REDIS_OK;
        }
        
        result.len = reply->len;
        result.str = reply->str;
        result.integer = reply->integer;
        result.element = reply->element;
        result.elements = reply->elements;
        result.type = reply->type;
        
        return type;
    }

    int RedisClient::Get(int64_t key, redisReply& result)
    {
        int type = REDIS_ERR;
        redisReply *reply = NULL;
        
        reply = (redisReply*)ExcuteCommand("GET %d", key);
        
        if (IsExcuteSucceed(CmdType::GET, reply))
        {
            type = REDIS_OK;
        }
        
        result.len = reply->len;
        result.str = reply->str;
        result.integer = reply->integer;
        result.element = reply->element;
        result.elements = reply->elements;
        result.type = reply->type;
        
        return type;
    }

    int RedisClient::HSet(const string &key, const string &filed, const string &value)
    {
        int type = REDIS_ERR;
        redisReply *reply = NULL;
        
        reply = (redisReply*)ExcuteCommand("HSET %b %b %b",
                                           key.c_str(), key.size(), filed.c_str(), filed.size(), value.c_str(), value.size());
        
        if (IsExcuteSucceed(CmdType::HSET, reply))
        {
            type = REDIS_OK;
        }
        
        freeReplyObject(reply);
        
        return type;
    }

    int RedisClient::HSet(int64_t key, int64_t filed, const string &value)
    {
        int type = REDIS_ERR;
        redisReply *reply = NULL;
        
        reply = (redisReply*)ExcuteCommand("HSET %d %d %b", key, filed, value.c_str(), value.size());
        
        if (IsExcuteSucceed(CmdType::HSET, reply))
        {
            type = REDIS_OK;
        }
        
        freeReplyObject(reply);
        
        return type;
    }

    int RedisClient::HGet(const string &key, const string &filed, redisReply& result)
    {
        int type = REDIS_ERR;
        redisReply *reply = NULL;
        
        reply = (redisReply*)ExcuteCommand("HGET %b %b", key.c_str(), key.size(), filed.c_str(), filed.size());
        
        if (IsExcuteSucceed(CmdType::HGET, reply))
        {
            type = REDIS_OK;
        }
        
        result.len = reply->len;
        result.str = reply->str;
        result.integer = reply->integer;
        result.element = reply->element;
        result.elements = reply->elements;
        result.type = reply->type;
        
        return type;
    }
    
    int RedisClient::HGet(const string &key, int64_t filed, redisReply& result)
    {
        int type = REDIS_ERR;
        redisReply *reply = NULL;
        
        reply = (redisReply*)ExcuteCommand("HGET %b %d", key.c_str(), key.size(), filed);
        
        if (IsExcuteSucceed(CmdType::HGET, reply))
        {
            type = REDIS_OK;
        }
        
        result.len = reply->len;
        result.str = reply->str;
        result.integer = reply->integer;
        result.element = reply->element;
        result.elements = reply->elements;
        result.type = reply->type;
        
        return type;
    }
    
    int RedisClient::HGet(int64_t key, int64_t filed, redisReply* result)
    {
        return 0;
    }
}