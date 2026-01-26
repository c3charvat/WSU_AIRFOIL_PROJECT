#include "MovementLua.hpp"
#include "Movement.hpp"
#include "HalSerial.hpp"
#include <cstring>

// External serial object
extern HalSerial Serial;

// External movement functions and structs
extern void move_function(struct PositionStruct *current_pos, struct PositionStruct *input_data, struct Error *error);
extern void home_all(struct PositionStruct *current_pos, struct Error *error);
extern struct PositionStruct gCurrentPosition;
extern struct Error gError;

// Helper: get Lua number argument
static float get_arg(lua_State* L, int idx) {
    return static_cast<float>(luaL_checknumber(L, idx));
}

// Helper: get stepper by axis name
static SpeedyStepper* get_stepper(const char* axis) {
    if (strcmp(axis, "x0") == 0) return &x0_Stepper;
    if (strcmp(axis, "x1") == 0) return &x1_Stepper;
    if (strcmp(axis, "y0") == 0) return &y0_Stepper;
    if (strcmp(axis, "y1") == 0) return &y1_Stepper;
    if (strcmp(axis, "y2") == 0) return &y2_Stepper;
    if (strcmp(axis, "y3") == 0) return &y3_Stepper;
    if (strcmp(axis, "aoat") == 0) return &aoat_Stepper;
    if (strcmp(axis, "aoab") == 0) return &aoab_Stepper;
    return nullptr;
}

// move_absolute(x, y, aoat, aoab)
static int lua_move_absolute(lua_State* L) {
    float x = get_arg(L, 1);
    float y = get_arg(L, 2);
    float aoat = get_arg(L, 3);
    float aoab = get_arg(L, 4);
    PositionStruct target;
    target.xpos = x;
    target.ypos = y;
    target.aoatpos = aoat;
    target.aoabpos = aoab;
    move_function(&gCurrentPosition, &target, &gError);
    Serial.println("[LUA] move_absolute called");
    return 0;
}

// move_relative(dx, dy, daoat, daoab)
static int lua_move_relative(lua_State* L) {
    float dx = get_arg(L, 1);
    float dy = get_arg(L, 2);
    float daoat = get_arg(L, 3);
    float daoab = get_arg(L, 4);
    PositionStruct target = gCurrentPosition;
    target.xpos += dx;
    target.ypos += dy;
    target.aoatpos += daoat;
    target.aoabpos += daoab;
    move_function(&gCurrentPosition, &target, &gError);
    Serial.println("[LUA] move_relative called");
    return 0;
}

// home_all()
static int lua_home_all(lua_State* L) {
    home_all(&gCurrentPosition, &gError);
    Serial.println("[LUA] home_all called");
    return 0;
}

// get_position() -> returns table {x, y, aoat, aoab}
static int lua_get_position(lua_State* L) {
    lua_newtable(L);
    lua_pushstring(L, "x"); lua_pushnumber(L, gCurrentPosition.xpos); lua_settable(L, -3);
    lua_pushstring(L, "y"); lua_pushnumber(L, gCurrentPosition.ypos); lua_settable(L, -3);
    lua_pushstring(L, "aoat"); lua_pushnumber(L, gCurrentPosition.aoatpos); lua_settable(L, -3);
    lua_pushstring(L, "aoab"); lua_pushnumber(L, gCurrentPosition.aoabpos); lua_settable(L, -3);
    return 1;
}

// set_speed(axis, speed)
static int lua_set_speed(lua_State* L) {
    const char* axis = luaL_checkstring(L, 1);
    float speed = static_cast<float>(luaL_checknumber(L, 2));
    SpeedyStepper* stepper = get_stepper(axis);
    if (stepper) {
        stepper->setSpeedInStepsPerSecond(speed);
        Serial.println((String("[LUA] set_speed ") + axis + " = " + String(speed)).c_str());
    } else {
        Serial.println((String("[LUA] set_speed: unknown axis ") + axis).c_str());
    }
    return 0;
}

// set_accel(axis, accel)
static int lua_set_accel(lua_State* L) {
    const char* axis = luaL_checkstring(L, 1);
    float accel = static_cast<float>(luaL_checknumber(L, 2));
    SpeedyStepper* stepper = get_stepper(axis);
    if (stepper) {
        stepper->setAccelerationInStepsPerSecondPerSecond(accel);
        Serial.println((String("[LUA] set_accel ") + axis + " = " + String(accel)).c_str());
    } else {
        Serial.println((String("[LUA] set_accel: unknown axis ") + axis).c_str());
    }
    return 0;
}

namespace MovementLua {
    void register(lua_State* L) {
        lua_register(L, "move_absolute", lua_move_absolute);
        lua_register(L, "move_relative", lua_move_relative);
        lua_register(L, "home_all", lua_home_all);
        lua_register(L, "get_position", lua_get_position);
        lua_register(L, "set_speed", lua_set_speed);
        lua_register(L, "set_accel", lua_set_accel);
        // Add more as needed (set_speed, set_accel, etc.)
    }
}
