//
//  klua.hpp
//  lesson01
//
//  Created by 冯文斌 on 16/9/13.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef klua_hpp
#define klua_hpp

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include "lua.hpp"

using namespace std;

class CLuaEngine {
public:
    
    enum SERVER_TYPE {
        LOGIC = 1,
        LOGIN = 2
    };
    
    CLuaEngine();
    ~CLuaEngine();
    void Close();
    int InitState(int server_type);
    int CallLua(const std::string& request);
    
    static int LuaOpenLib(lua_State* pLuaState);
    static const luaL_Reg m_luaLibs[];
    static const luaL_Reg m_commonLibs[];
    
private:
    void stackDump(lua_State* L);
    
    lua_State* m_pLuaState;
    map<int, string> server_path;
};

#endif /* klua_hpp */
