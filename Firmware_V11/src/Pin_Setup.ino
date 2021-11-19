#include <Arduino.h>
#include <SpeedyStepper.h>
/* 
This file sets up the Pin Modes
*/
int BUTTON = PE7; // encoder click on Creality Melzi screen
int BEEPER = PE8; // factory beeper on Creality Melzi screen
int ENCODER_RT = PE9; // left turn on the encoder 
int ENCODER_LT = PE12; // Right trun on the encoder
// Motor 0
const int MOTOR0_STEP_PIN = PF13 ;  // X axis 
const int MOTOR0_DIRECTION_PIN = PF12;  // X axis
const int MOTOR0_ENABLE = PF14;  // X axis
// Motor 1
const int MOTOR1_STEP_PIN = PG0;  // Y axis "AoA Bottom"
const int MOTOR1_DIRECTION_PIN = PG1;  // Y axis
const int MOTOR1_ENABLE = PF15;  // Y axis
// Motor 2
const int MOTOR2_STEP_PIN = PF11;  // Z axis  "X motion"
const int MOTOR2_DIRECTION_PIN = PG3;  // Z axis
const int MOTOR2_ENABLE = PG5;  // Z axis
// Motor 3
const int MOTOR3_STEP_PIN = PG4;  // E0 axis "Y motion"
const int MOTOR3_DIRECTION_PIN = PC1;  // E0 axis
const int MOTOR3_ENABLE = PA0;  // E0 axis;
// Motor 4
const int MOTOR4_STEP_PIN = PF9;  // z axis "y motion
const int MOTOR4_DIRECTION_PIN  = PF10;  // z axis // extra setpper for new mtoherboard
const int MOTOR4_ENABLE= PG2;  // z axis;
// Motor 5
const int MOTOR5_STEP_PIN = PC13;  // z axis "y motion
const int MOTOR5_DIRECTION_PIN  = PF0;  // z axis // extra setpper for new mtoherboard
const int MOTOR5_ENABLE= PF1;  // z axis;
// Motor 6
const int MOTOR6_STEP_PIN = PE2;  // z axis "y motion
const int MOTOR6_DIRECTION_PIN  = PE3;  // z axis // extra setpper for new mtoherboard
const int MOTOR6_ENABLE= PD4;  // z axis;
// Motor 7
const int MOTOR7_STEP_PIN = PE6;  // z axis "y motion
const int MOTOR7_DIRECTION_PIN  = PA14;  // z axis // extra setpper for new mtoherboard
const int MOTOR7_ENABLE= PE0;  // z axis;

const int Motor0LimitSw =PG6;
const int Motor1LimitSw =PG12;
const int Motor2LimitSw =PG9;
const int Motor3LimitSw =PG13;
const int Motor4LimitSw =PG10;
const int Motor5LimitSw =PG14;
const int Motor6LimitSw =PG11;
const int Motor7LimitSw =PG15;
// Reset Pin -> off of the RGB HEADDER J37 
//const int Reset=PB0;



void PIN_SETUP(){
// RESET
//pinMode(Reset,OUTPUT);
//digitalWrite(Reset,HIGH);

//X Stepper
pinMode(MOTOR0_STEP_PIN, OUTPUT);
pinMode(MOTOR0_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR0_ENABLE , OUTPUT);
// Y Stepper 
pinMode(MOTOR1_STEP_PIN, OUTPUT);
pinMode(MOTOR1_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR1_ENABLE , OUTPUT);
// Z Stepper // Hardware Doubled
pinMode(MOTOR2_STEP_PIN, OUTPUT);
pinMode(MOTOR2_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR2_ENABLE , OUTPUT);
// E0 Stepper A0A top
pinMode(MOTOR3_STEP_PIN, OUTPUT);
pinMode(MOTOR3_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR3_ENABLE , OUTPUT);
// E1 Stepper A0A Bottom
pinMode(MOTOR4_STEP_PIN, OUTPUT);
pinMode(MOTOR4_DIRECTION_PIN, OUTPUT);
pinMode(MOTOR4_ENABLE, OUTPUT);
// E2 Extra
pinMode(MOTOR5_STEP_PIN, OUTPUT);
pinMode(MOTOR5_DIRECTION_PIN, OUTPUT);
pinMode(MOTOR5_ENABLE, OUTPUT);
// E3 Extra
pinMode(MOTOR6_STEP_PIN, OUTPUT);
pinMode(MOTOR6_DIRECTION_PIN, OUTPUT);
pinMode(MOTOR6_ENABLE, OUTPUT);
// E4 Extra
pinMode(MOTOR7_STEP_PIN, OUTPUT);
pinMode(MOTOR7_DIRECTION_PIN, OUTPUT);
pinMode(MOTOR7_ENABLE, OUTPUT);
// Limit Switches
pinMode(Motor0LimitSw, OUTPUT);
pinMode(Motor1LimitSw, OUTPUT);
pinMode(Motor2LimitSw, OUTPUT);
pinMode(Motor3LimitSw, OUTPUT);
pinMode(Motor4LimitSw, OUTPUT);
pinMode(Motor5LimitSw, OUTPUT);
pinMode(Motor6LimitSw, OUTPUT);
pinMode(Motor7LimitSw, OUTPUT);
// Extras 
// pinMode(4, OUTPUT); // Fan Pin Initilization 
pinMode(BEEPER, OUTPUT); // Beeper on LCD
pinMode(BUTTON, INPUT); // Encoder button
pinMode(ENCODER_RT, INPUT); // Encoder Move Direction
pinMode(ENCODER_LT, INPUT); // Encoder Move Direction 
// Stepper Enables 
digitalWrite(MOTOR0_ENABLE , LOW); // Set the Enable Pin to Low to Enable the Driver 
digitalWrite(MOTOR1_ENABLE , LOW);
digitalWrite(MOTOR2_ENABLE , LOW);
digitalWrite(MOTOR3_ENABLE , LOW);
digitalWrite(MOTOR4_ENABLE , LOW);
//digitalWrite(MOTOR5_ENABLE , LOW);
//digitalWrite(MOTOR6_ENABLE , LOW); // Extras disabled for now
//digitalWrite(MOTOR7_ENABLE , LOW);

// digitalWrite(BEEPER, HIGH); // test if board is running code without lcd
// Connect the Stepper Library To the Correct Pins 
Xstepper.connectToPins(MOTOR0_STEP_PIN, MOTOR0_DIRECTION_PIN);
Ystepper.connectToPins(MOTOR1_STEP_PIN, MOTOR1_DIRECTION_PIN);
Zstepper.connectToPins(MOTOR2_STEP_PIN, MOTOR2_DIRECTION_PIN);
E0stepper.connectToPins(MOTOR3_STEP_PIN, MOTOR3_DIRECTION_PIN);
E1stepper.connectToPins(MOTOR4_STEP_PIN, MOTOR4_DIRECTION_PIN);
E2stepper.connectToPins(MOTOR5_STEP_PIN, MOTOR5_DIRECTION_PIN);
E3stepper.connectToPins(MOTOR6_STEP_PIN, MOTOR6_DIRECTION_PIN);
E4stepper.connectToPins(MOTOR7_STEP_PIN, MOTOR7_DIRECTION_PIN);
}