#include <Arduino.h>
#include <SpeedyStepper.h>

int BUTTON = 35; // encoder click on Creality Melzi screen
int BEEPER = 27; // factory beeper on Creality Melzi screen
const int MOTOR0_STEP_PIN = A0;  // X axis 
const int MOTOR0_DIRECTION_PIN = A1;  // X axis
const int MOTOR0_ENABLE = 38;  // X axis
const int MOTOR1_STEP_PIN = A6;  // Y axis "AoA Bottom"
const int MOTOR1_DIRECTION_PIN = A7;  // Y axis
const int MOTOR1_ENABLE = A2;  // Y axis
const int MOTOR2_STEP_PIN = 46;  // Z axis  "X motion"
const int MOTOR2_DIRECTION_PIN = 48;  // Z axis
const int MOTOR2_ENABLE = A8;  // Z axis
const int MOTOR3_STEP_PIN = 26;  // E0 axis "Y motion"
const int MOTOR3_DIRECTION_PIN = 28;  // E0 axis
const int MOTOR3_ENABLE = 24;  // E0 axis;
const int MOTOR4_STEP_PIN = 36;  // z axis "y motion
const int MOTOR4_DIRECTION_PIN  = 34;  // z axis // extra setpper for new mtoherboard
const int MOTOR4_ENABLE= 30;  // z axis;



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
pinMode(17, INPUT); // Encoder Move Direction
pinMode(23, INPUT); // Encoder Move Direction 
// Stepper Enables 
digitalWrite(MOTOR0_ENABLE , LOW); // Set the Enable Pin to Low to Enable the Driver 
digitalWrite(MOTOR0_ENABLE , LOW);
digitalWrite(MOTOR0_ENABLE , LOW);
digitalWrite(MOTOR0_ENABLE , LOW);
digitalWrite(MOTOR0_ENABLE , LOW);
// Connect the Stepper Library To the Correct Pins 
Xstepper.connectToPins(MOTOR0_STEP_PIN, MOTOR0_DIRECTION_PIN);
Ystepper.connectToPins(MOTOR1_STEP_PIN, MOTOR1_DIRECTION_PIN);
Zstepper.connectToPins(MOTOR2_STEP_PIN, MOTOR2_DIRECTION_PIN);
E0stepper.connectToPins(MOTOR3_STEP_PIN, MOTOR3_DIRECTION_PIN);
E1stepper.connectToPins(MOTOR4_STEP_PIN, MOTOR4_DIRECTION_PIN);

}