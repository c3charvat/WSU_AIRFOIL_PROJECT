#include <string>
#include "Movement.hpp"
#include "Settings.hpp"
#include "SpeedyStepper.h"
#include "Pin_Setup.h"
#include "Data_structures.h"
#include "amt21_driver.hpp"
using namespace std;

extern SpeedyStepper x0_Stepper;   // motor 0
extern SpeedyStepper y0_Stepper;   // motor 1
extern SpeedyStepper y1_Stepper;   // motor 2_1 2_2 os mirriored of this axis but doesnt work?
extern SpeedyStepper y3_Stepper;   // motor 3
extern SpeedyStepper aoat_Stepper; // motor 4
extern SpeedyStepper aoab_Stepper; // motor 5
extern SpeedyStepper y2_Stepper;   // motor 6
extern SpeedyStepper x1_Stepper;   // motor 7

void initialize_Movement_Struct(struct PositionStruct *pos, struct ControlStruct *control)
{
    pos->xpos = 0;
    pos->ypos = 0;
    pos->aoatpos = 0;
    pos->aoabpos = 0;
    control->wificonnected = false;
    control->usbconnected = false;
    pos->source = control; // goal here is to create a link to a single structre that will update with what interface has control
}

void initialize_Movement_Struct_NC(struct PositionStruct *pos)
{
    pos->xpos = 0;
    pos->ypos = 0;
    pos->aoatpos = 0;
    pos->aoabpos = 0;
    //control->wificonnected = false;
    //control->usbconnected = false;
    pos->source = NULL; // goal here is to create a link to a single structre that will update with what interface has control
}

bool bounds_check(float pos_max, float pos_min, float desired_pos)
{
    if (desired_pos > pos_max || desired_pos < pos_min)
    {
        // if the pos is greater than the max or the pos less than the min
        return false;
    }
    else
    {
        return true;
    }
}

PositionStruct REL_MOVEMENT_CALC(struct PositionStruct *current_pos, struct PositionStruct *input_pos, struct Error *error)
{
    struct PositionStruct next_pos;
    float current_position_data[4] = {current_pos->xpos, current_pos->ypos, current_pos->aoatpos, current_pos->aoabpos};
    float input_position_data[4] = {input_pos->xpos, input_pos->ypos, input_pos->aoatpos, input_pos->aoabpos};
    float ammount_to_move[4] = {0, 0, 0, 0};

    bool bounds_check_flag = false;
    for (size_t i = 0; i < 3; i++)
    {
        bounds_check_flag = bounds_check(Settings::MinMaxArray[i * 2], Settings::MinMaxArray[(i * 2) + 1], input_position_data[i]);
        if (bounds_check_flag == false)
        {
            // Display Bounds Error
            error->error_name = "Bounds Check Failed";
            error->error_information_1 = String(Settings::MinMaxArray[i * 2]);
            error->error_information_2 = String(Settings::MinMaxArray[(i * 2) + 1]);
            error->error_information_3 = String(input_position_data[i]);
            next_pos.xpos=0;
            next_pos.ypos=0;
            next_pos.aoatpos=0;
            next_pos.aoatpos=0;
            return next_pos;
        }
    }

    for (size_t i = 0; i < 3; i++)
    {
        if (input_position_data[i] > current_position_data[i])
        { // Test Cases
          //  1                   0
          // -4                  -9
          //  4                  -8
            // if the next destination is grater than the current postion and the current posttion is postive
            ammount_to_move[i] = input_position_data[i] - current_position_data[i]; // These are the same i think ove covered every case?
        }
        else if (input_position_data[i] < current_position_data[i])
        { // Test Cases
          //   2                       8
          //   -8                      -2
          //    -4                      6
            // if the next pos is grater that the current position and the current position is negative
            ammount_to_move[i] = input_position_data[i] - current_position_data[i];
        }
        else
        {
            // next_position_data[i]==current_position_data[i]
            ammount_to_move[i] = 0; // your at where they requested // or somthin funky happend still dont move
        }
    }
    next_pos.xpos=ammount_to_move[0];
    next_pos.ypos=ammount_to_move[1];
    next_pos.aoatpos=ammount_to_move[2];
    next_pos.aoatpos=ammount_to_move[3];
    return next_pos;
}

