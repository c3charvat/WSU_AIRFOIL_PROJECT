#include "MovementLua.hpp"
#include "Movement.hpp"
#include "Data_structures.h"
#include "HalSerial.hpp"
#include "amt21_driver.hpp"
#include "SpeedyStepper.h"
#include <cstring>
extern "C" {
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

// Stepper externs (defined in Pin_Setup.cpp)
extern SpeedyStepper x0_Stepper;
extern SpeedyStepper x1_Stepper;
extern SpeedyStepper y0_Stepper;
extern SpeedyStepper y1_Stepper;
extern SpeedyStepper y2_Stepper;
extern SpeedyStepper y3_Stepper;
extern SpeedyStepper aoat_Stepper;
extern SpeedyStepper aoab_Stepper;

// External serial object
extern HalSerial Serial;

// External movement functions and structs
// Declarations come from Movement.hpp (included above); no re-declaration needed.
extern struct PositionStruct gCurrentPosition;
extern struct Error gError;
extern uint16_t gAoatEncoderZero;  // encoder counts at last home, set by lua_home_all
extern uint16_t gAoabEncoderZero;
#ifdef Has_rs485_ecoders
extern Amt21Encoder aoat_Encoder;
extern Amt21Encoder aoab_Encoder;
#endif

// Wrappers that handle both encoder and non-encoder builds transparently
static inline void call_move_function(PositionStruct* cur, PositionStruct* target, Error* err) {
#ifdef Has_rs485_ecoders
    move_function(cur, target, err, Settings::AOA_T_NODE_ADDR, Settings::AOA_B_NODE_ADDR);
#else
    move_function(cur, target, err);
#endif
}

static inline void call_home_all(PositionStruct* cur, Error* err) {
#ifdef Has_rs485_ecoders
    home_all(cur, err, Settings::AOA_T_NODE_ADDR, Settings::AOA_B_NODE_ADDR);
#else
    home_all(cur, err);
#endif
}

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
    call_move_function(&gCurrentPosition, &target, &gError);
    if (gMotionWasAborted) {
        return luaL_error(L, "move_absolute aborted mid-motion");
    }
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
    call_move_function(&gCurrentPosition, &target, &gError);
    if (gMotionWasAborted) {
        return luaL_error(L, "move_relative aborted mid-motion");
    }
    Serial.println("[LUA] move_relative called");
    return 0;
}

// home_all()
static int lua_home_all(lua_State* L) {
    call_home_all(&gCurrentPosition, &gError);
#ifdef Has_rs485_ecoders
    // Record the encoder counts that correspond to the home angle offsets.
    // sync_aoa_position() uses these as the zero reference for future re-syncs.
    gAoatEncoderZero = aoat_Encoder.amt_get_pos();
    gAoabEncoderZero = aoab_Encoder.amt_get_pos();
#endif
    Serial.println("[LUA] home_all called");
    return 0;
}

// sync_aoa_position()
// Reads the absolute AOA encoders and recomputes gCurrentPosition.aoatpos /
// aoabpos from the zero reference captured at the last home_all().
// Call this after an abort to recover accurate AOA position without re-homing.
// Returns the corrected {aoat, aoab} angles as a Lua table.
#ifdef Has_rs485_ecoders
static int lua_sync_aoa_position(lua_State* L) {
    // AMT21 14-bit encoder: 16384 counts per full revolution.
    static constexpr float COUNTS_PER_DEG = 16384.0f / 360.0f;

    // Signed delta from home counts handles wrap-around within the ±40° range.
    int16_t aoat_delta = (int16_t)(aoat_Encoder.amt_get_pos() - gAoatEncoderZero);
    int16_t aoab_delta = (int16_t)(aoab_Encoder.amt_get_pos() - gAoabEncoderZero);

    gCurrentPosition.aoatpos = (float)aoat_delta / COUNTS_PER_DEG + Settings::AOA_T_HOME_OFFSET;
    gCurrentPosition.aoabpos = (float)aoab_delta / COUNTS_PER_DEG + Settings::AOA_B_HOME_OFFSET;

    Serial.println("[LUA] sync_aoa_position: AOA position corrected from encoders");

    lua_newtable(L);
    lua_pushstring(L, "aoat"); lua_pushnumber(L, gCurrentPosition.aoatpos); lua_settable(L, -3);
    lua_pushstring(L, "aoab"); lua_pushnumber(L, gCurrentPosition.aoabpos); lua_settable(L, -3);
    return 1;
}
#endif

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
        Serial.println((String("[LUA] set_speed ") + axis + " = " + std::to_string(speed)).c_str());
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
        Serial.println((String("[LUA] set_accel ") + axis + " = " + std::to_string(accel)).c_str());
    } else {
        Serial.println((String("[LUA] set_accel: unknown axis ") + axis).c_str());
    }
    return 0;
}

namespace MovementLua {
    void registerFunctions(lua_State* L) {
        lua_register(L, "move_absolute", lua_move_absolute);
        lua_register(L, "move_relative", lua_move_relative);
        lua_register(L, "home_all", lua_home_all);
        lua_register(L, "get_position", lua_get_position);
        lua_register(L, "set_speed", lua_set_speed);
        lua_register(L, "set_accel", lua_set_accel);
#ifdef Has_rs485_ecoders
        lua_register(L, "sync_aoa_position", lua_sync_aoa_position);
#endif
        // Add more as needed (set_speed, set_accel, etc.)
    }
}
