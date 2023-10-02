#ifndef LUA_HELPERS_H
#define LUA_HELPERS_H

#include "Arduino.h"

#define LUA_USE_C89

#include "lua\lua.hpp"
//#include "..\lua\src\lualib.h"
//#include "..\lua\src\lauxlib.h"



class LuaHelper {
  public:
    void LuaHelpers();
    lua_State *_state;
    String lua_runstring(const String *script);
    void _lua_register(const String name, const lua_CFunction function);

  private:
    String addConstants();
};

#endif