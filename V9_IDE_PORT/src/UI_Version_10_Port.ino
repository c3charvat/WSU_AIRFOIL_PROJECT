// Demo #2 Full UI Implemtation
#include <Arduino.h>  // Include Github links here
#include <U8g2lib.h>
#include <SpeedyStepper.h>
#include <TMCStepper.h>
#include <TMCStepper_UTILITY.h>
using namespace TMC2208_n;
#define DRIVER_ADDRESS 0b00

TMC2209Stepper driverX(A9, 40, .11f, DRIVER_ADDRESS ); // (RX, TX,RSENSE) Software serial X axis

#ifdef U8X8_HAVE_HW_SPI // UI Communications protocal not sure if this is neassary or not used from example.
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Physcial System Char~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int Xpos_MAX = 500; // Max X length in MM
int Ypos_MAX = 500;// MAy Y length in MM
// X lead screw pitch implment when the physcal system is desined and ready
// Y lead screw pitch
int AOA_MAX = 500; // Angle of attack max in 360 degrees
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Pin Define~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Define the LCD Pins ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 25, /* data=*/ 29, /* CS=*/ 27, /* reset=*/ 16);
// Define the LCD Type and Pins Reset is currently pin 29 which us unused or unconnected on the board.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MOTION CONTROL PIN DEFINE  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int button = 16; // encoder click on Creality Melzi screen
int beeper = 27; // factory beeper on Creality Melzi screen
const int MOTOR0_STEP_PIN = A0;  // e axis "AoA Top"
const int MOTOR0_DIRECTION_PIN = A1;  // e axis
const int MOTOR0_ENABLE = 38;  // e axis
SpeedyStepper Estepper; // initalize stepper 1
const int MOTOR1_STEP_PIN = A6;  // z axis "AoA Bottom"
const int MOTOR1_DIRECTION_PIN = A7;  // z axis
const int MOTOR1_ENABLE = A2;  // z axis
SpeedyStepper Zstepper;
const int MOTOR2_STEP_PIN = 46;  // x axis  "X motion"
const int MOTOR2_DIRECTION_PIN = 48;  // x axis
const int MOTOR2_ENABLE = A8;  // x axis
SpeedyStepper Xstepper;
const int MOTOR3_STEP_PIN = 26;  // z axis "Y motion"
const int MOTOR3_DIRECTION_PIN = 28;  // z axis
const int MOTOR3_ENABLE = 24;  // z axis;
SpeedyStepper Ystepper;
//const int MOTOR4_STEP_PIN = **;  // z axis "Y motion"
//const int MOTOR4_DIRECTION_PIN = **;  // z axis // Extra setpper for new mtoherboard
//const int MOTOR4_ENABLE = **;  // z axis;
//SpeedyStepper E2stepper;




// Stepper settings
int Stepper_acell[5]; // setpper accleration array initliazation // modified for new motherboard this wont get used though since the extra stepper is going to mirror another axis
int Stepper_speed[5]; // modified for new motherboard this wont get used though since the extra stepper is going to mirror another axis
uint8_t  Acceleration; // for the LCD UI
uint8_t  Speed;        // For the LCD UI
int Micro_stepping[5] = {256, 256, 256, 256, 256}; //mirco stepping for the drivers // E ,Z, X, Y, E2 // modified for new mothermoard
float Degree_per_step[5] = {1.8, 1.8, 1.8, 1.8, 1.8}; //mirco stepping for the drivers // E ,Z, X, Y, E2 // modified for new mothermoard
//

// ~~~~~~~~~~~~~~~~~~~~~~~~~ Trigger Pin Define  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int TRIGGER_PIN = 24; // Temp0 pin on Melzi board


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Serial Input Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const byte numChars = 64;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
float Position_Data[5]; //Temporary varibles to hold the seperated values untill they are assinged corretly // modified for new motherboard
int Speed_Data[5]; // // modified for new motherboard this wont get used though since the extra stepper is going to mirror another axis
int Acell_Data[5];// // modified for new motherboard this wont get used though since the extra stepper is going to mirror another axis
boolean newData = false;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Menu Stuff~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const char *Main_menu =     // Define the Main Menu options
  "A.O.A Top\n"
  "A.O.A Bottom\n"
  "X Movement\n"
  "Y Movement\n"
  "T.Continious->Start\n"
  "Settings";

const char *Setting_list = // Define the Settings Sub menu options
  "Acceleration\n"
  "Speed\n"
  "Movment Mode\n"
  "BACK";

const char *Com_select = // Communcations method select menu
  "SERIAL\n"
  "LCD";
