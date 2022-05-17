/* Air Foil Project code
Version 12
Written by: Collin Charvat
liscence: N/A
This program was written to drive the Dual Airfoil experiment at Wright State Uni.
The main gaols of this code are to drive 5 steppers in multiple different modes of operation/
They are:
LCD Static - As Soon as an axis is commanded to move, move and the stay there and hold until otherwise.
LCD Trigger - allow the end user to input muiltple axis destinations then press a menu selection to move all axis at once
LCD w. Ext. Trigger - The same as above, but the menu does nothing instead the system waits on a physical trigger to begin motion
Serial Static- As soon as data is passed in via serial move.
Serial W. Ext. Trigger - allow the user to program the desired destination the press the external trigger to move
*/
/* Libraries
These are sets of pre- Made functions and code that simplifies the code.
*/
#include <Arduino.h> // Include Github links here
#include <U8g2lib.h>
#include <SpeedyStepper.h>
#include <TMCStepper.h>
#include <TMCStepper_UTILITY.h>
#include <stm32yyxx_ll_gpio.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
using namespace TMC2208_n; // Allows the TMC2209 to use functions out of tmc2208 required
#define DRIVER_ADDRESS 0b00
/*
In the Case of this set up since the drivers are in Uart mode the adress of the Driver is zero since each of drivers has their own UART wire
*/
// TMC Stepper Class
TMC2209Stepper driverX(PC4, PA6, .11f, DRIVER_ADDRESS);    // (RX, TX,RSENSE, Driver address) Software serial X axis
TMC2209Stepper driverX2(PE1, PA6, .11f, DRIVER_ADDRESS);    // (RX, TX,RSENSE, Driver address) Software serial X axis
TMC2209Stepper driverY0(PD11, PA6, .11f, DRIVER_ADDRESS);  // (RX, TX,RSENSE, Driver address) Software serial X axis
TMC2209Stepper driverY1(PC6, PA6, .11f, DRIVER_ADDRESS);  // (RX, TX,RSENSE, Driver address) Software serial X axis
TMC2209Stepper driverY2(PD3, PA6, .11f, DRIVER_ADDRESS);  // (RX, TX,RSENSE, Driver address) Software serial X axis
TMC2209Stepper driverY3(PC7, PA6, .11f, DRIVER_ADDRESS);   // (RX, TX,RSENSE, Driver Address) Software serial X axis
TMC2209Stepper driverAOAT(PF2, PA6, .11f, DRIVER_ADDRESS); // (RX, TX,RESENSE, Driver address) Software serial X axis
TMC2209Stepper driverAOAB(PE4, PA6, .11f, DRIVER_ADDRESS); // (RX, TX,RESENSE, Driver address) Software serial X axis
// TMC2209Stepper driverE3(PE1, PA6, .11f, DRIVER_ADDRESS ); // (RX, TX,RESENSE, Driver address) Software serial X ax
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Physcial System Char~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* In This section are the maximum travel distances for each of the axis */
int Micro_stepping[5] = {64, 64, 64, 64, 64};         // mirco stepping for the drivers
float Degree_per_step[5] = {1.8, 1.8, 1.8, 1.8, 1.8}; // mirco stepping for the drivers
const int Xpos_MAX = 350;                             // Max X length in MM
const int Ypos_MAX = 245;                             // MAy Y length in MM
const int X_Lead_p = 2;                               // X lead screw pitch in mm/revolution
const int Y_Lead_p = 2;                               // Y lead screw pitch in mm
const int AOA_MAX = 40;                              // Angle of attack max in 360 degrees
float X_mm_to_micro = (1 / X_Lead_p) * (360 / Degree_per_step[0]) * (Micro_stepping[0]);
float Y_mm_to_micro = (1 / Y_Lead_p) * (360 / Degree_per_step[1]) * (Micro_stepping[1]);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Pin Define~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Define the LCD Pins ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/PE13, /* data=*/PE15, /* CS=*/PE14, /* reset=*/PE10); //
// Define the LCD Type and Pins Reset is currently pin 29 which us unused or unconnected on the board.
// ~~~~~~~~~~~~~~~~~~~~~~~~~ Trigger Pin Define  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const int TRIGGER_PIN = 24; // Temp0 pin on Melzi board
volatile bool Go_Pressed = false; // Default state of the trigger ISR variable 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~ Estop Pin define ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const int Estop_pin = 24;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MOTION CONTROL  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initalize Stepper Class
/*
In the case of this project:
Speedy Stepper is driving the steppers movment through the traditional Step,Dir interface of the driver.
This simplifies things greatly in code at the cost of speed. Which on the 180MHz this board runs on is not an issue.
The advanced features of the Stepper Driver are handled via Uart.
*/
SpeedyStepper X_stepper;    // motor 0
SpeedyStepper Y0_stepper;   // motor 1
SpeedyStepper Y1_stepper;  // motor 2_1 2_2
SpeedyStepper Y3_stepper;   // motor 3
SpeedyStepper AOAT_stepper; // motor 4
SpeedyStepper AOAB_stepper; // motor 5
SpeedyStepper Y2_stepper;
SpeedyStepper X2_stepper;
//  Stepper settings

