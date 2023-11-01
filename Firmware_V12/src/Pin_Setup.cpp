#include <Arduino.h>
#include <SpeedyStepper.h>
#include <Settings.hpp>
#include <Pin_Setup.hpp>
/*
This file sets up the Pin Modes
It its important to note that Stepper driver 7 shares pins with the swd interface...
So if you are not programming with dfu mode nothing will actuall program and wverthing ill result in errors
The SwD interface will not work untill a full power off and the SwD pins have been freed from any use in code.
 https://www.st.com/resource/en/application_note/cd00167594-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf
 Page 54 for more information
*/
// LCD SETUP
const int BUTTON = PE7;      // encoder click on Creality Melzi screen
const int BEEPER = PE8;      // factory beeper on Creality Melzi screen
const int ENCODER_RT = PE9;  // left turn on the encoder //lcd encoder
const int ENCODER_LT = PE12; // Right trun on the encoder

// RS485 is going to use the exp2 headder 
const int RS485_READ_ENABLE = PE9;
const int RS485_WRITE_ENABLE = PE12;


// Fan setup
const int FAN0 = PA8;
const int FAN1 = PE5;
const int FAN2 = PD12;

// Motor 0
const int MOTOR0_STEP_PIN = PF13;      // X axis X motion
const int MOTOR0_DIRECTION_PIN = PF12; // X axis
const int MOTOR0_ENABLE = PF14;        // X axis
// Motor 1
const int MOTOR1_STEP_PIN = PG0;      // Y axis Y motion
const int MOTOR1_DIRECTION_PIN = PG1; // Y axis
const int MOTOR1_ENABLE = PF15;       // Y axis
// Motor 2
const int MOTOR2_STEP_PIN = PF11;     // Z axis y motion
const int MOTOR2_DIRECTION_PIN = PG3; // Z axis
const int MOTOR2_ENABLE = PG5;        // Z axis
// Motor 3
const int MOTOR3_STEP_PIN = PG4;      // E0 axis AoAt
const int MOTOR3_DIRECTION_PIN = PC1; // E0 axis
const int MOTOR3_ENABLE = PA0;        // E0 axis;
// Motor 4
const int MOTOR4_STEP_PIN = PF9;       // e1 axis AoAB
const int MOTOR4_DIRECTION_PIN = PF10; // e1 axis //
const int MOTOR4_ENABLE = PG2;         // e1 axis;
// Motor 5
const int MOTOR5_STEP_PIN = PC13;     // z axis
const int MOTOR5_DIRECTION_PIN = PF0; // z axis
const int MOTOR5_ENABLE = PF1;        // z axis;
// Motor 6
const int MOTOR6_STEP_PIN = PE2;      // z axis
const int MOTOR6_DIRECTION_PIN = PE3; // z axis
const int MOTOR6_ENABLE = PD4;        // z axis;
// Motor 7
const int MOTOR7_STEP_PIN = PE6;  // z axis
const int MOTOR7_DIRECTION_PIN  = PA14;  // z axis  // disabled for SWD programming
const int MOTOR7_ENABLE= PE0;  // z axis;

// Limit switches

const int Motor0LimitSw = PG6;
const int Motor1LimitSw = PG12;
const int Motor2LimitSw = PG9;
const int Motor3LimitSw = PG13;
const int Motor4LimitSw = PG10;
const int Motor5LimitSw = PG14;
const int Motor6LimitSw = PG11;
const int Motor7LimitSw = PG15;


// Interrupts Varible declerations
volatile bool x0home = false;
volatile bool x1home = false;
volatile bool y0home = false;
volatile bool y1home = false;
volatile bool y2home = false;
volatile bool y3home = false;
volatile bool aoathome = false;
volatile bool aoabhome = false;

