# 1 "C:\\Users\\ecslogon\\AppData\\Local\\Temp\\tmpljo4otxn"
#include <Arduino.h>
# 1 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/UI_Version_11.ino"
# 18 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/UI_Version_11.ino"
#include <Arduino.h>
#include <U8g2lib.h>
#include <SpeedyStepper.h>
#include <TMCStepper.h>
#include <TMCStepper_UTILITY.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
using namespace TMC2208_n;
#define DRIVER_ADDRESS 0b00




TMC2209Stepper driverX(PC4, PA6, .11f, DRIVER_ADDRESS );
TMC2209Stepper driverY(PD11, PA6, .11f, DRIVER_ADDRESS );
TMC2209Stepper driverZ(PC6, PA6, .11f, DRIVER_ADDRESS );
TMC2209Stepper driverE0(PC7, PA6, .11f, DRIVER_ADDRESS );
TMC2209Stepper driverE1(PF2, PA6, .11f, DRIVER_ADDRESS );



int Micro_stepping[5] = {64, 64, 64, 64, 64};
float Degree_per_step[5] = {1.8, 1.8, 1.8, 1.8, 1.8};
const int Xpos_MAX = 500;
const int Ypos_MAX = 500;
const int X_Lead_p=2;
const int Y_Lead_p=2;
const int AOA_MAX = 500;


U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, PE13, PE15, PE14, PE10);


int TRIGGER_PIN = 24;
# 64 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/UI_Version_11.ino"
SpeedyStepper Xstepper;
SpeedyStepper Ystepper;
SpeedyStepper Zstepper;
SpeedyStepper E0stepper;
SpeedyStepper E1stepper;


uint8_t* Acceleration;
uint8_t* Speed;


const byte numChars = 64;
char receivedChars[numChars]={};
char tempChars[numChars]={};


int Speed_Data[5]={0,0,0,0,0};
int Acell_Data[5]={0,0,0,0,0};
bool newData = false;


float AoA[2];


uint8_t AoA_t_value[3];
uint8_t AoA_b_value[3];

float Xpos;
uint8_t X_value[3];

float Ypos;
uint8_t Y_value[3];

float CurrentPositions[5] = {0, 0, 0, 0, 0};
float movevar[5]={0,0,0,0,0};

bool Abs_pos_error= false;

const char *Main_menu =
  "X Movement\n"
  "Y Movement\n"
  "A.O.A Top\n"
  "A.O.A Bottom\n"
  "T.Continious->Start\n"
  "Settings";

const char *Setting_list =
  "Acceleration\n"
  "Speed\n"
  "Movment Mode\n"
  "BACK";

const char *Com_select =
  "SERIAL\n"
  "LCD";
const char *Motion_select =
  "LCD Static\n"
  "LCD Trigger\n"
  "LCD External Trigger\n"
  "Serial Cont.\n"
  "Serial Cont. Ext.T.\n";

const char *Error_String =
  "Acknowledge\n"
  "Main Menu\n"
  "Hard Restart\n";

uint8_t current_selection = 0;
uint8_t Sub_selection = 0;
uint8_t Com_selection = 2;
uint8_t Motion_selection = 1;

int Go_Pressed = 0;
void SET_ACELL(float x, float y, float E0, float E1);
void SET_SPEED(int x, int y, int E0, int E1);
void TRIGGER_WAIT(int pin);
void setup(void);
void loop(void);
float ABS_POS(float input, int selection);
void DRIVER_SETUP();
bool Angle_Error(int a);
void Somthing_Error(void);
void Parsing_Error(void);
void MOVE_FUNCTION(void);
void PIN_SETUP();
void recvWithStartEndMarkers();
bool parseData();
void showParsedData();
void gui_output_function();
void serial_flush_buffer();
void MAIN_MENU();
void SERIAL_UI(void);
void Draw_button(U8G2 u8g2, uint8_t x, uint8_t y, uint8_t width, String str, bool clicked);
void Draw_dialog(U8G2 u8g2, uint8_t x, uint8_t y, uint8_t width, uint8_t height, String title, String msg1, String msg2, String msg3, String msg4, String btn, bool clicked);
#line 141 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/UI_Version_11.ino"
void SET_ACELL(float x, float y, float E0, float E1) {
  Xstepper.setAccelerationInRevolutionsPerSecondPerSecond(x);
  Ystepper.setAccelerationInRevolutionsPerSecondPerSecond(y);
  Zstepper.setAccelerationInRevolutionsPerSecondPerSecond(y);
  E0stepper.setAccelerationInRevolutionsPerSecondPerSecond(E0);
  E1stepper.setAccelerationInRevolutionsPerSecondPerSecond(E1);

}
void SET_SPEED(int x, int y, int E0, int E1) {
  Xstepper.setSpeedInRevolutionsPerSecond(x);
  Ystepper.setSpeedInRevolutionsPerSecond(y);
  Zstepper.setSpeedInRevolutionsPerSecond(y);
  E0stepper.setSpeedInRevolutionsPerSecond(E0);
  E1stepper.setSpeedInRevolutionsPerSecond(E1);

}
void TRIGGER_WAIT(int pin) {
  int Button_state = 0;
  while (1) {
    Button_state = digitalRead(pin);
    if (Button_state == HIGH) {
      return;
    }
  }
}