uint8_t *Acceleration; // For The Acceleration setting in the LCD UI
uint8_t *Speed;        // For the LCD UI
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Serial Input Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const byte numChars = 64;          // Max Number of charcter read from serial in one input
char receivedChars[numChars] = {}; // Initialize a charcter array
char tempChars[numChars] = {};     // temporary char array used when parsing since "strtok" causes data loss

// variables to hold the parsed data
int Speed_Data[5] = {0, 0, 0, 0, 0}; // Hold the Speed Data
int Acell_Data[5] = {0, 0, 0, 0, 0}; // Hold the acelleration data
bool newData = false;                // Cotrol Entry into the Serial Reader
// ~~~~~~~~~~~~~~~~~~~ LCD Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~ Angle of Attack Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float AoA[2]; // floating point for AoA: Top,Bottom
// Passing in unsigned 8 bit intger ( Thats what the fucking UI command wants)
// the max of a 8 bit int is 255 and there are 360 derees ** this willl have to be changed to support up to .05 ) which will require
uint8_t AoA_t_value[4]; // Top AoA: Hundreds,tens/ones,Decimal
uint8_t AoA_b_value[4]; // Bottom AoA: Hundreds,tens/ones,Decimal
// ~~~~~~~~~~~~~~~~~~~~~~~~~~ X Movement Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float Xpos;         // X position float value
uint8_t X_value[3]; // X pos : Hundreds,tens/ones,Decimal
// ~~~~~~~~~~~~~~~~~~~~~~~~~~ Y Movemnt Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float Ypos;         // X position float value
uint8_t Y_value[3]; // Y pos: Hundreds,tens/ones,Decimal
// ~~~~~~~~~~~~~~~~~~~~~~~~~ Absolute Tracking Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float CurrentPositions[5] = {0, 0, 0, 0, 0}; // X,Y,AOAT,AOAB -> " x y AOAT AOAB,EXTRA" // modified for new motherboard
float movevar[5] = {0, 0, 0, 0, 0};          // X,Y,AOAT,AOAB , E2 // modified for new motherboard this wont get used though since the extra stepper is going to mirror another axis
// ~~~~~~~~~~~~~~~~~~~~~~~~~ Homing function varibles ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
volatile bool xhome = false;
volatile bool x2home = false;
volatile bool y1home = false;
volatile bool y2home = false;
volatile bool y3home = false;
volatile bool y4home = false;
volatile bool aoathome = false;
volatile bool aoabhome = false;
const int Motor0LimitSw = PG6; 
const int Motor1LimitSw =PG12;
const int Motor2LimitSw = PG9; 
const int Motor3LimitSw =PG13; 
const int Motor4LimitSw = PG10; 
const int Motor5LimitSw =PG14; 
const int Motor6LimitSw = PG11; 
const int Motor7LimitSw =PG15;  
// Reset Pin -> off of the RGB HEADDER J37
// const int Reset=PB0;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Menu Stuff~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool Abs_pos_error = false;

const char *Main_menu = // Define the Main Menu options
    "X Movement\n"
    "Y Movement\n"
    "A.O.A Top\n"
    "A.O.A Bottom\n"
    "Settings";

const char *Setting_list = // Define the Settings Sub menu options
    "Acceleration\n"
    "Speed\n"
    "Serial Com.\n" // "Trigger Mode\n"
    "Home All Axis\n"
    "BACK";

const char *Com_select = // Communcations method select menu
    "SERIAL\n"
    "LCD";
