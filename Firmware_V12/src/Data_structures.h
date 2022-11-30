#include "Arduino.h"
#ifndef DATASTRUCT_H
#define DATASTRUCT_H

///// structure definitions 
/// Packet structs (To be sent)
struct __attribute__((packed)) ConnectStatusStruct{
  bool wifi;
  bool usb;
};

struct __attribute__((packed)) ControlStruct{ // store which com method has control // make this an enum?
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


/// Internal Structs

struct Error {
  String error_name;
  String error_information_1;
  String error_information_2;
  String error_information_3;
  String originating_function_name;
  bool recoverable;
};

struct DriverSettings{
  int xdirver_microstepping;
  int ydirver_microstepping;
  int aoatdirver_microstepping;
  int aoabdirver_microstepping;
};
#endif