void setup(void) {
  const float X_to_micro=(1/X_Lead_p)*360*(1/Degree_per_step[0])*Micro_stepping[0];
  const float Y_to_micro=(1/X_Lead_p)*360*(1/Degree_per_step[1])*Micro_stepping[1];
  Serial.begin(9600);
  Serial.println(X_to_micro);


  DRIVER_SETUP();
  PIN_SETUP();
  Serial.println("PUT LCD INTO DESIRED MODE AND SERIAL COMMUNCATION -->BEFORE<-- YOU INPUT --->ANYTHING<---!!!\n");
  Serial.println("");
  SET_ACELL(400, 400, 400, 400);
  SET_SPEED(1000,1200, 1400, 1600);
  u8g2.begin( PE7, PE9, PE12, PC15);


  u8g2.setFont(u8g2_font_6x12_tr);
}



void loop(void) {

  MAIN_MENU();
# 200 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/UI_Version_11.ino"
  if ( current_selection == 1 ) {


    u8g2.userInterfaceInputValue( "X movment:", "", &X_value[1] , 0, 5 , 1 , " *-* Hundreds of MM ");
    u8g2.userInterfaceInputValue( "X movment:", "", &X_value[2] , 0, 60 , 2 , " *-* Tens/Ones MM ");
    u8g2.userInterfaceInputValue( "X movment:", "", &X_value[3] , 0, 9 , 1 , " *-* Decimal MM ");
    Xpos = X_value[0] * 1000 + X_value[1] * 100 + X_value[2] + X_value[3] / 10;

    MOVE_FUNCTION();
  }
  if ( current_selection == 2 ) {


    u8g2.userInterfaceInputValue( "Y movment:", "", &Y_value[1] , 0, 3 , 1 , " *-* Hundreds of MM ");
    u8g2.userInterfaceInputValue( "Y movment:", "", &Y_value[2] , 0, 60 , 2 , " *-* Tens/Ones MM ");
    u8g2.userInterfaceInputValue( "Y movment:", "", &Y_value[3] , 0, 9 , 1 , " *-* Decimal MM ");
    Ypos = Y_value[0] * 1000 + Y_value[1] * 100 + Y_value[2] + Y_value[3] / 10;

    MOVE_FUNCTION();

  }
  if ( current_selection == 3 ) {
    u8g2.userInterfaceInputValue( "AOA Top:", "", &AoA_t_value[0] , 0, 5 , 1 , " 0-3 Hundreds Degrees");
    u8g2.userInterfaceInputValue( "AOA Top:", "", &AoA_t_value[1] , 0, 99 , 2 , " 0-99 Tens/Ones Degree");
    u8g2.userInterfaceInputValue( "AOA Top:", "", &AoA_t_value[2] , 0, 9 , 1 , " 0-9 Decimal Degree");

    AoA[0] = AoA_t_value[0] * 100 + AoA_t_value[1] + AoA_t_value[2] / 10;

    MOVE_FUNCTION();





  }
  if ( current_selection == 4 ) {
    u8g2.userInterfaceInputValue( "AOA Bottom:", "", &AoA_b_value[0] , 0, 3 , 1 , " 0-3 Hundreds Degrees");
    u8g2.userInterfaceInputValue( "AOA Bottom:", "", &AoA_b_value[1] , 0, 99 , 2 , " 0-99 Tens/Ones Degree");
    u8g2.userInterfaceInputValue( "AOA Bottom:", "", &AoA_b_value[2] , 0, 9 , 1 , " 0-9 Decimal Degree");

    AoA[1] = AoA_b_value[0] * 100 + AoA_b_value[1] + AoA_b_value[2] / 10;

    MOVE_FUNCTION();

  }
  if ( current_selection == 5 ) {


    u8g2.userInterfaceMessage("", "", "Ready to Go?", " Ok \n Cancel ");
    if (Go_Pressed == 1 ) {

      movevar[0] = ABS_POS(Xpos, 1);
      movevar[1] = ABS_POS(Ypos, 2);
      movevar[2] = ABS_POS(AoA[0], 3);
      movevar[3] = ABS_POS(AoA[1], 4);
      if (Abs_pos_error==true){
      movevar[0] =0;
      movevar[1] =0;
      movevar[2] =0;
      movevar[3] =0;
      Go_Pressed = 0;
      MAIN_MENU();
      }
    Xstepper.setupRelativeMoveInSteps(movevar[0] / (Degree_per_step[0] / Micro_stepping[0]));
    Ystepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
    Zstepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
    E0stepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2]));
    E1stepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3]));
        while ((!E0stepper.motionComplete()) || (!E1stepper.motionComplete()) || (!Zstepper.motionComplete()) || (!Xstepper.motionComplete()) || (!Ystepper.motionComplete())) {
      Xstepper.processMovement();
      Ystepper.processMovement();
      Zstepper.processMovement();
      E0stepper.processMovement();
      E1stepper.processMovement();
      }

      Serial.println("Go Pressed\n ");
      Serial.print(current_selection);
      Go_Pressed = 0;
    }
  }
  if ( current_selection == 6 ) {
    Sub_selection = u8g2.userInterfaceSelectionList(
                      "Settings",
                      Sub_selection,
                      Setting_list);







    if ( Sub_selection == 1 ) {

      u8g2.userInterfaceInputValue( "Acceleration:", "", Acceleration , 0, 20 , 2 , "*25 Steps/s^2");
      SET_ACELL(*Acceleration * 25, *Acceleration * 25, *Acceleration * 25, *Acceleration * 25);
    }
    if ( Sub_selection == 2 ) {

      u8g2.userInterfaceInputValue( "Speed:", "", Speed , 0, 20 , 2 , "*25 Steps/s");
      SET_SPEED(*Speed * 25, *Speed * 25, *Speed * 25, *Speed * 25);
    }
    if ( Sub_selection == 3 ) {

      Motion_selection = u8g2.userInterfaceSelectionList("Motion Behavior", 1, Motion_select);
      if (Motion_selection == 1) {



        MAIN_MENU();
      }
      if (Motion_selection == 2) {



        MAIN_MENU();
      }
      if (Motion_selection == 3) {



        MAIN_MENU();
      }
      if (Motion_selection == 4) {



        Com_selection = 1;
        SERIAL_UI();
      }
      if (Motion_selection == 5) {



        Com_selection = 1;
        SERIAL_UI();
      }
    }
    if ( Sub_selection == 4 ) {

      MAIN_MENU();
    }
  }


}
# 356 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/UI_Version_11.ino"
extern "C" void SystemClock_Config(void)
{
#ifdef OCTOPUS_BOARD
#ifdef OCTOPUS_BOARD_FROM_HSI

  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};



  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);



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


  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
