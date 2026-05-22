/**
 * @file MovementLua.hpp
 * @brief Lua bindings for motion control (stepper, homing, position)
 *
 * Exposes motion functions to Lua scripts:
 *   move_absolute(x, y, aoat, aoab)
 *   move_relative(dx, dy, daoat, daoab)
 *   home_all()
 *   get_position() -> {x, y, aoat, aoab}
 *   set_speed(axis, speed)
 *   set_accel(axis, accel)
 *
 * Usage: Call MovementLua::registerFunctions(lua_State*) in LuaRuntime
 */

#ifndef MOVEMENT_LUA_HPP
#define MOVEMENT_LUA_HPP

extern "C" {
#include "lua/lua.h"
}

namespace MovementLua {
    // Register all motion functions to Lua
    void registerFunctions(lua_State* L);
}

#endif // MOVEMENT_LUA_HPP
