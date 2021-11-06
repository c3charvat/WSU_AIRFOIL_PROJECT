#include <Arduino.h>
#include <SpeedyStepper.h>
/* 
This file sets up the Pin Modes
*/
int BUTTON = PE7; // encoder click on Creality Melzi screen
int BEEPER = PE8; // factory beeper on Creality Melzi screen
int ENCODER_RT = PE9; // left turn on the encoder 
int ENCODER_LT = PE12; // Right trun on the encoder
const int MOTOR0_STEP_PIN = PF13 ;  // X axis 
const int MOTOR0_DIRECTION_PIN = PF12;  // X axis
const int MOTOR0_ENABLE = PF14;  // X axis
const int MOTOR1_STEP_PIN = PG0;  // Y axis "AoA Bottom"
const int MOTOR1_DIRECTION_PIN = PG1;  // Y axis
const int MOTOR1_ENABLE = PF15;  // Y axis
const int MOTOR2_STEP_PIN = PF11;  // Z axis  "X motion"
const int MOTOR2_DIRECTION_PIN = PG3;  // Z axis
const int MOTOR2_ENABLE = PG5;  // Z axis
const int MOTOR3_STEP_PIN = PG4;  // E0 axis "Y motion"
const int MOTOR3_DIRECTION_PIN = PC1;  // E0 axis
const int MOTOR3_ENABLE = PA0;  // E0 axis;
const int MOTOR4_STEP_PIN = PF9;  // z axis "y motion
const int MOTOR4_DIRECTION_PIN  = PF10;  // z axis // extra setpper for new mtoherboard
const int MOTOR4_ENABLE= PG2;  // z axis;



void PIN_SETUP(){
// X Stepper

pinMode(MOTOR0_STEP_PIN, OUTPUT);
pinMode(MOTOR0_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR0_ENABLE , OUTPUT);
// Y Stepper
pinMode(MOTOR1_STEP_PIN, OUTPUT);
pinMode(MOTOR1_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR1_ENABLE , OUTPUT);
// Z Stepper
pinMode(MOTOR2_STEP_PIN, OUTPUT);
pinMode(MOTOR2_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR2_ENABLE , OUTPUT);
// E0 Stepper
pinMode(MOTOR3_STEP_PIN, OUTPUT);
pinMode(MOTOR3_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR3_ENABLE , OUTPUT);
// E1 Stepper
pinMode(MOTOR4_STEP_PIN, OUTPUT);
pinMode(MOTOR4_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR4_ENABLE , OUTPUT);
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
digitalWrite(MOTOR0_ENABLE , LOW);
//SDCard Pins
//pinMode(PC8, INPUT);
//pinMode(PC9, INPUT);
//pinMode(PC10, INPUT); 
//pinMode(PC11, INPUT);
//pinMode(PC12, OUTPUT);
// digitalWrite(BEEPER, HIGH); // test if board is running code without lcd
// Connect the Stepper Library To the Correct Pins 
Xstepper.connectToPins(MOTOR0_STEP_PIN, MOTOR0_DIRECTION_PIN);
Ystepper.connectToPins(MOTOR1_STEP_PIN, MOTOR1_DIRECTION_PIN);
Zstepper.connectToPins(MOTOR2_STEP_PIN, MOTOR2_DIRECTION_PIN);
E0stepper.connectToPins(MOTOR3_STEP_PIN, MOTOR3_DIRECTION_PIN);
E1stepper.connectToPins(MOTOR4_STEP_PIN, MOTOR4_DIRECTION_PIN);
}