#else

  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};



  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);



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


  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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

  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};



  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);



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


  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
# 1 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/ABS_POS.ino"

float ABS_POS(float input, int selection)
{






  float calcpos;
  if (selection == 2 || selection == 3)
  {


    if (input > AOA_MAX)
    {
      Angle_Error(selection);
      return 0;
    }
    else
    {

      if (input == CurrentPositions[selection] || input == 360)
      {
        return 0;
      }

      if (input > CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection];
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }
      if (input < CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection];

        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }
    }
  }
  if (selection == 0)
  {

    if (input > Xpos_MAX)
    {
      Angle_Error(3);
      return 0;
    }
    else
    {

      if (input == CurrentPositions[selection])
      {
        return 0;
      }

      if (input > CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection];
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }

      if (input < CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection];
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }
    }
  }
  if (selection == 1)
  {

    if (input > Ypos_MAX)
    {
      Angle_Error(4);
      return 0;
    }
    else
    {

      if (input == CurrentPositions[selection])
      {
        return 0;
      }

      if (input > CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection];
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }

      if (input < CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection];
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }
    }
  }
  else
  {

    Somthing_Error();
    return 0;
  }
  return 0;
}
# 1 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/Driver_Setup.ino"

#include <TMCStepper.h>
#include <TMCStepper_UTILITY.h>
void DRIVER_SETUP(){
driverX.beginSerial(115200);
Serial.println("Driver X Enabled\n");
driverX.begin();
driverX.rms_current(850);
driverX.microsteps(64);
driverX.en_spreadCycle(0);
driverX.pwm_ofs_auto ();
driverX.pwm_autograd(1);
driverX.pwm_autoscale(1);

driverY.beginSerial(230400);
Serial.println("Driver Y Enabled\n");
driverY.begin();
driverY.rms_current(1100);
driverY.microsteps(64);
driverX.en_spreadCycle(0);
driverY.pwm_ofs_auto ();
driverY.pwm_autoscale(1);
driverY.pwm_autograd(1);

driverZ.beginSerial(115200);
Serial.println("Driver Z Enabled\n");
driverZ.begin();
driverZ.rms_current(850);
driverZ.microsteps(64);
driverZ.pwm_ofs_auto ();

driverE0.beginSerial(115200);
Serial.println("Driver X Enabled\n");
driverE0.begin();
driverE0.rms_current(850);
driverE0.microsteps(64);
driverE0.pwm_ofs_auto ();

driverE1.beginSerial(115200);
Serial.println("Driver X Enabled\n");
driverE1.begin();
driverE1.rms_current(850);
driverE1.microsteps(64);
driverE1.pwm_ofs_auto ();
}
# 1 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/Error.ino"
#include <Arduino.h>