void pin_setup()
{
    // RESET
    // pinMode(Reset,OUTPUT);
    // digitalWrite(Reset,HIGH);

    // RS485
    pinMode(RS485_READ_ENABLE, OUTPUT);
    pinMode(RS485_WRITE_ENABLE, OUTPUT);

    // X Stepper
    pinMode(MOTOR0_STEP_PIN, OUTPUT);
    pinMode(MOTOR0_DIRECTION_PIN, OUTPUT);
    pinMode(MOTOR0_ENABLE, OUTPUT);
    // Y0 Stepper
    pinMode(MOTOR1_STEP_PIN, OUTPUT);
    pinMode(MOTOR1_DIRECTION_PIN, OUTPUT);
    pinMode(MOTOR1_ENABLE, OUTPUT);
    // "Z" Y Stepper 1// Hardware Doubled
    pinMode(MOTOR2_STEP_PIN, OUTPUT);
    pinMode(MOTOR2_DIRECTION_PIN, OUTPUT);
    pinMode(MOTOR2_ENABLE, OUTPUT);
    // Y2
    pinMode(MOTOR3_STEP_PIN, OUTPUT);
    pinMode(MOTOR3_DIRECTION_PIN, OUTPUT);
    pinMode(MOTOR3_ENABLE, OUTPUT);
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
    if (DevConstants::SWD_PROGRAMING_MODE == false)
    {
        pinMode(MOTOR7_STEP_PIN, OUTPUT);
        pinMode(MOTOR7_DIRECTION_PIN, OUTPUT);
        pinMode(MOTOR7_ENABLE, OUTPUT);
    }
    // Limit Switches
    pinMode(Motor0LimitSw, INPUT);
    pinMode(Motor1LimitSw, INPUT);
    pinMode(Motor2LimitSw, INPUT);
    pinMode(Motor3LimitSw, INPUT);
    pinMode(Motor4LimitSw, INPUT);
    pinMode(Motor5LimitSw, INPUT);
    pinMode(Motor6LimitSw, INPUT);
    if(DevConstants::SWD_PROGRAMING_MODE == false){
        pinMode(Motor7LimitSw, INPUT);
    }
    //  Extras
    //  pinMode(4, OUTPUT); // Fan Pin Initilization
    pinMode(BEEPER, OUTPUT);    // Beeper on LCD
    pinMode(BUTTON, INPUT);     // Encoder button
    pinMode(ENCODER_RT, INPUT); // Encoder Move Direction
    pinMode(ENCODER_LT, INPUT); // Encoder Move Direction

    // Fan Stuff
    pinMode(FAN0, OUTPUT);
    pinMode(FAN1, OUTPUT);
    pinMode(FAN2, OUTPUT);
    digitalWrite(FAN0, HIGH);
    digitalWrite(FAN1, HIGH);
    digitalWrite(FAN2, HIGH);
    /*
    Fan 6 and 7 are on by default.
    The remaining fan pins can be reused the top pin is hot as defined by the jumpers see pin diagram.
    */
    // Stepper Enables
    digitalWrite(MOTOR0_ENABLE, LOW); // Set the Enable Pin to Low to Enable the Driver
    digitalWrite(MOTOR1_ENABLE, LOW);
    digitalWrite(MOTOR2_ENABLE, LOW);
    digitalWrite(MOTOR3_ENABLE, LOW);
    digitalWrite(MOTOR4_ENABLE, LOW);
    digitalWrite(MOTOR5_ENABLE, LOW);
    digitalWrite(MOTOR6_ENABLE, LOW); // Extras disabled for now
    if(DevConstants::SWD_PROGRAMING_MODE == false){
        digitalWrite(MOTOR7_ENABLE , LOW);
    }
    // digitalWrite(BEEPER, HIGH); // test if board is running code without lcd
    // Connect the Stepper Library To the Correct Pins

    // Attach intrrupts 
    attachInterrupt(digitalPinToInterrupt(Motor0LimitSw), x0HomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor1LimitSw), y0HomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor2LimitSw), y1HomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor3LimitSw), y2HomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor4LimitSw), y3HomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor5LimitSw), aoatHomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor6LimitSw), aoabHomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor7LimitSw), x1HomeIsr, CHANGE);

}

