#include "Arduino.h"
#ifndef DATASTRUCT_H
#define DATASTRUCT_H

// structure definitions 
struct __attribute__((packed)) ConnectTestStruct{
  char name;
  bool connected;
};

struct __attribute__((packed)) ControlStruct{ // store which com method has control
    bool usbconnected;
    bool wificonnected;
  //(wireless or usb)
}; // keep track of 


struct __attribute__((packed)) PositionStruct {
  float xpos;
  float ypos;
  float aoatpos;
  float aoabpos;
  struct ControlStruct* source; // pointer to the control source struct
};

//typedef struct PositionStruct positionstruct; // if i decide to make it a tyoe later



#endif