bool Angle_Error(int a) {
  uint8_t Error_selection;
  const char *Axis[4] = {"AoA Top\n", "AoA Bottom\n", "X Axis\n", "Y Axis\n"};
  Error_selection = u8g2.userInterfaceMessage("Error:", Axis[a-1], "input is over the max allowed\n", Error_String);





  if ( Error_selection == 1) {

    Abs_pos_error= true;
    return 0;
  }
  if ( Error_selection == 2) {

    Abs_pos_error= true;
    MAIN_MENU();
    return 0;
  }
}
void Somthing_Error(void) {
  uint8_t Error_selection;
  Error_selection = u8g2.userInterfaceMessage("Error:", " Somthing Went Wrong!:\n", "\n", Error_String);





  if ( Error_selection == 1) {

    return;

  }
  if ( Error_selection == 2) {

    MAIN_MENU();
  }
  if ( Error_selection ==3){


  }
}
void Parsing_Error(void){
  uint8_t Error_selection;
  Error_selection = u8g2.userInterfaceMessage("Somthing Went Wrong:", "Code not Reconized:\n", "\n","ok");





  if ( Error_selection == 1) {

    return;
  }
}
# 1 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/Move_Functions.ino"







void MOVE_FUNCTION(void)
{
movevar[0]=0;
movevar[1]=0;
movevar[2]=0;
movevar[3]=0;
  Serial.println("I got to \"MOVE_FUNCTION()\".");
  if (Motion_selection != 2)
  {
    Serial.println("Motion_selection != 2");

      movevar[0] = ABS_POS(Xpos, 0);
      movevar[1] = ABS_POS(Ypos, 1);
      movevar[2] = ABS_POS(AoA[0], 2);
      movevar[3] = ABS_POS(AoA[1], 3);
      gui_output_function();
  }

  if (Motion_selection == 1 || Motion_selection == 4)
  {
    Serial.println("Motion_selection == 1 or 4");

    Xstepper.setupRelativeMoveInSteps(movevar[0] / (Degree_per_step[2] / Micro_stepping[2]));
    Ystepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[2] / Micro_stepping[2]));
    Zstepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[2] / Micro_stepping[2]));
    E0stepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2]));
    E1stepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3]));

    Serial.println("Entering while loop");
    while ((!E0stepper.motionComplete()) || (!E1stepper.motionComplete()) || (!Zstepper.motionComplete()) || (!Xstepper.motionComplete()) || (!Ystepper.motionComplete()))
    {
      Xstepper.processMovement();
      Ystepper.processMovement();
      Zstepper.processMovement();
      E0stepper.processMovement();
      E1stepper.processMovement();
    }
    Serial.println("Exited While Loop");
    if(Motion_selection == 1)
    {
      Abs_pos_error=false;
      MAIN_MENU();
    }
    else
    {
      Abs_pos_error=false;
      return;
    }

  }
  if (Motion_selection == 2)
  {




    Go_Pressed = 1;
    return;
  }

  if (Motion_selection == 3 || Motion_selection == 5)
  {

    Xstepper.setupRelativeMoveInSteps(movevar[0] / (Degree_per_step[0] / Micro_stepping[0]));
    Ystepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
    Zstepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
    E0stepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2]));
    E1stepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3]));



    digitalWrite(27, HIGH);
    TRIGGER_WAIT(TRIGGER_PIN);
    digitalWrite(27, LOW);
    while ((!E0stepper.motionComplete()) || (!E1stepper.motionComplete()) || (!Zstepper.motionComplete()) || (!Xstepper.motionComplete()) || (!Ystepper.motionComplete()))
    {
      Xstepper.processMovement();
      Ystepper.processMovement();
      Zstepper.processMovement();
      E0stepper.processMovement();
      E1stepper.processMovement();
    }
    Abs_pos_error=false;

  }
}
# 1 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/Pin_Setup.ino"
#include <Arduino.h>
#include <SpeedyStepper.h>