const char *Motion_select = // Communcations method select menu
  "LCD Static\n"
  "LCD Trigger\n"
  "LCD External Trigger\n"
  "Serial Cont.\n"
  "Serial Cont. Ext.T.\n";

const char *Error_String = // Error strings
  "Go Back\n"
  "Main Menu\n";

uint8_t current_selection = 0; // Keep track of current selection in menu
uint8_t Sub_selection = 0;
uint8_t Com_selection = 2; // communications selection tracker by default use the LCD
uint8_t Motion_selection = 1; // Default to Static

int Go_Pressed = 0; // By default the go button does nothing
// ~~~~~~~~~~~~~~~~~~~ Angle of Attack Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float AoA[2]; // floating point for AoA: Top,Bottom
// Passing in unsigned 8 bit intger ( Thats what the fucking UI command wants)
//the max of a 8 bit int is 255 and there are 360 derees ** this willl have to be changed to support up to .05 ) which will require
uint8_t AoA_t_value[3]; // Top AoA: Hundreds,tens/ones,Decimal
uint8_t AoA_b_value[3]; // Bottom AoA: Hundreds,tens/ones,Decimal
// ~~~~~~~~~~~~~~~~~~~~~~~~~~ X Movement Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float Xpos;  // X position float value
uint8_t X_value[3]; // X pos : Hundreds,tens/ones,Decimal
// ~~~~~~~~~~~~~~~~~~~~~~~~~~ Y Movemnt Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float Ypos;  // X position float value
uint8_t Y_value[3]; // Y pos: Hundreds,tens/ones,Decimal
// ~~~~~~~~~~~~~~~~~~~~~~~~~ Absolute Tracking Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float CurrentPositions[5] = {0, 0, 0, 0, 0}; // AoA_Top, AoA_Bottom,X,Y,E2  -> " E Z X Y E2 " // modified for new motherboard
float movevar[5]; // E ,Z, X, Y , E2 // modified for new motherboard this wont get used though since the extra stepper is going to mirror another axis
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`End Variables~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Acelleration/ Speed Set/ Trigger Functions  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SET_ACELL(float e, float z, float x, float y) {
  Estepper.setAccelerationInRevolutionsPerSecondPerSecond(e);
  Zstepper.setAccelerationInStepsPerSecondPerSecond(z);
  Xstepper.setAccelerationInStepsPerSecondPerSecond(x); // Change to mm/s when the system is being implmented
  Ystepper.setAccelerationInStepsPerSecondPerSecond(y); // Change to mm/s when the system is being implmented
  //  E2stepper.setAccelerationInStepsPerSecondPerSecond(*put axis its mirrioring here*);
}
void SET_SPEED(int e, int z, int x, int y) {
  Estepper.setSpeedInRevolutionsPerSecond(e);
  Zstepper.setSpeedInStepsPerSecond(z);
  Xstepper.setSpeedInStepsPerSecond(x); // Change to mm/s^2 when the system is being implmented
  Ystepper.setSpeedInStepsPerSecond(y); // Change to mm/s^2 when the system is being implmented
  //  E2stepper.setSpeedInStepsPerSecond(*Axis its mirrioring here*);
}
void TRIGGER_WAIT(int pin) {
  int Button_state = 0;
  while (1) {
    Button_state = digitalRead(pin); // Basically Just get caught up in here untill the pin is Pulled up
    if (Button_state == HIGH) {
      return;
    }
  }
}

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SETUP lOOP~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup(void) {
  Serial.begin(230400); // Begin serial ouput communication for debug and input of value array.
   driverX.beginSerial(115200);
    Serial.print("got here 1");
    driverX.begin();
    Serial.print("got here 2");
  driverX.begin();
  driverX.rms_current(850); // mA
  driverX.microsteps(64);
  driverX.pwm_ofs_auto ();
  Serial.print("PUT LCD INTO DESIRED MODE AND SERIAL COMMUNCATION -->BEFORE<-- YOU INPUT --->ANYTHING<---!!!\n");
  Serial.print("");
  SET_ACELL(-70, 500, 500, 500); // Set motor acceleration
  SET_SPEED(100, 2000, 200, 200); // Set motor Speed
  Estepper.connectToPins(MOTOR0_STEP_PIN, MOTOR0_DIRECTION_PIN);
  Zstepper.connectToPins(MOTOR1_STEP_PIN, MOTOR1_DIRECTION_PIN);
  Xstepper.connectToPins(MOTOR2_STEP_PIN, MOTOR2_DIRECTION_PIN);
  Ystepper.connectToPins(MOTOR3_STEP_PIN, MOTOR3_DIRECTION_PIN);
  pinMode(beeper, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(38, OUTPUT);
  pinMode(35, INPUT);
  pinMode(17, INPUT);
  pinMode(23, INPUT);
  //pinMode(MOTOR2_ENABLE, OUTPUT);// enabling the pin as an output
  //digitalWrite(4, HIGH);
  //digitalWrite(MOTOR1_ENABLE, LOW);
  digitalWrite(38, LOW);
  u8g2.begin(/* menu_select_pin= */ 35, /* menu_next_pin= */ 17, /* menu_prev_pin= */ 23, /* menu_home_pin= */ 52 );
  // Define the System Font see https://github.com/olikraus/u8g2/wiki/u8g2reference for more information about the commands
  u8g2.setFont(u8g2_font_6x12_tr);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VOID LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop(void) {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ User Interface Code ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  MAIN_MENU(); // issues the main menu command
  //
  //  if ( current_selection == 0 ) {
  //    u8g2.userInterfaceMessage(
  //      "Nothing selected.",
  //      "",
  //      "",
  //      " ok ");
  //  }
  if ( current_selection == 1 ) {
    u8g2.userInterfaceInputValue( "AOA Top:", "", &AoA_t_value[0] , 0, 5 , 1 , " 0-3 Hundreds Degrees");
    u8g2.userInterfaceInputValue( "AOA Top:", "", &AoA_t_value[1] , 0, 99 , 2 , " 0-99 Tens/Ones Degree"); // Error Message needs to be made if the input is made over the max AoA
    u8g2.userInterfaceInputValue( "AOA Top:", "", &AoA_t_value[2] , 0, 9 , 1 , " 0-9 Decimal Degree");
    //  headder,re string, pointer to unsigned char, min value, max vlaue, # of digits , post char
    AoA[0] = AoA_t_value[0] * 100 + AoA_t_value[1] + AoA_t_value[2] / 10; // This is the desierd angle we want in a floting point int.
    // Move function call here
    MOVE_FUNCTION();
    // 200 possible steps per revolution and 1/16 miro stepping meaning a pssiblity of 3,200 possible postions 360*/1.8 degrees/step

    //Serial.print(AoA);
    //Serial.print(H_value); // DEBUG Serial Print out
    //Serial.print(value);-
  }
  if ( current_selection == 2 ) {
    u8g2.userInterfaceInputValue( "AOA Bottom:", "", &AoA_b_value[0] , 0, 3 , 1 , " 0-3 Hundreds Degrees");
    u8g2.userInterfaceInputValue( "AOA Bottom:", "", &AoA_b_value[1] , 0, 99 , 2 , " 0-99 Tens/Ones Degree"); // Error Message needs to be made if the input is made over the max AoA
    u8g2.userInterfaceInputValue( "AOA Bottom:", "", &AoA_b_value[2] , 0, 9 , 1 , " 0-9 Decimal Degree");
    //  headder,re string, pointer to unsigned char, min value, max vlaue, # of digits , post char
    AoA[1] = AoA_b_value[0] * 100 + AoA_b_value[1] + AoA_b_value[2] / 10; // This is the desierd angle we want in a floting point int.
    // move function call here
    MOVE_FUNCTION();
    // 200 possible steps per revolution and 1/16 miro stepping meaning a pssiblity of 3,200 possible postions 360*/1.8 degrees/step
  }
  if ( current_selection == 3 ) {
    // X movement
    //u8g2.userInterfaceInputValue( "X movment:", "", &X_value[0] , 0, 3 , 1 , " *-* Thousands of MM "); // Removed at the request of Dr. Y Functionality preserved
    u8g2.userInterfaceInputValue( "X movment:", "", &X_value[1] , 0, 3 , 1 , " *-* Hundreds of MM ");
    u8g2.userInterfaceInputValue( "X movment:", "", &X_value[2] , 0, 60 , 2 , " *-* Tens/Ones MM ");
    u8g2.userInterfaceInputValue( "X movment:", "", &X_value[3] , 0, 9 , 1 , " *-* Decimal MM ");
    Xpos = X_value[0] * 1000 + X_value[1] * 100 + X_value[2] + X_value[3] / 10; // add the two intgers toghter into a float because jesus its so much easier to work with the intger
    // move function call here
    MOVE_FUNCTION();
  }
  if ( current_selection == 4 ) {
    // Y movemnt
    //u8g2.userInterfaceInputValue( "X movment:", "", &Y_value[0] , 0, 3 , 1 , " *-* Thousands of MM "); // Removed at the request of Dr. Y Functionality preserved
    u8g2.userInterfaceInputValue( "X movment:", "", &Y_value[1] , 0, 3 , 1 , " *-* Hundreds of MM ");
    u8g2.userInterfaceInputValue( "X movment:", "", &Y_value[2] , 0, 60 , 2 , " *-* Tens/Ones MM ");
    u8g2.userInterfaceInputValue( "X movment:", "", &Y_value[3] , 0, 9 , 1 , " *-* Decimal MM ");
    Ypos = Y_value[0] * 1000 + Y_value[1] * 100 + Y_value[2] + Y_value[3] / 10; // add the two intgers toghter into a float because jesus its so much easier to work with the intger
    /// move function call here
    MOVE_FUNCTION();
  }
  if ( current_selection == 5 ) {
    // Go Buttom for The Continious mode
    // create an if statment witha ready bool here So Go only works once per update of the data. in the move function
    u8g2.userInterfaceMessage("", "", "Ready to Go?", " Ok \n Cancel ");
    if (Go_Pressed == 1 ) {
      digitalWrite(27, HIGH);// Sound Buzzer That The Control Borad is waiting on user
      while ((!Estepper.motionComplete()) || (!Zstepper.motionComplete()) || (!Xstepper.motionComplete()) || (!Ystepper.motionComplete())) { // While
        Estepper.processMovement();
        Zstepper.processMovement();
        Xstepper.processMovement();
        Ystepper.processMovement();
        //E2stepper.processMovement();
      }
      digitalWrite(27, LOW);// turn the anoying thing off
      Serial.println("Go Pressed\n ");
      Serial.print(current_selection);
      Go_Pressed = 0;
    }
  }
  if ( current_selection == 6 ) {
    Sub_selection = u8g2.userInterfaceSelectionList(   // Bings up the Main Menu
                      "Settings",
                      Sub_selection,
                      Setting_list);
    //    if ( Sub_selection == 0 ) {
    //      u8g2.userInterfaceMessage(
    //        "Nothing selected.",
    //        "",
    //        "",
    //        " ok ");
    //    }
    if ( Sub_selection == 1 ) {
      // Acceleration Settings Code
      u8g2.userInterfaceInputValue( "Acceleration:", "", Acceleration , 0, 20 , 2 , "*25 Steps/s^2");
      SET_ACELL(Acceleration * 25, Acceleration * 25, Acceleration * 25, Acceleration * 25);
    }
    if ( Sub_selection == 2 ) {
      // Speed Settings
      u8g2.userInterfaceInputValue( "Speed:", "", Speed , 0, 20 , 2 , "*25 Steps/s");
      SET_SPEED(Speed * 25, Speed * 25, Speed * 25, Speed * 25);
    }
    if ( Sub_selection == 3 ) {
      // Motion Select function
      Motion_selection = u8g2.userInterfaceSelectionList("Motion Behavior", 1, Motion_select); // Retuns 1: Normal //2: Continous //3: Continious W. Trigger // 4. Serial Continous // 5. Serial Trigger
      if (Motion_selection == 1) {
        // Static Mode LCD
        //Serial.println("Motion Selection: ");// Debug Stuff
        //Serial.print(Motion_selection);
        MAIN_MENU(); // Retrun back to main menu
      }
      if (Motion_selection == 2) {
        // Continious LCD w. LCD Trigger
        //Serial.println("Motion Selection: ");// Debug Stuff
        //Serial.print(Motion_selection);
        MAIN_MENU();
      }
      if (Motion_selection == 3) {
        //  "Continious With External Trigger
        //Serial.println("Motion Selection: ");// Debug Stuff
        //Serial.print(Motion_selection);
        MAIN_MENU();
      }
      if (Motion_selection == 4) {
        // Serial Cont.\n"
        //Serial.println("Motion Selection: ");// Debug Stuff
        //Serial.print(Motion_selection);
        Com_selection = 1; // switch to serial Comunications
        SERIAL_UI(); // call the serial UI
      }
      if (Motion_selection == 5) {
        // "Serial Cont. W. T.\n";
        //Serial.println("Motion Selection: "); // Debug Stuff
        //Serial.print(Motion_selection);
        Com_selection = 1; // switch to serial Comunications
        SERIAL_UI(); // call the serial UI
      }
    }
    if ( Sub_selection == 4 ) {
      // Head back to Main meanu
      MAIN_MENU();
    }
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ END User Interface Code ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
} // ~~~~ End Main LOOP
