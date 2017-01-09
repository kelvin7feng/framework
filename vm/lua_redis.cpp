//
//  lua_redis.cpp
//  server
//
//  Created by 冯文斌 on 17/1/6.
//  Copyright © 2017年 kelvin. All rights reserved.
//

#include <iostream>
#include <string.h>
#include "lua_redis.hpp"
#include "db_buffer.h"
#include "db_client_manager.hpp"

/*static int TestCallCpp(lua_State* lua_state);
static int PushRedisSet(lua_State* lua_state);
static int PushRedisGet(lua_State* lua_state);
static int PushRedisDel(lua_State* lua_state);
static int PushRedisHSet(lua_State* lua_state);
static int PushRedisHGet(lua_State* lua_state);
static int PushRedisHDel(lua_State* lua_state);
static int PushRedisExpire(lua_State* lua_state);*/

static int PushRedisSet(lua_State* lua_state)
{
    int nRetCode = 0;
    int nParam = lua_gettop(lua_state);
    if(nParam != 5)
    {
        std::cout << "count of param is not equal to 2..." << std::endl;
        return 0;
    }
    
    unsigned int uUserId = lua_tonumber(lua_state, nParam - 4);
    unsigned int uEventType = lua_tonumber(lua_state, nParam - 3);
    std::string szTableName = lua_tostring(lua_state, nParam - 2);
    std::string szKey = lua_tostring(lua_state, nParam - 1);
    std::string szValue = lua_tostring(lua_state, nParam);
    
    IKG_Buffer* pBuffer = DB_CreateSetBuffer(szTableName, szKey, szValue);
    DB_SetBufferHead(pBuffer, uUserId, uEventType);
    
    g_pDBClientMgr->PushRedisRequest(1, pBuffer);
    
    return nRetCode;
}

static int PushRedisGet(lua_State* lua_state)
{
    int nRetCode = 0;
    int nParam = lua_gettop(lua_state);
    if(nParam != 4)
    {
        std::cout << "count of param is not equal to 2..." << std::endl;
        return 0;
    }
    
    unsigned int uUserId = lua_tonumber(lua_state, 1);
    unsigned int uEventType = lua_tonumber(lua_state, 2);
    std::string szTableName = lua_tostring(lua_state, 3);
    std::string szKey = lua_tostring(lua_state, 4);
    
    IKG_Buffer* pBuffer = DB_CreateGetBuffer(szTableName, szKey);
    DB_SetBufferHead(pBuffer, uUserId, uEventType);
    
    g_pDBClientMgr->PushRedisRequest(1, pBuffer);
    
    return nRetCode;
}

static int PushRedisDel(lua_State* lua_state)
{
    int nRetCode = 0;
    int nParam = lua_gettop(lua_state);
    if(nParam != 4)
    {
        std::cout << "count of param is not equal to 2..." << std::endl;
        return 0;
    }
    
    unsigned int uUserId = lua_tonumber(lua_state, 1);
    unsigned int uEventType = lua_tonumber(lua_state, 2);
    std::string szTableName = lua_tostring(lua_state, 3);
    std::string szKey = lua_tostring(lua_state, 4);
    
    IKG_Buffer* pBuffer = DB_CreateDelBuffer(szTableName, szKey);
    DB_SetBufferHead(pBuffer, uUserId, uEventType);
    
    g_pDBClientMgr->PushRedisRequest(1, pBuffer);
    
    return nRetCode;
}

static int PushRedisHSet(lua_State* lua_state)
{
    int nRetCode = 0;
    int nParam = lua_gettop(lua_state);
    if(nParam != 5)
    {
        std::cout << "count of param is not equal to 2..." << std::endl;
        return 0;
    }
    
    unsigned int uUserId = lua_tonumber(lua_state, nParam - 4);
    unsigned int uEventType = lua_tonumber(lua_state, nParam - 3);
    std::string szTableName = lua_tostring(lua_state, nParam - 2);
    std::string szKey = lua_tostring(lua_state, nParam - 1);
    std::string szValue = lua_tostring(lua_state, nParam);
    
    IKG_Buffer* pBuffer = DB_CreateHSetBuffer(szTableName, szKey, szValue);
    DB_SetBufferHead(pBuffer, uUserId, uEventType);
    
    g_pDBClientMgr->PushRedisRequest(1, pBuffer);
    
    return nRetCode;
}

static const luaL_reg sLuaFunction[] =
{
    {"PushRedisSet", PushRedisSet},
    {"PushRedisGet", PushRedisGet},
    {"PushRedisDel", PushRedisDel},
    {"PushRedisHSet", PushRedisHSet},
    {NULL, NULL}
};

int LuaRegisterRedis(lua_State* lua_state)
{
    luaL_register(lua_state, "CRedis", sLuaFunction);
    return 1;
}
