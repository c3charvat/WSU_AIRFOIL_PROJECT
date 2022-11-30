
#include "Arduino.h" // Include Github links here
#include "U8g2lib.h"
#include "SpeedyStepper.h"
#include "SerialTransfer.h"
#include "TMCStepper.h"
#include "TMCStepper_UTILITY.h"
#include "stm32yyxx_ll_gpio.h"
// Include custom functions after this
#include "Pin_Setup.hpp"
#include "Settings.hpp"
#include "Data_structures.h"
#include "Movement.hpp"
#include "amt21_driver.hpp"

#ifdef U8X8_HAVE_HW_SPI
#include "SPI.h"
#endif
#ifdef U8X8_HAVE_HW_I2C
#include "Wire.h"
#endif
typedef void (*pFunction)(void); // bootloader jump function
using namespace TMC2208_n;       // Allows the TMC2209 to use functions out of tmc2208 required
#define DRIVER_ADDRESS 0b00
#define BOOTLOADER_FLAG_VALUE 0xDEADBEEF
#define BOOTLOADER_FLAG_OFFSET 100
#define BOOTLOADER_ADDRESS 0x1FFF0000
#define DOCTOPUS_BOARD
#define DOCTOPUS_BOARD_FROM_HSE
using namespace std;

extern int _estack;
uint32_t *bootloader_flag;
pFunction JumpToApplication;
uint32_t JumpAddress;
//
HardwareSerial Serial2(PD9, PD8);  // Second serial instance for the wifi
HardwareSerial Serial3(PE14, PE8); // Third serial instance for the rs484 encoders. This can be treated as a simplex, the only time we write out is to initilize.

// u8g2 lcd
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/PE13, /* data=*/PE15, /* CS=*/PE14, /* reset=*/PE10);

//// Setpper Driver Initilization
// TMC Stepper Class
TMC2209Stepper driver_X(PC4, PA6, .11f, DRIVER_ADDRESS);    // (RX, TX,RSENSE, Driver address) Software serial X axis
TMC2209Stepper driver_X2(PE1, PA6, .11f, DRIVER_ADDRESS);   
TMC2209Stepper driver_Y0(PD11, PA6, .11f, DRIVER_ADDRESS);  
TMC2209Stepper driver_Y1(PC6, PA6, .11f, DRIVER_ADDRESS);   
TMC2209Stepper driver_Y2(PD3, PA6, .11f, DRIVER_ADDRESS);   
TMC2209Stepper driver_Y3(PC7, PA6, .11f, DRIVER_ADDRESS);   
TMC2209Stepper driver_AOAT(PF2, PA6, .11f, DRIVER_ADDRESS); 
TMC2209Stepper driver_AOAB(PE4, PA6, .11f, DRIVER_ADDRESS); 

// Speedy Stepper Class     // Octopus board plug.
SpeedyStepper x0_Stepper;   // motor 0
SpeedyStepper y0_Stepper;   // motor 1
SpeedyStepper y1_Stepper;   // motor 2_1 2_2 os mirriored of this axis but doesnt work?
SpeedyStepper y3_Stepper;   // motor 3
SpeedyStepper aoat_Stepper; // motor 4
SpeedyStepper aoab_Stepper; // motor 5
SpeedyStepper y2_Stepper;   // motor 6
SpeedyStepper x1_Stepper;   // motor 7

// Packetized Serial Trasfer
SerialTransfer esp32_Com;
SerialTransfer usb_Com;

// encoder classes
Amt21Encoder aoat_Encoder( Serial3, Amt21Encoder::i14BIT, Amt21Encoder::i54, RS485_READ_ENABLE, RS485_WRITE_ENABLE);
Amt21Encoder aoab_Encoder( Serial3, Amt21Encoder::i14BIT, Amt21Encoder::i74, RS485_READ_ENABLE, RS485_WRITE_ENABLE);


void setup()
{
  // put the initlization code here.
  pin_setup();
  driver_setup();


  /// Setup Innterupts
  x0_Stepper.connectToPins(MOTOR0_STEP_PIN, MOTOR0_DIRECTION_PIN);
  y0_Stepper.connectToPins(MOTOR1_STEP_PIN, MOTOR1_DIRECTION_PIN);
  y1_Stepper.connectToPins(MOTOR2_STEP_PIN, MOTOR2_DIRECTION_PIN);
  y3_Stepper.connectToPins(MOTOR3_STEP_PIN, MOTOR3_DIRECTION_PIN);
  aoat_Stepper.connectToPins(MOTOR4_STEP_PIN, MOTOR4_DIRECTION_PIN);
  aoab_Stepper.connectToPins(MOTOR5_STEP_PIN, MOTOR5_DIRECTION_PIN);
  x1_Stepper.connectToPins(MOTOR6_STEP_PIN, MOTOR6_DIRECTION_PIN);
  if (DevConstants::SWD_PROGRAMING_MODE == false)
  {
    y2_Stepper.connectToPins(MOTOR7_STEP_PIN, MOTOR7_DIRECTION_PIN);
  }
}

