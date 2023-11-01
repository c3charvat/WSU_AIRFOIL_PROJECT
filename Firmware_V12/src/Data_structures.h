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

/*
Pointer refrence:

string* hello(string* charBuffer){
	cout<< charBuffer << "\n"; //address
    cout<< *charBuffer << "\n";//value Pizza
    return charBuffer;// address 
}

int main() {
  string food = "Pizza";
  string* ptr = &food;
  
  cout << hello(&food);


// Output the value of food (Pizza)
cout << food << "\n";

// Output the memory address of food (0x6dfed4)
cout << &food << "\n";

// Access the memory address of food and output its value (Pizza)
cout << *ptr << "\n";

// Change the value of the pointer
*ptr = "Hamburger";

// Output the new value of the pointer (Hamburger)
cout << *ptr << "\n";

// Output the new value of the food variable (Hamburger)
cout << food << "\n";

// create the data strucures
  // ConnectStatusStruct *Connectionstatus_ptr, Connectionstatus; // initalise a pointer to a strcut of connect test and
  // Connectionstatus_ptr = &Connectionstatus;
  // ControlStruct *Source_ptr, Source;
  // Source_ptr = &Source;
  // PositionStruct *RecievedData_ptr, RecievedData;
  // RecievedData_ptr = &RecievedData;
  // PositionStruct *CurrentPostions_ptr, CurrentPostions;
  // CurrentPostions_ptr = &CurrentPostions;

  // // Error Struct

  // Error *Error_ptr, Error_struct;
  // Error_ptr = &Error_struct;

  // // initilize the structures
  // initialize_movement_struct(CurrentPostions_ptr, Source_ptr);
  // initialize_movement_struct(RecievedData_ptr, Source_ptr);


*/