int BUTTON = PE7;
int BEEPER = PE8;
int ENCODER_RT = PE9;
int ENCODER_LT = PE12;
const int MOTOR0_STEP_PIN = PF13 ;
const int MOTOR0_DIRECTION_PIN = PF12;
const int MOTOR0_ENABLE = PF14;
const int MOTOR1_STEP_PIN = PG0;
const int MOTOR1_DIRECTION_PIN = PG1;
const int MOTOR1_ENABLE = PF15;
const int MOTOR2_STEP_PIN = PF11;
const int MOTOR2_DIRECTION_PIN = PG3;
const int MOTOR2_ENABLE = PG5;
const int MOTOR3_STEP_PIN = PG4;
const int MOTOR3_DIRECTION_PIN = PC1;
const int MOTOR3_ENABLE = PA0;
const int MOTOR4_STEP_PIN = PF9;
const int MOTOR4_DIRECTION_PIN = PF10;
const int MOTOR4_ENABLE= PG2;

const int Reset=PB0;



void PIN_SETUP(){





pinMode(MOTOR0_STEP_PIN, OUTPUT);
pinMode(MOTOR0_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR0_ENABLE , OUTPUT);

pinMode(MOTOR1_STEP_PIN, OUTPUT);
pinMode(MOTOR1_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR1_ENABLE , OUTPUT);

pinMode(MOTOR2_STEP_PIN, OUTPUT);
pinMode(MOTOR2_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR2_ENABLE , OUTPUT);

pinMode(MOTOR3_STEP_PIN, OUTPUT);
pinMode(MOTOR3_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR3_ENABLE , OUTPUT);

pinMode(MOTOR4_STEP_PIN, OUTPUT);
pinMode(MOTOR4_DIRECTION_PIN , OUTPUT);
pinMode(MOTOR4_ENABLE , OUTPUT);


pinMode(BEEPER, OUTPUT);
pinMode(BUTTON, INPUT);
pinMode(ENCODER_RT, INPUT);
pinMode(ENCODER_LT, INPUT);

digitalWrite(MOTOR0_ENABLE , LOW);
digitalWrite(MOTOR1_ENABLE , LOW);
digitalWrite(MOTOR2_ENABLE , LOW);
digitalWrite(MOTOR3_ENABLE , LOW);
digitalWrite(MOTOR0_ENABLE , LOW);
# 75 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/Pin_Setup.ino"
Xstepper.connectToPins(MOTOR0_STEP_PIN, MOTOR0_DIRECTION_PIN);
Ystepper.connectToPins(MOTOR1_STEP_PIN, MOTOR1_DIRECTION_PIN);
Zstepper.connectToPins(MOTOR2_STEP_PIN, MOTOR2_DIRECTION_PIN);
E0stepper.connectToPins(MOTOR3_STEP_PIN, MOTOR3_DIRECTION_PIN);
E1stepper.connectToPins(MOTOR4_STEP_PIN, MOTOR4_DIRECTION_PIN);
}
# 1 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/Serial_Read.ino"
#include <Arduino.h>