int main()
{
  ////////////////////////////////////////// Begin bootloder operation ////////////////////////////////////////////////////////////////////////
  // Do not edit above this line in main it will break the bootloader code
  // Run bootloader code
  // This is hella dangerous messing with the stack here but hey gotta learn somehow
  bootloader_flag = (uint32_t *)(&_estack - BOOTLOADER_FLAG_OFFSET); // below top of stack ******* The bootloader offset will have to be checked after the first flash to make sure it doesnt get overwritten******
  if (*bootloader_flag == BOOTLOADER_FLAG_VALUE)
  {

    *bootloader_flag = 1;
    /* Jump to system memory bootloader */
    HAL_SuspendTick(); // Kill whats running (in a sense)
    HAL_RCC_DeInit();  // Kill the the clocks
    HAL_DeInit();      // Kill the  HAL layer all together
    JumpAddress = *(__IO uint32_t *)(BOOTLOADER_ADDRESS + 4);
    JumpToApplication = (pFunction)JumpAddress;
    //__ASM volatile ("movs r3, #0\nldr r3, [r3, #0]\nMSR msp, r3\n" : : : "r3", "sp");
    JumpToApplication();
  }
  if (*bootloader_flag = 1)
  {
    HAL_RCC_DeInit();                 // Kill the clocks they are configured wrong. (HSI DEFAULTS NO PERIFIERIALS)
    SystemClock_Config();             // Reconfigure the system clock to HSE
    HAL_Init();                       // Reinit the HAL LAYER
    __HAL_RCC_GPIOC_CLK_ENABLE();     // Enable the clocks
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
  *bootloader_flag = 0;                // So next boot won't be affecteed // Fall through the boot code section and set the boot flag to 0 if everything goes good.

  ////////////////////////////////////////// Begin normal operation /////////////////////////////////////////////////

  // create the data strucures
  ConnectStatusStruct *Connectionstatus_ptr, Connectionstatus; // initalise a pointer to a strcut of connect test and
  Connectionstatus_ptr = &Connectionstatus;
  ControlStruct *Source_ptr, Source;
  Source_ptr = &Source;
  PositionStruct *RecievedData_ptr, RecievedData;
  RecievedData_ptr = &RecievedData;
  PositionStruct *CurrentPostions_ptr, CurrentPostions;
  CurrentPostions_ptr = &CurrentPostions;

  // Error Struct

  Error *Error_ptr, Error_struct;
  Error_ptr = &Error_struct;

  // initilize the structures
  initialize_movement_struct(CurrentPostions_ptr, Source_ptr);
  initialize_movement_struct(RecievedData_ptr, Source_ptr);
  /// run setup
  setup();
  // initilise External Coms
  Serial2.begin(9600);    // usb C coms
  usbCOM.begin(Serial2);  // hand off the serial instance to serial transfer
  Serial.begin(9600);     // ESP32 COMS
  esp32COM.begin(Serial); // hand off the serial instance to serial transfer
  Serial3.begin(9600);    // rs485 encoders start serial

 
  u8g2.begin(/* menu_select_pin= */ PE7, /* menu_next_pin= */ PE12, /* menu_prev_pin= */ PE9, /* menu_home_pin= */ PC15); // pc 15 was selected at random to be an un used pin
  // Leave this outside the Pin Define and in the main dir. As it also serves as a class defintion.
  // Define the System Font see https://github.com/olikraus/u8g2/wiki/u8g2reference for more information about the commands
  u8g2.setFont(u8g2_font_6x12_tr);

  home_all(CurrentPostions_ptr, Error_ptr, Settings::AOA_T_NODE_ADDR, Settings::AOA_B_NODE_ADDR); // home all call
  //////Main Applications
  // infinite loop
  for (;;)
  { // run the main
    if (esp32COM.available() || usbCOM.available())
    {
      // parse the packet comming in
      // use this variable to keep track of how many
      // bytes we've processed from the receive buffer
      uint16_t recSize = 0;

      // recSize = esp32COM.rxObj(testStruct, recSize);
    }
    // statement(s)
  }
}

/* For As long As the Octopus Board is used under no circustances should this ever be modified !!!*/
/*
This section of code determines how the system clock is cofigured this is important for the
STM32F446ZET6 in this case our board runs at 168 MHz not the 8Mhz external clock the board expects by default
No need to understand, attempt to or even try to.
Include it in every version that is compiled form this platfrom IO envrioment/stm32dunio envrioment
Underclocked to 168mhz for stability?
*/

extern "C" void SystemClock_Config(void)
{
// #ifdef OCTOPUS_BOARD
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
  // #else
  //   /* nucleo board, 8MHz external clock input, HSE in bypass mode */
  //   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  //   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  //   RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  //   /** Configure the main internal regulator output voltage
  //    */
  //   __HAL_RCC_PWR_CLK_ENABLE();
  //   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  //   /** Initializes the RCC Oscillators according to the specified parameters
  //    * in the RCC_OscInitTypeDef structure.
  //    */
  //   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  //   RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  //   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  //   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  //   RCC_OscInitStruct.PLL.PLLM = 4;
  //   RCC_OscInitStruct.PLL.PLLN = 168;
  //   RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  //   RCC_OscInitStruct.PLL.PLLQ = 7;
  //   RCC_OscInitStruct.PLL.PLLR = 2;
  //   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  //   {
  //     Error_Handler();
  //   }
  //   /** Initializes the CPU, AHB and APB buses clocks
  //    */
  //   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  //   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  //   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  //   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  //   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  //   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  //   {
  //     Error_Handler();
  //   }
  //   PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  //   PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
  //   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  //   {
  //     Error_Handler();
  //   }
  // #endif
}