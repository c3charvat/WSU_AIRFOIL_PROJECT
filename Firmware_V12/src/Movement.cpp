#include "Movement.hpp"
#include "Settings.hpp"
#include "Data_structures.h"

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

float ABS_POS(struct PositionStruct *current_pos, struct PositionStruct *next_pos)
{
    float current_position_data[4]={0,0,0,0};
    float next_position_data[4]={0,0,0,0};
    // break out the struct to individual varibles
    current_position_data[0] = current_pos->xpos;
    current_position_data[1] = current_pos->ypos;
    current_position_data[2] = current_pos->aoatpos;
    current_position_data[3] = current_pos->aoabpos;

    next_position_data[0] = next_pos->xpos;
    next_position_data[1] = next_pos->ypos;
    next_position_data[2] = next_pos->aoatpos;
    next_position_data[3] = next_pos->aoabpos;

    bool bounds_check_flag=false;
    for (size_t i = 0; i < 3; i++)
    {
        bounds_check_flag=bounds_check(Settings::MinMaxArray[i*2] ,Settings::MinMaxArray[(i*2)+1],next_position_data[i]);
        if(bounds_check_flag==false)
        {
            //Display Bounds Error
            
            return 0; // Dont move
        }    
    }

    
}