void recvWithStartEndMarkers()
{
  static bool printedMsg = 0;
  if (printedMsg == 0)
  {
    Serial.println("Got to revrecvWithStartEndMarkers()\n");
    printedMsg = 1;
  }

  static bool recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;
  while (Serial.available() > 0 && newData == false)
  {

    rc = Serial.read();
    if (recvInProgress == true)
    {
      if (rc != endMarker)
      {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars)
        {
          ndx = numChars - 1;
        }
      }
      else
      {
        receivedChars[ndx] = '\0';
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }
    else if (rc == startMarker)
    {
      recvInProgress = true;
    }
  }
}
# 66 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/Serial_Read.ino"
bool parseData()
{
  Serial.println("Got to parse data\n");
  char *strtokIndx;

  float Temp_Pos[5] = {Xpos, Ypos, AoA[0], AoA[1], NULL};
  int Setting_Num;
  int Temp_Settings[5];


  strtokIndx = strtok(tempChars, " ");
  Serial.println(strtokIndx[0]);
  if (strtokIndx[0] == 'R' || strtokIndx[0] == 'r')
  {

    digitalWrite(Reset,LOW);
  }
  if (strtokIndx[0] == 'G' || strtokIndx[0] == 'g')
  {
    Serial.println("IT Starts With A G\n");

    strtokIndx = strtok(NULL, " ");
    Serial.println(strtokIndx[0]);
    if (strtokIndx[0] == 'H' || strtokIndx[0] == 'h')
    {
      Serial.println("IT has an h\n");
      strtokIndx = strtok(NULL, " ");
      Serial.println(strtokIndx[0]);
      if (strtokIndx == NULL)
      {

      }
      else
      {
        while (strtokIndx != NULL)
        {
          if (strtokIndx[0] == 'X' || strtokIndx[0] == 'x')
          {
            Xstepper.moveToHomeInRevolutions(-1,20,20,PG6);
          }
          if (strtokIndx[0] == 'Y' || strtokIndx[0] == 'y')
          {

            Ystepper.moveToHomeInRevolutions(-1,20,20,PG9);
            Zstepper.moveToHomeInRevolutions(-1,20,20,PG13);
          }
          if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a')
          {
            if (strtokIndx[3] == 'T' || strtokIndx[3] == 't')
            {

              E0stepper.moveToHomeInRevolutions(-1,20,20,PG10);
            }
            if (strtokIndx[3] == 'B' || strtokIndx[3] == 'b')
            {

              E1stepper.moveToHomeInRevolutions(-1,20,20,PG11);
            }
          }
        }
      }
    }
    else if (isdigit(strtokIndx[0]))
    {
      Serial.print("G-code entered does not match the correct format please try again when prompted\n");
      strtokIndx = NULL;
      return false;
    }
    else
    {

      while (strtokIndx != NULL)
      {
        Serial.println(strtokIndx[0]);
        if (strtokIndx[0] == 'X' || strtokIndx[0] == 'x')
        {
          Serial.println("IT has an X\n");
          char *substr = strtokIndx + 1;
          Temp_Pos[0] = atof(substr);
        }
        if (strtokIndx[0] == 'Y' || strtokIndx[0] == 'y')
        {
          char *substr = strtokIndx + 1;
          Temp_Pos[1] = atof(substr);
        }
        if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a')
        {
          if (strtokIndx[3] == 'T' || strtokIndx[3] == 't')
          {
            char *substr = strtokIndx + 4;
            Temp_Pos[2] = atof(substr);
          }
          if (strtokIndx[3] == 'B' || strtokIndx[3] == 'b')
          {
            char *substr = strtokIndx + 4;
            Temp_Pos[3] = atof(substr);
          }
        }
        if (strtokIndx == NULL)
        {
          Serial.print("G-code entered does not match the correct format please try again when prompted\n");
          strtokIndx = NULL;
          return false;
        }

        strtokIndx = strtok(NULL, " ");
# 181 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/Serial_Read.ino"
      }
    }
    Xpos= Temp_Pos[0];
    Ypos = Temp_Pos[1];
    AoA[0]= Temp_Pos[2];
    AoA[1] = Temp_Pos[3];





    MOVE_FUNCTION();
    return true;
  }


  if (strtokIndx[0] == 'M' || strtokIndx[0] == 'm')
  {
    Serial.println(("strtokIndx[0] == M or m; strtokIndx[0] == " + std::string(strtokIndx)).c_str());
    strtokIndx = strtok(NULL, " ");
    Serial.println(("Got next strtokIndx; strtokIndx[0] == " + std::string(strtokIndx)).c_str());
    if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a')
    {
      Serial.println("strtokIndx[0] == A or a");
      Setting_Num = 0;
    }
    else if (strtokIndx[0] == 'S' || strtokIndx[0] == 's')
    {
      Serial.println("strtokIndx[0] == S or a");
      Setting_Num = 1;
    }
    else if (strtokIndx[0] == 'D' || strtokIndx[0] == 'D')
    {
      Serial.println("strtokIndx[0] == D or d");
      strtokIndx = strtok(NULL, " ");
      Serial.println("strtokIndx = strtok() called.");
      if (strtokIndx[0] == 'M' || strtokIndx[0] == 'm')
      {
        Serial.println("strtokIndx[0] == M or m");
        Setting_Num = 2;
      }
      if (strtokIndx[0] == 'P' || strtokIndx[0] == 'p')
      {
        Serial.println("strtokIndx[0] == P or p");
        Setting_Num = 3;
      }
      if (strtokIndx[0] == 'S' || strtokIndx[0] == 's')
      {
        Serial.println("strtokIndx[0] == S or s");
        Setting_Num = 4;
      }
    }
    else
    {
      Serial.print("M-code entered does not match the correct format please try again when prompted");
      return false;
    }
    strtokIndx = strtok(NULL, " ");
    Serial.println(("strtokIndx == " + std::string(strtokIndx)).c_str());
    if (isdigit(strtokIndx[0]))
    {
      Serial.println("Got to assigning numbers");
      Temp_Settings[0] = atof(strtokIndx);
      Temp_Settings[1] = atof(strtokIndx);
      Temp_Settings[2] = atof(strtokIndx);
      Temp_Settings[3] = atof(strtokIndx);
    }
    else
    {
      Serial.println("Not a digit; About to enter temp-settings assignment while loop.");
      while (strtokIndx != NULL)
      {
        Serial.println("Entered temp-settings assignment while loop.");
        if (strtokIndx[0] == 'X' || strtokIndx[0] == 'x')
        {
          Serial.println("Assign X");
          char *substr = strtokIndx + 1;
          Temp_Settings[0] = atof(substr);
        }
        if (strtokIndx[0] == 'Y' || strtokIndx[0] == 'y')
        {
          Serial.println("Assign Y.");
          char *substr = strtokIndx + 1;
          Temp_Settings[1] = atof(substr);
        }
        if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a')
        {
          Serial.println("strtokIndx[0] == A or a");
          if (strtokIndx[3] == 'T' || strtokIndx[3] == 't')
          {
            Serial.println("strtokIndx[0] == T or t");
            char *substr = strtokIndx + 4;
            Temp_Settings[2] = atof(substr);
          }
          if (strtokIndx[3] == 'B' || strtokIndx[3] == 'b')
          {
            Serial.println("strtokIndx[0] == B or b");
            char *substr = strtokIndx + 4;
            Temp_Settings[3] = atof(substr);
          }
        }
        else
        {
          Serial.print("M-code entered does not match the correct format please try again when prompted\n");
          strtokIndx = NULL;
          return false;
        }
        strtokIndx = strtok(NULL, " ");
      }
    }
    if (Setting_Num == 0)
    {
      Serial.println("Set acceleration");
      Acell_Data[0] = Temp_Settings[0];
      Acell_Data[1] = Temp_Settings[1];
      Acell_Data[2] = Temp_Settings[2];
      Acell_Data[3] = Temp_Settings[3];
      SET_ACELL(Acell_Data[0], Acell_Data[1], Acell_Data[2], Acell_Data[3]);
      return true;
    }
    if (Setting_Num == 1)
    {
      Serial.println("Set speed");
      Speed_Data[0] = Temp_Settings[0];
      Speed_Data[1] = Temp_Settings[1];
      Speed_Data[2] = Temp_Settings[2];
      Speed_Data[3] = Temp_Settings[3];
      SET_SPEED(Speed_Data[0], Speed_Data[1], Speed_Data[2], Speed_Data[3]);
      return true;
    }
    if (Setting_Num == 2)
    {
      Serial.println("Setting_Num == 2; Microstepping");
      if (Temp_Settings[0] != 0 || Temp_Settings[0] != 16 || Temp_Settings[0] != 64 || Temp_Settings[0] != 256 || Temp_Settings[1] != 0 || Temp_Settings[1] != 16 || Temp_Settings[1] != 64 || Temp_Settings[1] != 256 ||
          Temp_Settings[2] != 0 || Temp_Settings[2] != 16 || Temp_Settings[2] != 64 || Temp_Settings[2] != 256 || Temp_Settings[3] != 0 || Temp_Settings[3] != 16 || Temp_Settings[3] != 64 || Temp_Settings[3] != 256)
      {
        Serial.println("Microstepping not an acceptable input");
        return false;
      }
      else
      {
        Serial.println("Setting microstepping");
        Micro_stepping[0] = Temp_Settings[0];
        Micro_stepping[1] = Temp_Settings[1];
        Micro_stepping[2] = Micro_stepping[1];
        Micro_stepping[3] = Temp_Settings[2];
        Micro_stepping[4] = Temp_Settings[3];
        driverX.microsteps(Micro_stepping[0]);
        driverY.microsteps(Micro_stepping[1]);
        driverZ.microsteps(Micro_stepping[2]);
        driverE0.microsteps(Micro_stepping[3]);
        driverE1.microsteps(Micro_stepping[4]);
        return true;
      }
    }
    if (Setting_Num == 3)
    {
      Serial.println("Setting_Num == 3");
      if (Temp_Settings[0] != 0 || Temp_Settings[0] != 1)
      {
        return false;
      }
      else
      {
        driverX.en_spreadCycle(Temp_Settings[0]);
        driverY.en_spreadCycle(Temp_Settings[1]);
        driverZ.en_spreadCycle(Temp_Settings[2]);
        driverE0.en_spreadCycle(Temp_Settings[3]);
        driverE1.en_spreadCycle(Temp_Settings[4]);
        return true;
      }
    }
    if (Setting_Num == 4)
    {
      if (Temp_Settings[0] != 0 || Temp_Settings[0] != 1)
      {
        return false;
      }
      else
      {
        return true;
      }
    }
  }
  else
  {
    Serial.println("Shouldn't Have made it here\n");
    return false;
  }
}

