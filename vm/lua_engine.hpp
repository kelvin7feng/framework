//
//  lua_engine.hpp
//  lua_five_one
//
//  Created by 冯文斌 on 16/11/28.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef lua_engine_hpp
#define lua_engine_hpp

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
};

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>

using namespace std;

class LuaEngine {
    
public:
    enum SERVER_TYPE {
        LOGIC = 1,
        LOGIN = 2
    };
    
    LuaEngine();
    ~LuaEngine();
    void Close();
    lua_State* GetLuaState();
    int InitState(int server_type);
    int CallLua(const std::string& request);
    
private:
    void stackDump(lua_State* L);
    
    lua_State* m_lua_state;
    map<int, string> server_path;
};

#endif /* lua_engine_hpp */
