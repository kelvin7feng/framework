//
//  klua.cpp
//  lesson01
//
//  Created by 冯文斌 on 16/9/13.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <string>
#include "lua.hpp"
#include "klua.hpp"
#include "map"

using namespace std;

CLuaEngine::CLuaEngine()
{
    server_path[SERVER_TYPE::LOGIC] = "./../script/logic_main.lua";
    server_path[SERVER_TYPE::LOGIN] = "./../script/login_main.lua";
}

CLuaEngine::~CLuaEngine()
{
    this->Close();
}

void CLuaEngine::Close()
{
    lua_close(m_pLuaState);
}

//Lua调用C++的函数
static int Average(lua_State* pLuaState){
    //获取参数个数
    int n = lua_gettop(pLuaState);
    double sum = 0;
    int temp = 0;
    
    for (int i = 1; i <= n; i++) {
        temp = lua_tonumber(pLuaState, i);
        sum += temp;
    }
    
    int avg = sum/n;
    lua_pushnumber(pLuaState, avg);
    
    return 1;
}

void CLuaEngine::stackDump(lua_State* L){
    cout<<"\n------------begin dump lua stack------------"<<endl;
    int i = 0;
    int top = lua_gettop(L);
    int stack_index_1 = top;
    int stack_index_2 = -1;
    for (i = 1; i <= top; ++i) {
        int t = lua_type(L, i);
        printf("%d|",stack_index_1);
        switch (t) {
            case LUA_TSTRING:
            {
                printf("'%s' ", lua_tostring(L, i));
            }
                break;
            case LUA_TBOOLEAN:
            {
                printf(lua_toboolean(L, i) ? "true " : "false ");
            }break;
            case LUA_TNUMBER:
            {
                printf("%g ", lua_tonumber(L, i));
            }
                break;
            default:
            {
                printf("%s ", lua_typename(L, t));
            }
                break;
        }
        printf("|%d\n",stack_index_2);
        stack_index_1 --;
        stack_index_2 --;
        
    }
    cout<<"------------end dump lua stack--------------\n"<<endl;
}

//调用脚本处理
int CLuaEngine::CallLua(const std::string& request)
{
    lua_settop(m_pLuaState, 0);
    lua_getglobal(m_pLuaState, "ClientRequest");
    lua_pushstring(m_pLuaState, request.c_str());
    
    int ret = lua_pcall(m_pLuaState, 1, 1, 0);
    
    //调用出错
    if(ret)
    {
        const char *pErrorMsg = lua_tostring(m_pLuaState, -1);
        cout << pErrorMsg << endl;
        return 0;
    }
    
    //取值输出
    if (lua_isnumber(m_pLuaState, -1))
    {
        int fValue = lua_tonumber(m_pLuaState, -1);
        if(fValue){
            //成功逻辑
        }
    } else {
        //返回值类型错误
    }
    
    lua_settop(m_pLuaState, 0);
    
    return 0;
}

//Lua引擎初始化
int CLuaEngine::InitState(int server_type)
{
    m_pLuaState = luaL_newstate();
    
    const luaL_Reg *lib = m_luaLibs;
    for(; lib->func != NULL; lib++)
    {
        luaL_requiref(m_pLuaState, lib->name, lib->func, 1);
        lua_settop(m_pLuaState, 0);
    }
    
    luaL_openlibs(m_pLuaState);
    
    string szScriptPath = server_path[server_type];
    int nStatus = luaL_dofile(m_pLuaState, szScriptPath.c_str());
    
    int nResult = 0;
    if(nStatus == LUA_OK)
    {
        nResult = lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0);
    }
    else
    {
        std::cout<<"load file error [file:"<<szScriptPath.c_str()<<"]: "<<lua_tostring(m_pLuaState, -1)<<endl;
        
        exit(1);
    }
    
    return nResult;
}

const luaL_Reg CLuaEngine::m_commonLibs[] =
{
    {"Average", Average},
    {NULL, NULL}
};

const luaL_Reg CLuaEngine::m_luaLibs[] =
{
    {"base", luaopen_base},
    {"io", luaopen_io},
    {"luaEngine", CLuaEngine::LuaOpenLib},
    {NULL, NULL}
};

int CLuaEngine::LuaOpenLib(lua_State* pLuaState)
{
    luaL_newlib(pLuaState, m_commonLibs);
    return 1;
}