// const char *Motion_select = // motion select menu 
//     "Trigger ON\n"
//     "Trigger OFF";

const char *Error_String = // Error strings
    "Acknowledge\n"
    "Main Menu\n"
    "Software Restart";

uint8_t current_selection = 0; // Keep track of current selection in menu
uint8_t Sub_selection = 0;
uint8_t Com_selection = 2;    // communications selection tracker by default use the LCD
//uint8_t Motion_selection = 2; // Default to OFF



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`End Variables~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Acelleration/ Speed Set/ Trigger Functions  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SET_ACELL(float x, float y, float E0, float E1)
{
  X_stepper.setAccelerationInRevolutionsPerSecondPerSecond(x * 2);
  X2_stepper.setAccelerationInRevolutionsPerSecondPerSecond(x * 2);
  Y0_stepper.setAccelerationInRevolutionsPerSecondPerSecond(y * 2);
  Y1_stepper.setAccelerationInRevolutionsPerSecondPerSecond(y * 2); // By Default the Z axis will allways be attached to the Y (Verticle Axis)
  Y2_stepper.setAccelerationInRevolutionsPerSecondPerSecond(y * 2);
  Y3_stepper.setAccelerationInRevolutionsPerSecondPerSecond(y * 2);
  AOAT_stepper.setAccelerationInRevolutionsPerSecondPerSecond(E0); // Change to mm/s when the system is being implmented
  AOAB_stepper.setAccelerationInRevolutionsPerSecondPerSecond(E1); // Change to mm/s when the system is being implmented
  //  E2stepper.setAccelerationInStepsPerSecondPerSecond(*put axis its mirrioring here*);
  Acell_Data[0] = x;
  Acell_Data[1] = y;
  Acell_Data[2] = E0;
  Acell_Data[3] = E1;
}
void SET_SPEED(int x, int y, int E0, int E1)
{
  X_stepper.setSpeedInRevolutionsPerSecond(x * 2); // multpied by the leadscrew pitch because we are in rev/s and the value entered is in mm/s
  X2_stepper.setSpeedInRevolutionsPerSecond(x * 2);
  Y0_stepper.setSpeedInRevolutionsPerSecond(y * 2);
  Y1_stepper.setSpeedInRevolutionsPerSecond(y * 2);
  Y2_stepper.setSpeedInRevolutionsPerSecond(y * 2);
  Y3_stepper.setSpeedInRevolutionsPerSecond(y * 2); // Change to mm/s^2 when the system is being implmented
  AOAT_stepper.setSpeedInRevolutionsPerSecond(E0);  // Change to mm/s^2 when the system is being implmented ?
  AOAB_stepper.setSpeedInRevolutionsPerSecond(E1);
  //  E2stepper.setSpeedInStepsPerSecond(*Axis its mirrioring here*);
  Speed_Data[0] = x;
  Speed_Data[1] = y;
  Speed_Data[2] = E0;
  Speed_Data[3] = E1;
}

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SETUP lOOP~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup(void)
{
  // float X_to_micro=(1/X_Lead_p)*360*(1/Degree_per_step[0])*Micro_stepping[0];
  float X_to_micro = 6400;
  float Y_to_micro = 6400;
  Serial.begin(9600); // Begin serial ouput communication for debug and input of value array.
  // while (! Serial); // debug waiting for the computer to connect
  Serial.println(Y_to_micro);
  // set up the interrpts
  attachInterrupt(digitalPinToInterrupt(Motor0LimitSw), xHomeIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Motor1LimitSw), y1HomeIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Motor2LimitSw), y2HomeIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Motor3LimitSw), y3HomeIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Motor4LimitSw), y4HomeIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Motor5LimitSw), aoatHomeIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Motor6LimitSw), aoabHomeIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Motor7LimitSw), x2HomeIsr, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), motionTriggerIsr, FALLING); //External Trigger
  //attachInterrupt(digitalPinToInterrupt(Estop_pin), estopIsr, FALLING); //External Trigger  

  // delay(5000); // dely five seconds so the monitor can connect first --> VsCode monitor only
  DRIVER_SETUP();
  PIN_SETUP(); // Initilize all the Pins
  Serial.println("PUT LCD INTO DESIRED MODE AND SERIAL COMMUNCATION -->BEFORE<-- YOU INPUT --->ANYTHING<---!!!\n");
  Serial.println("");
  SET_ACELL(10, 10, 10, 10);   // Set motor acceleration
  SET_SPEED(10, 20, 20, 20); // Set motor Speed
  gui_output_function();       // initilize the GUI
                               /* Here we need to home all Axis and print over serial : % X0.00 Y0.00 T0.00 B0.00 % to initilize the GUI */

  u8g2.begin(/* menu_select_pin= */ PE7, /* menu_next_pin= */ PE12, /* menu_prev_pin= */ PE9, /* menu_home_pin= */ PC15); // pc 15 was selected at random to be an un used pin
  // Leave this outside the Pin Define and in the main dir. As it also serves as a class defintion.
  // Define the System Font see https://github.com/olikraus/u8g2/wiki/u8g2reference for more information about the commands
  u8g2.setFont(u8g2_font_6x12_tr);
  Draw_bitmap(); // DRAW THE BOOT SCREEN
  HomeAll();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VOID LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop(void)
{
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
  if (current_selection == 1)
  {
    // X movement
    // u8g2.userInterfaceInputValue( "X movment:", "", &X_value[0] , 0, 3 , 1 , " *-* Thousands of MM "); // Removed at the request of Dr. Y Functionality preserved
    u8g2.userInterfaceInputValue("X movment:", "", &X_value[1], 0, 5, 1, " *-* Hundreds of MM ");
    u8g2.userInterfaceInputValue("X movment:", "", &X_value[2], 0, 60, 2, " *-* Tens/Ones MM ");
    u8g2.userInterfaceInputValue("X movment:", "", &X_value[3], 0, 9, 1, " *-* Decimal MM ");
    Xpos = X_value[0] * 1000 + X_value[1] * 100 + X_value[2] + X_value[3] / 10; // add the two intgers toghter into a float because jesus its so much easier to work with the intger
    // move function call here
    MOVE_FUNCTION();
  }
  if (current_selection == 2)
  {
    // Y movemnt
    // u8g2.userInterfaceInputValue( "Y movment:", "", &Y_value[0] , 0, 3 , 1 , " *-* Thousands of MM "); // Removed at the request of Dr. Y Functionality preserved
    u8g2.userInterfaceInputValue("Y movment:", "", &Y_value[1], 0, 3, 1, " *-* Hundreds of MM ");
    u8g2.userInterfaceInputValue("Y movment:", "", &Y_value[2], 0, 60, 2, " *-* Tens/Ones MM ");
    u8g2.userInterfaceInputValue("Y movment:", "", &Y_value[3], 0, 9, 1, " *-* Decimal MM ");
    Ypos =Y_value[0] * 1000 + Y_value[1] * 100 + Y_value[2] + Y_value[3] / 10; // add the two intgers toghter into a float because jesus its so much easier to work with the intger
    /// move function call here
    MOVE_FUNCTION();
  }
  if (current_selection == 3)
  {
    u8g2.userInterfaceInputValue("AOA top:", "-", &AoA_t_value[0], 0, 20, 1, " 0-20 Negitive");
    u8g2.userInterfaceInputValue("AOA Top:", "", &AoA_t_value[1], 0, 9, 1, " 0-9 Negitive Decimal Degree");
    u8g2.userInterfaceInputValue("AOA Top:", "", &AoA_t_value[2], 0, 20, 3, " 0-20 Tens/Ones Degree"); // Error Message needs to be made if the input is made over the max AoA
    u8g2.userInterfaceInputValue("AOA Top:", "", &AoA_t_value[3], 0, 9, 1, " 0-9 Decimal Degree");
    //  headder,re string, pointer to unsigned char, min value, max vlaue, # of digits , post char
    AoA[0] = -1* AoA_t_value[0]+ -1*AoA_t_value[1]/10 + AoA_t_value[2]+AoA_t_value[3]/10; // This is the desierd angle we want in a floting point int.
    // Move function call here
    MOVE_FUNCTION();
    // 200 possible steps per revolution and 1/16 miro stepping meaning a pssiblity of 3,200 possible postions 360*/1.8 degrees/step

    // Serial.print(AoA);
    // Serial.print(H_value); // DEBUG Serial Print out
    // Serial.print(value);-
  }
  if (current_selection == 4)
  {
    u8g2.userInterfaceInputValue("AOA Bottom:", "-", &AoA_b_value[0], 0, 20, 1, " 0-20 Negitive");
    u8g2.userInterfaceInputValue("AOA Bottom:", "", &AoA_b_value[1], 0, 9, 1, " 0-9 Decimal Degree");
    u8g2.userInterfaceInputValue("AOA Bottom:", "", &AoA_b_value[2], 0, 20, 2, " -5-20 Tens/Ones Degree"); // Error Message needs to be made if the input is made over the max AoA
    u8g2.userInterfaceInputValue("AOA Bottom:", "", &AoA_b_value[3], 0, 9, 1, " 0-9 Decimal Degree");
    //  headder,re string, pointer to unsigned char, min value, max vlaue, # of digits , post char
    AoA[1] = -1* AoA_b_value[0]+ -1*AoA_b_value[1]/10 + AoA_b_value[2]+AoA_b_value[3]/10; // This is the desierd angle we want in a floting point int.
    // move function call here
    MOVE_FUNCTION();
    // 200 possible steps per revolution and 1/16 miro stepping meaning a pssiblity of 3,200 possible postions 360*/1.8 degrees/step
  }

  if (current_selection == 5)
  {
    Sub_selection = u8g2.userInterfaceSelectionList( // Bings up the Main Menu
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
    if (Sub_selection == 1)
    {
      // Acceleration Settings Code
      u8g2.userInterfaceInputValue("Acceleration:", "", Acceleration, 0, 20, 2, "*25 Rev/s^2");
      SET_ACELL(*Acceleration * 25, *Acceleration * 25, *Acceleration , *Acceleration);
    }
    if (Sub_selection == 2)
    {
      // Speed Settings
      u8g2.userInterfaceInputValue("Speed:", "", Speed, 0, 20, 2, "*25 Rev/s");
      SET_SPEED(*Speed * 25, *Speed * 25, *Speed * 5, *Speed * 5);
    }
    if (Sub_selection == 3)
    {
      // Motion Select function
      Com_selection = 1; // switch to serial Comunications
      SERIAL_UI();       // call the serial UI
    }
    // if (Sub_selection == 4)
    // {
    //   // Trigger mode
    //   Motion_selection = u8g2.userInterfaceSelectionList( // Bings up the trigger Menu
    //       "Trigger Select",
    //       Motion_selection,
    //       Motion_select);
    // }
    if (Sub_selection == 4)
    {
      // Home All Axis
      HomeAll();
    }
    if (Sub_selection == 5)
    {
      // Head back to Main meanu
      MAIN_MENU();
    }
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ END User Interface Code ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
} // ~~~~ End Main LOOP
// ~~~~~~~~~ Homing Interrupt code ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The point of this section of code is to interrupt the movement of the axis and stop them as they move towards home
/* Just some refrence code here
volatile bool xhome1=false;
volatile bool xhome2=false;
volatile bool yhome1=false;
volatile bool yhome2=false;
volatile bool aoathome=false;
volatile bool aoabhome=false;
In general all interrputs need to be kept as short as possible
*/

void xHomeIsr()
{
  xhome = !xhome; // set set them as hommed when the homing function is called
}
void x2HomeIsr()
{
  x2home = !xhome; // set set them as hommed when the homing function is called
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
 
  aoathome = !aoathome;
}
void aoabHomeIsr()
{
  aoabhome = !aoabhome;
}
void motionTriggerIsr(){
  Go_Pressed = true;
}
void estopIsr(){
  NVIC_SystemReset(); // use a software reset to kill the board 
}

/* For As long As the Octopus Board is used under no circustances should this ever be modified !!!*/
/*
This section of code determines how the system clock is cofigured this is important for the
STM32F446ZET6 in this case our board runs at 168 MHz not the 8Mhz external clock the board expects by default
No need to understand, attempt to or even try to.
Include it in every version that is compiled form this platfrom IO envrioment
*/

extern "C" void SystemClock_Config(void)
{
#ifdef OCTOPUS_BOARD
#ifdef OCTOPUS_BOARD_FROM_HSI
  /* boot from HSI, internal 16MHz RC, to 168MHz. **NO USB POSSIBLE**, needs HSE! */
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
#else
  /* boot from HSE, crystal oscillator (12MHz) */
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
#endif
#else
  /* nucleo board, 8MHz external clock input, HSE in bypass mode */
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
#endif
}