void MOVE_FUNCTION(struct PositionStruct *current_pos, struct PositionStruct *input_data, struct Error *error)
{
    struct PositionStruct *next_pos_ptr,next_pos;
    next_pos=REL_MOVEMENT_CALC(current_pos,input_data,error);


    x0_Stepper.setupRelativeMoveInSteps(next_pos.xpos / 5 * 200 * 8);  // Future: Make these iun terms of MM
    x1_Stepper.setupRelativeMoveInSteps(next_pos.xpos / 5 * 200 * 8); // Future: Make these iun terms of MM

    y0_Stepper.setupRelativeMoveInSteps(next_pos.ypos / 2 * 200 * 8);
    y1_Stepper.setupRelativeMoveInSteps(next_pos.ypos / 2 * 200 * 8);
    y2_Stepper.setupRelativeMoveInSteps(next_pos.ypos / 2 * 200 * 8);
    y3_Stepper.setupRelativeMoveInSteps(next_pos.ypos / 2 * 200 * 8);

    aoat_Stepper.setupRelativeMoveInSteps(next_pos.aoatpos / 1.8 * 5.18 * 8);
    aoab_Stepper.setupRelativeMoveInSteps(next_pos.aoabpos / 1.8 * 5.18 * 8);

    // Call A Blocking Function that Stops the Machine from doing anything else while the stepper is moving  This is desired since we aren not updating mid move.
    Serial.println("Entering while loop");
    while ((!x0_Stepper.motionComplete()) || (!y0_Stepper.motionComplete()) || (!y1_Stepper.motionComplete()) || (!y2_Stepper.motionComplete()) || (!y3_Stepper.motionComplete()) || (!aoat_Stepper.motionComplete()) || (!aoab_Stepper.motionComplete()))
    {
        if (DEV_constants::Endstop_bypass_enable == true)
        {
            break;
        }
        x0_Stepper.processMovement();
        x1_Stepper.processMovement();
        y0_Stepper.processMovement();
        y1_Stepper.processMovement(); // moving the Steppers here was a simple soltuion to having to do speical system intrrupts
        y2_Stepper.processMovement();
        y3_Stepper.processMovement();
        aoat_Stepper.processMovement();
        aoab_Stepper.processMovement();
    }
    Serial.println("Exited While Loop");
    current_pos->xpos=current_pos->xpos+next_pos.xpos;
    current_pos->ypos=current_pos->ypos+next_pos.ypos;
    current_pos->aoatpos=current_pos->aoatpos+next_pos.aoatpos;
    current_pos->aoabpos=current_pos->aoabpos+next_pos.aoabpos;

}

