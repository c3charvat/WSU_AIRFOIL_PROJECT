#ifndef Settings_Hpp
#define Settings_Hpp

// define your own namespace to hold constants
namespace DEV_constants
{
    // constants have internal linkage by default
    constexpr bool Swd_programming_mode = true;
    constexpr bool Endstop_bypass_enable = true;
    constexpr bool Verbose_mode = true;
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

    constexpr float MinMaxArray[8] = {X_POSITION_MAX, X_POSITION_MIN, Y_POSITION_MAX, 
    Y_POSITION_MIN, AOA_T_POSITION_MAX, AOA_T_POSITION_MIN, AOA_B_POSITION_MAX, AOA_B_POSITION_MIN};
    // physical machine settings
    constexpr int X_Lead_p = 2; // lead screw pitch in mm/revolution
    constexpr int Y_Lead_p = 2;

}

#endif