void driver_setup()
{
  gDriverX.beginSerial(115200); // X driver Coms begin
  Serial.println("Driver X Enabled\n");
  gDriverX.begin();
  gDriverX.rms_current(1100); // mA
  gDriverX.microsteps(64);
  //gDriverX.en_spreadCycle(0); // Page 44 use stealth chop
  gDriverX.pwm_ofs_auto ();
  gDriverX.pwm_autograd(1);
  gDriverX.pwm_autoscale(1);
  gDriverX.toff(5);

  gDriverX2.beginSerial(115200);
  Serial.println("Driver X2 Enabled\n");
  gDriverX2.begin();
  gDriverX2.rms_current(1100); // mA
  gDriverX2.microsteps(64);
  gDriverX2.pwm_ofs_auto ();
  gDriverX2.pwm_autograd(1);
  gDriverX2.pwm_autoscale(1);
  gDriverX2.toff(5);

  gDriverY0.beginSerial(115200);
  Serial.println("Driver Y0 Enabled\n");
  gDriverY0.begin();
  gDriverY0.rms_current(900); // mA
  gDriverY0.microsteps(64);
  //gDriverY0.en_spreadCycle(0);
  gDriverY0.pwm_ofs_auto ();
  gDriverY0.pwm_autoscale(1);
  gDriverY0.pwm_autograd(1);
  gDriverY0.toff(5);
  

  gDriverY1.beginSerial(115200);
  Serial.println("Driver Y12 Enabled\n");
  gDriverY1.begin();
  gDriverY1.rms_current(900); // mA
  gDriverY1.microsteps(64);
  gDriverY1.pwm_ofs_auto ();
  gDriverY1.pwm_autograd(1);
  gDriverY1.pwm_autoscale(1);
  gDriverY1.toff(5);

  gDriverY2.beginSerial(115200);
  Serial.println("Driver Y12 Enabled\n");
  gDriverY2.begin();
  gDriverY2.rms_current(900); // mA
  gDriverY2.microsteps(64);
  gDriverY2.pwm_ofs_auto ();
  gDriverY2.pwm_autograd(1);
  gDriverY2.pwm_autoscale(1);
  gDriverY2.toff(5);

  gDriverY3.beginSerial(115200);
  Serial.println("Driver Y3 Enabled\n");
  gDriverY3.begin();
  gDriverY3.rms_current(850); // mA
  gDriverY3.microsteps(64);
  gDriverY3.pwm_ofs_auto ();
  gDriverY3.pwm_autograd(1);
  gDriverY3.pwm_autoscale(1);
  gDriverY3.toff(5);

  gDriverAOAT.beginSerial(115200);
  Serial.println("driver e1 enabled\n");
  gDriverAOAT.begin();
  gDriverAOAT.rms_current(900); // ma
  gDriverAOAT.microsteps(64);
  gDriverAOAT.pwm_ofs_auto ();
  gDriverAOAT.pwm_autograd(1);
  gDriverAOAT.pwm_autoscale(1);
  gDriverAOAT.toff(5);

  gDriverAOAB.beginSerial(115200);
  Serial.println("Driver E2 Enabled\n");
  gDriverAOAB.begin();
  gDriverAOAB.rms_current(900); // mA
  gDriverAOAB.microsteps(64);
  gDriverAOAB.pwm_ofs_auto ();
  gDriverAOAB.pwm_autograd(1);
  gDriverAOAB.pwm_autoscale(1);
  gDriverAOAB.toff(5);

}



// Interrupts 
void x0HomeIsr()
{
  x0home = !x0home; // set set them as hommed when the homing function is called
}
void x1HomeIsr()
{
  x1home = !x1home;
}
void y0HomeIsr()
{
  y0home = !y0home;
}
void y1HomeIsr()
{
  y1home = !y1home;
}
void y2HomeIsr()
{
  y2home = !y2home;
}
void y3HomeIsr()
{
  y3home = !y3home;
}

//#ifndef Has_rs485_ecoders
void aoatHomeIsr()
{
  aoathome = !aoathome;
}
void aoabHomeIsr()
{
  aoabhome = !aoabhome;
}

//#endif
// void motionTriggerIsr()
// {
//   Go_Pressed = true;
// }
void estopIsr()
{
  NVIC_SystemReset(); // use a software reset to kill the board
}