void HomeAll(struct PositionStruct *current_pos, struct Error *error)
{
    //// Move all the axis 3 mm forward (Yes This lends itself to the potential of the axis moving beyond what is specified )
    //// This ensures that all the axis are not allready on their limit swtiches
    if (digitalRead(PG6) == HIGH)
    {
        x0_Stepper.setupRelativeMoveInSteps(10 / 5 * 200 * 8); // Future: Make these iun terms of MM
        x1_Stepper.setupRelativeMoveInSteps(10 / 5 * 200 * 8); // Future: Make these iun terms of MM
    }
    if (digitalRead(PG12) == HIGH || digitalRead(PG9) == HIGH || digitalRead(PG13) == HIGH || digitalRead(PG10) == HIGH) // make sure the end stop isnt allready pressed
    {
        y0_Stepper.setupRelativeMoveInSteps(15 / 2 * 200 * 8);
        y1_Stepper.setupRelativeMoveInSteps(15 / 2 * 200 * 8);
        y2_Stepper.setupRelativeMoveInSteps(15 / 2 * 200 * 8);
        y3_Stepper.setupRelativeMoveInSteps(15 / 2 * 200 * 8);
    }
    // AOAT_Stepper.setupRelativeMoveInSteps(20/.36*8); // handle this later due to them not being able to roatate 360 degrees
    // AOAB_Stepper.setupRelativeMoveInSteps(20/.36*8);
    while ((!x0_Stepper.motionComplete()) || (!y0_Stepper.motionComplete()) || (!y1_Stepper.motionComplete()) || (!y2_Stepper.motionComplete()) || (!y3_Stepper.motionComplete()) || (!aoat_Stepper.motionComplete()) || (!aoab_Stepper.motionComplete()))
    {
        if (DEV_constants::Endstop_bypass_enable == true)
        {
            break;
        }
        x0_Stepper.processMovement();
        x1_Stepper.processMovement();
        y0_Stepper.processMovement();
        y1_Stepper.processMovement(); // moving the Steppers here was a simple soltuion to having to do speical system intrrupts
        y2_Stepper.processMovement();
        y3_Stepper.processMovement();
        // aoat_Stepper.processMovement();
        // aoab_Stepper.processMovement();
    }
    Serial.print("got through the firt part of home all");
    x0home = false; // we are now garenteed to be at least 5 off the axis
    // x1home= false;
    y0home = false;
    y2home = false;
    y2home = false;
    y3home = false;
    aoathome = false;
    aoabhome = false;
    // Refrencing the block diagram of the stm32f446 on page 16 of the pfd to understand the ports refrenced below
    // a quick guide can be found here: https://gist.github.com/iwalpola/6c36c9573fd322a268ce890a118571ca#brr---bit-reset-register
    /*
    PinPrefix -> Port Name -
    PA -> GPIO port A
    PB -> GPIO port B
    PC -> GPIO port C
    PD -> GPIO port D
    PE -> GPIO port E
    */
    LL_GPIO_SetOutputPin(GPIOF, LL_GPIO_PIN_12);
    LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_1);
    LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_3);
    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_1);
    LL_GPIO_SetOutputPin(GPIOF, LL_GPIO_PIN_10);
    LL_GPIO_SetOutputPin(GPIOF, LL_GPIO_PIN_0); // reset pins to default state
    LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_3); // reset pins to default state
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_14);

    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
    LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_13);
    LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_0);
    LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_11);
    LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_4);
    LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_9); // reset pins to default state
    LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_2);
    LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_6);
    delay(10);
    while (x0home == false || y0home == false || y1home == false || y2home == false || y3home == false) // || aoathome == false) || aoabhome == false||
    {                                                                                                   // While they arent hit the end stop we move the motors
        if (DEV_constants::Endstop_bypass_enable == true)
        {
            break;
        }
        if (x0home == false)
        {
            // The X axis is not home
            LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_13);
            LL_GPIO_TogglePin(GPIOE, LL_GPIO_PIN_2);
        }
        // if (xhome == false) // changed so the x axis only uses on enstop
        // {
        //   // The X axis is home
        //   // motorgpiof=motorgpiof-0b0010000000000000; // remove the 13th digit
        // }
        if (y0home == false)
        {
            // motorgpiog=motorgpiog-0b0000000000000001; // remove pg0
            // motorgpiof=motorgpiof-0b0000100000000000; // remove pf11
            LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_11);
        }
        if (y1home == false)
        {
            LL_GPIO_TogglePin(GPIOG, LL_GPIO_PIN_0);
        }
        if (y2home == false)
        {
            // motorgpiog=motorgpiog-0b0000000000000001; // remove pg0
            // motorgpiof=motorgpiof-0b0000100000000000; // remove pf11
            LL_GPIO_TogglePin(GPIOG, LL_GPIO_PIN_4);
        }
        if (y3home == false)
        {
            // motorgpiog=motorgpiog-0b0000000000000001; // remove pg0
            // motorgpiof=motorgpiof-0b0000100000000000; // remove pf11
            LL_GPIO_TogglePin(GPIOE, LL_GPIO_PIN_6);
        }
        // if (aoathome == false)
        // {
        //   // motorgpiog=motorgpiog-0b0000000000010000;
        //   LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_9);
        // }
        // if (aoabhome == false)
        // {
        //   // motorgpiof=motorgpiof-0b0000001000000000;
        //   LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
        // }
        delayMicroseconds(10); // delay between high and low (Aka how long the pin is high)
        // reset pins to default state (Low), if it wastn triggered to high above it will remain at low
        // LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
        LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_13);
        LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_0);
        LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_11);
        LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_4);
        // LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_9); // reset pins to default state
        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_2);
        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_6);
        delayMicroseconds(280); // delay between high states, how long between step signals
                                // Serial.print("Hl");// debug to make sure it got here // kept short to minimize time
    }
    // begin AoA Homing
    int Thomecount = 160 * 8; // # nuber of possbile steps to make a half circle
    int Bhomecount = 160 * 8;
    LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_10);
    LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_0);
    while (aoathome == false || aoabhome == false)
    { // While they arent hit the end stop we move the motors
        if (DEV_constants::Endstop_bypass_enable == true)
        {
            break;
        }
        if (aoathome == false && Thomecount > 0)
        {
            // motorgpiog=motorgpiog-0b0000000000010000;
            LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_9);
            Thomecount = Thomecount - 1;
        }
        else // if it doesnt dint it going forward switch directions and go a back.
        {
            LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_10); // switch directions to slwitch directions
            delayMicroseconds(5);
            LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_9);
            Thomecount = 200 * 8;
        }
        if (aoabhome == false && Bhomecount > 0)
        {
            // motorgpiof=motorgpiof-0b0000001000000000;
            LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
            Bhomecount = Bhomecount - 1;
        }
        else
        {
            LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_0);
            delayMicroseconds(5);
            LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
            Bhomecount = 200 * 8;
        }
        delayMicroseconds(5); // delay between high and low (Aka how long the pin is high)
        // reset pins to default state (Low), if it wastn triggered to high above it will remain at low
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
        LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_9); // reset pins to default state
        delayMicroseconds(500);
    }

    current_pos->xpos = Settings::X_POSITION_MAX; // Its at the top of its stoke
    current_pos->ypos = Settings::Y_POSITION_MAX;
    current_pos->aoatpos = Settings::AOA_T_HOME_OFFSET;
    current_pos->aoabpos = Settings::AOA_B_HOME_OFFSET;

    volatile bool xhome = false;
    volatile bool yhome = false;
    volatile bool aoathome = false; // second it leaves this function we assume its not home
    volatile bool aoabhome = false;

    struct PositionStruct *input_data_ptr, input_data;
    initialize_Movement_Struct_NC(input_data_ptr);

    MOVE_FUNCTION(current_pos,input_data_ptr,error);
    

    // TODO: Call the move function here and bring the wings back to level and stacked against eachtoher
}