void showParsedData()


{
  Serial.println("Parsed Data Debug output");
  Serial.print("X Pos");
  Serial.println(Xpos);
  Serial.print("Y Pos");
  Serial.println(Ypos);
  Serial.print("\nAoA Top");
  Serial.println(AoA[0]);
  Serial.print("AoA Bottom ");
  Serial.println(AoA[1]);






}
void gui_output_function()
{




 Serial.print("%");
 Serial.print("X");
 Serial.print(Xpos);
 Serial.print("Y");
 Serial.print(Ypos);
 Serial.print("T");
 Serial.print(AoA[0]);
 Serial.print("B");
 Serial.print(AoA[1]);
 Serial.print("%");
}
void serial_flush_buffer()
{
  while (Serial.read() >= 0)
    ;
  Serial.print("Serial Flushed");
}
# 1 "C:/Users/ecslogon/Documents/PlatformIO/Projects/WSU_AIRFOIL_PROJECT/Firmware_V11/src/UI_Functions.ino"

void MAIN_MENU()
{


  current_selection = u8g2.userInterfaceSelectionList(
      "Air Foil Control",
      current_selection,
      Main_menu);
}



void SERIAL_UI(void)
{

  serial_flush_buffer();
  Serial.println("This demo expects data frommated like:\n <G X###.## Y###.## AoAT###.## AoAB###.##>\n OR\n <M A X###.## Y###.## AoAT###.## AoAB###.##>\n OR\n <M S X###.## Y###.## AoAT###.## AoAB###.##>");
  Serial.println("FAILURE TO FOLLOW THE FORMATTING CAN CAUSE UNEXPECTED SYSTEM MOVEMENT\n IVE DONE MY BEST TO PREVENT THIS\n");
  Serial.println();
  while (Com_selection == 1)
  {

    recvWithStartEndMarkers();
    if (newData == true)
    {
      strcpy(tempChars, receivedChars);


      parseData();
      showParsedData();
      newData = false;
    }


    String TempString0 = String(CurrentPositions[0]);
    TempString0 += " X Pos";
    String TempString1 = String(CurrentPositions[1]);
    TempString1 += " Y Pos";
    String TempString2 = String(CurrentPositions[2]);
    TempString2 += " AoA Top";
    String TempString3 = String(CurrentPositions[3]);
    TempString3 += " AoA Bottom:";
    u8g2.clearBuffer();
    Draw_dialog(u8g2, 0, 0, 128, 64, "Serial Mode\n", TempString0, TempString1, TempString2, TempString3, "Return", false);
    u8g2.sendBuffer();
    if (digitalRead(BUTTON) == LOW)
    {
      u8g2.clearBuffer();
      Draw_dialog(u8g2, 0, 0, 128, 64, "Serial Mode\n", TempString0, TempString1, TempString2, TempString3, "Return", true);
      delay(400);
      u8g2.sendBuffer();
      Com_selection = 2;

      MAIN_MENU();
    }
  }
}




