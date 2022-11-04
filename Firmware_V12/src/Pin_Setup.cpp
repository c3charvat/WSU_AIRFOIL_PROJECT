#include <Arduino.h>
#include <SpeedyStepper.h>
#include <Pin_Setup.h>
/* 
This file sets up the Pin Modes
It its important to note that Stepper driver 7 shares pins with the swd interface... 
So if you are not programming with dfu mode nothing will actuall program and wverthing ill result in errors
The SwD interface will not work untill a full power off and the SwD pins have been freed from any use in code. 
 https://www.st.com/resource/en/application_note/cd00167594-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf
 Page 54 for more information
*/
const int BUTTON = PE7; // encoder click on Creality Melzi screen
const int BEEPER = PE8; // factory beeper on Creality Melzi screen
const int ENCODER_RT = PE9; // left turn on the encoder 
const int ENCODER_LT = PE12; // Right trun on the encoder

// Fan setup
const int Fan0 = PA8;
const int Fan1 = PE5;
const int Fan2 = PD12; 


// Motor 0
const int MOTOR0_STEP_PIN = PF13 ;  // X axis X motion
const int MOTOR0_DIRECTION_PIN = PF12;  // X axis
const int MOTOR0_ENABLE = PF14;  // X axis
// Motor 1
const int MOTOR1_STEP_PIN = PG0;  // Y axis Y motion
const int MOTOR1_DIRECTION_PIN = PG1;  // Y axis
const int MOTOR1_ENABLE = PF15;  // Y axis
// Motor 2
const int MOTOR2_STEP_PIN = PF11;  // Z axis y motion
const int MOTOR2_DIRECTION_PIN = PG3;  // Z axis
const int MOTOR2_ENABLE = PG5;  // Z axis
// Motor 3
const int  MOTOR3_STEP_PIN = PG4;  // E0 axis AoAt
const int  MOTOR3_DIRECTION_PIN = PC1;  // E0 axis
const int  MOTOR3_ENABLE = PA0;  // E0 axis;
// Motor 4
const int  MOTOR4_STEP_PIN = PF9;  // e1 axis AoAB
const int  MOTOR4_DIRECTION_PIN  = PF10;  // e1 axis // 
const int  MOTOR4_ENABLE= PG2;  // e1 axis;
//Motor 5
const int  MOTOR5_STEP_PIN = PC13;  // z axis
const int  MOTOR5_DIRECTION_PIN  = PF0;  // z axis 
const int  MOTOR5_ENABLE= PF1;  // z axis;
// Motor 6
const int  MOTOR6_STEP_PIN = PE2;  // z axis 
const int  MOTOR6_DIRECTION_PIN  = PE3;  // z axis 
const int  MOTOR6_ENABLE= PD4;  // z axis;
// Motor 7
//const int * MOTOR7_STEP_PIN = PE6;  // z axis 
//const int * MOTOR7_DIRECTION_PIN  = PA14;  // z axis  // disabled for SWD programming 
//const int * MOTOR7_ENABLE= PE0;  // z axis;


// Limit switches

const int Motor0LimitSw = PG6;
const int Motor1LimitSw = PG12;
const int Motor2LimitSw = PG9;
const int Motor3LimitSw = PG13;
const int Motor4LimitSw = PG10;
const int Motor5LimitSw = PG14;
const int Motor6LimitSw = PG11;
const int Motor7LimitSw = PG15;



void PIN_SETUP(){
// RESET
//pinMode(Reset,OUTPUT);
//digitalWrite(Reset,HIGH);

//X Stepper 
pinMode(MOTOR0_STEP_PIN, OUTPUT);
pinMode(MOTOR0_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR0_ENABLE , OUTPUT);
// Y0 Stepper 
pinMode(MOTOR1_STEP_PIN, OUTPUT);
pinMode(MOTOR1_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR1_ENABLE , OUTPUT);
// "Z" Y Stepper 1// Hardware Doubled
pinMode(MOTOR2_STEP_PIN, OUTPUT);
pinMode(MOTOR2_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR2_ENABLE , OUTPUT);
// Y2 
pinMode(MOTOR3_STEP_PIN, OUTPUT);
pinMode(MOTOR3_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR3_ENABLE , OUTPUT);
// A0A Top
pinMode(MOTOR4_STEP_PIN, OUTPUT);
pinMode(MOTOR4_DIRECTION_PIN, OUTPUT);
pinMode(MOTOR4_ENABLE, OUTPUT);
// AoA Bottom  
pinMode(MOTOR5_STEP_PIN, OUTPUT);
pinMode(MOTOR5_DIRECTION_PIN, OUTPUT);
pinMode(MOTOR5_ENABLE, OUTPUT);
// x2 Motor 
pinMode(MOTOR6_STEP_PIN, OUTPUT);
pinMode(MOTOR6_DIRECTION_PIN, OUTPUT);
pinMode(MOTOR6_ENABLE, OUTPUT);
// Y motor 4 
//pinMode(MOTOR7_STEP_PIN, OUTPUT);
//pinMode(MOTOR7_DIRECTION_PIN, OUTPUT);
//pinMode(MOTOR7_ENABLE, OUTPUT);
// Limit Switches
pinMode(Motor0LimitSw, INPUT);
pinMode(Motor1LimitSw, INPUT);
pinMode(Motor2LimitSw, INPUT);
pinMode(Motor3LimitSw, INPUT);
pinMode(Motor4LimitSw, INPUT);
pinMode(Motor5LimitSw, INPUT);
pinMode(Motor6LimitSw, INPUT);
//pinMode(Motor7LimitSw, INPUT);
// Extras 
// pinMode(4, OUTPUT); // Fan Pin Initilization 
pinMode(BEEPER, OUTPUT); // Beeper on LCD
pinMode(BUTTON, INPUT); // Encoder button
pinMode(ENCODER_RT, INPUT); // Encoder Move Direction
pinMode(ENCODER_LT, INPUT); // Encoder Move Direction

// Fan Stuff
pinMode(Fan0, OUTPUT);
pinMode(Fan1,OUTPUT);
pinMode(Fan2,OUTPUT);
digitalWrite(Fan0,HIGH);
digitalWrite(Fan1,HIGH);
digitalWrite(Fan2,HIGH);
/*
Fan 6 and 7 are on by default.
The remaining fan pins can be reused the top pin is hot as defined by the jumpers see pin diagram. 
*/
// Stepper Enables 
digitalWrite(MOTOR0_ENABLE , LOW); // Set the Enable Pin to Low to Enable the Driver 
digitalWrite(MOTOR1_ENABLE , LOW);
digitalWrite(MOTOR2_ENABLE , LOW);
digitalWrite(MOTOR3_ENABLE , LOW);
digitalWrite(MOTOR4_ENABLE , LOW);
digitalWrite(MOTOR5_ENABLE , LOW);
digitalWrite(MOTOR6_ENABLE , LOW); // Extras disabled for now
//digitalWrite(MOTOR7_ENABLE , LOW);

// digitalWrite(BEEPER, HIGH); // test if board is running code without lcd
// Connect the Stepper Library To the Correct Pins 
}