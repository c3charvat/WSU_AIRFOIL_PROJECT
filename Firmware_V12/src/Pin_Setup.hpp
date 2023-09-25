#include<TMCStepper.h>
/* 
This file sets up the Pin Modes
It its important to note that Stepper driver 7 shares pins with the swd interface... 
So if you are not programming with dfu mode nothing will actuall program and wverthing ill result in errors
The SwD interface will not work untill a full power off and the SwD pins have been freed from any use in code. 
 https://www.st.com/resource/en/application_note/cd00167594-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf
 Page 54 for more information
*/

#ifndef PIN_CONSTANTS_H
#define PIN_CONSTANTS_H


// Misc connections 
extern const int BUTTON; // encoder click on Creality Melzi screen
extern const int BEEPER; // factory beeper on Creality Melzi screen
extern const int ENCODER_RT; // left turn on the encoder 
extern const int ENCODER_LT; // Right trun on the encoder

// RS485 Pin setup

extern const int RS485_READ_ENABLE;
extern const int RS485_WRITE_ENABLE;


// Fan setup
extern const int FAN0;
extern const int FAN1;
extern const int FAN2; 


// Motor 0
extern const int MOTOR0_STEP_PIN;  // X axis X motion
extern const int MOTOR0_DIRECTION_PIN;  // X axis
extern const int MOTOR0_ENABLE;  // X axis
// Motor 1
extern const int MOTOR1_STEP_PIN;  // Y axis Y motion
extern const int MOTOR1_DIRECTION_PIN;  // Y axis
extern const int MOTOR1_ENABLE;  // Y axis
// Motor 2
extern const int MOTOR2_STEP_PIN;  // Z axis y motion
extern const int MOTOR2_DIRECTION_PIN;  // Z axis
extern const int MOTOR2_ENABLE;  // Z axis
// Motor 3
extern const int MOTOR3_STEP_PIN;  // E0 axis AoAt
extern const int MOTOR3_DIRECTION_PIN;  // E0 axis
extern const int MOTOR3_ENABLE;  // E0 axis;
// Motor 4
extern const int MOTOR4_STEP_PIN;  // e1 axis AoAB
extern const int MOTOR4_DIRECTION_PIN;  // e1 axis // 
extern const int MOTOR4_ENABLE;  // e1 axis;
//Motor 5
extern const int MOTOR5_STEP_PIN;  // z axis
extern const int MOTOR5_DIRECTION_PIN;  // z axis 
extern const int MOTOR5_ENABLE;  // z axis;
// Motor 6
extern const int MOTOR6_STEP_PIN;  // z axis 
extern const int MOTOR6_DIRECTION_PIN;  // z axis 
extern const int MOTOR6_ENABLE;  // z axis;
// Motor 7
extern const int MOTOR7_STEP_PIN;  // z axis 
extern const int MOTOR7_DIRECTION_PIN;  // z axis  // disabled for SWD programming 
extern const int MOTOR7_ENABLE; // z axis;

// limit switches
extern const int Motor0LimitSw;
extern const int Motor1LimitSw;
extern const int Motor2LimitSw;
extern const int Motor3LimitSw;
extern const int Motor4LimitSw;
extern const int Motor5LimitSw;
extern const int Motor6LimitSw;
extern const int Motor7LimitSw;

// Interrupts Varible declerations
extern volatile bool x0home;
extern volatile bool x1home;
extern volatile bool y0home;
extern volatile bool y1home;
extern volatile bool y2home;
extern volatile bool y3home;
extern volatile bool aoathome;
extern volatile bool aoabhome;


// driver classes
extern TMC2209Stepper driver_X;
extern TMC2209Stepper driver_X2;
extern TMC2209Stepper driver_Y0;
extern TMC2209Stepper driver_Y1;
extern TMC2209Stepper driver_Y2;
extern TMC2209Stepper driver_Y3;
extern TMC2209Stepper driver_AOAT;
extern TMC2209Stepper driver_AOAB;



// functions 

void pin_setup();
void driver_setup();
void x0HomeIsr();
void x1HomeIsr();
void y0HomeIsr();
void y1HomeIsr();
void y2HomeIsr();
void y3HomeIsr();
void aoatHomeIsr();
void aoabHomeIsr();
void motionTriggerIsr();
void estopIsr();

#endif