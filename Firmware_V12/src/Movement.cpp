#include "Movement.hpp"
#include "Data_structures.h"

void initialize_Movement_Struct(struct PositionStruct *pos, struct ControlStruct *control){
    pos->xpos=0;
    pos->ypos=0;
    pos->aoatpos=0;
    pos->aoabpos=0;
    control->wificonnected = false;
    control->usbconnected =false;
    pos->source=control; // goal here is to create a link to a single structre that will update with what interface has control
}