void Draw_button(U8G2 u8g2, uint8_t x, uint8_t y, uint8_t width, String str, bool clicked)
{
  if (clicked)
  {
    u8g2.setDrawColor(1);
    u8g2.drawRBox(x, y + 1, width, u8g2.getMaxCharHeight() + 4, 2);
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(x + (width / 2) - ((String(str).length() * (u8g2.getMaxCharWidth())) / 2), y + u8g2.getMaxCharHeight() + 3, str.c_str());
  }
  else
  {
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(x, y, width, u8g2.getMaxCharHeight() + 6, 4);
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(x + (width / 2) - ((String(str).length() * (u8g2.getMaxCharWidth())) / 2), y + u8g2.getMaxCharHeight() + 2, str.c_str());
  }
}

void Draw_dialog(U8G2 u8g2, uint8_t x, uint8_t y, uint8_t width, uint8_t height, String title, String msg1, String msg2, String msg3, String msg4, String btn, bool clicked)
{
  u8g2.drawRFrame(x, y, width, height, 2);

  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(x + (width / 2) - ((String(title).length() * (u8g2.getMaxCharWidth())) / 2), y + u8g2.getMaxCharHeight(), title.c_str());
  u8g2.drawHLine(x, y + u8g2.getMaxCharHeight() + 1, width);

  u8g2.drawStr(x + 2, y + u8g2.getMaxCharHeight() * 2 + 1, msg1.c_str());
  u8g2.drawStr(x + 2, y + u8g2.getMaxCharHeight() * 3 + 1, msg2.c_str());
  u8g2.drawStr(x + 2, y + u8g2.getMaxCharHeight() * 4 + 1, msg3.c_str());
  u8g2.drawStr(x + 2, y + u8g2.getMaxCharHeight() * 5 + 1, msg4.c_str());

  Draw_button(u8g2, x + width / 4, y + height - u8g2.getMaxCharHeight() * 2, width / 2, btn, clicked);
}