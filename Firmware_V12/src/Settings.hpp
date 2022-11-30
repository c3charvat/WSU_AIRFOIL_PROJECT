#ifndef Settings_Hpp
#define Settings_Hpp


/*
Style Guide:
All functions are lower case
All Classes/Name Spaces/Enums first letter of each word is capitalized no _
Naming scheme:
    const expressions are in CAPS
*/

#define Has_rs485_ecoders


// define your own namespace to hold constants
namespace DevConstants
{
    // constants have internal linkage by default
    constexpr bool SWD_PROGRAMING_MODE = true;
    constexpr bool ENDSTOP_BYPASS_ENABLE = true;
    constexpr bool VERBOSE_MODE = true;
    constexpr bool INVERT_ENCODERS = true;
    #ifdef Has_rs485_ecoders
    constexpr bool RS485_ENCODER = true;
    #else
    constexpr bool RS485_ENCODER =false;
    #endif
    constexpr bool Usb_only = false;
    // constexpr bool
}

namespace Settings
{
    // physical limits
    // constexpr float X_POSITION_MAX = 390;           // max position in mm
    // constexpr float Y_POSITION_MAX = 245;
    constexpr float X_POSITION_MAX = (390 / 2);      // max position in mm
    constexpr float X_POSITION_MIN = -1 * (390 / 2); // max position in mm
    constexpr float Y_POSITION_MAX = (245);
    constexpr float Y_POSITION_MIN = 0;
    constexpr float AOA_T_POSITION_MAX = 40;
    constexpr float AOA_T_POSITION_MIN = -40;
    constexpr float AOA_B_POSITION_MAX = 40;
    constexpr float AOA_B_POSITION_MIN = -40;

    constexpr float AOA_T_HOME_OFFSET = -20;
    constexpr float AOA_B_HOME_OFFSET = -20;

    constexpr float MIN_MAX_ARRAY[8] = {X_POSITION_MAX, X_POSITION_MIN, Y_POSITION_MAX, 
    Y_POSITION_MIN, AOA_T_POSITION_MAX, AOA_T_POSITION_MIN, AOA_B_POSITION_MAX, AOA_B_POSITION_MIN};
    // physical machine settings
    constexpr int X_LEAD_SCREW_PITCH = 2; // lead screw pitch in mm/revolution
    constexpr int Y_LEAD_SCREW_PITCH = 2;
    constexpr int AOA_T_NODE_ADDR = 0x54;
    constexpr int AOA_B_NODE_ADDR = 0x34;

}

#endif