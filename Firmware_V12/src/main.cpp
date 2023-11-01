
#include <Arduino.h> // Include Github links here
#include "U8g2lib.h"
#include "SpeedyStepper.h"
// #include "SerialTransfer.h"
#include "TMCStepper.h"
#include "TMCStepper_UTILITY.h"
#include "stm32yyxx_ll_gpio.h"
#include "stm32yyxx_ll_gpio.h"
// Include custom functions after this
#include "Pin_Setup.hpp"
#include "Settings.hpp"
#include "Data_structures.h"
#include "Movement.hpp"
#include "amt21_driver.hpp"
#include "luapatch.h"
#include "lua\lua.hpp"
#include "lua\lualib.h"
#include "lua\lauxlib.h"
#include <STM32FreeRTOS.h>

#ifdef U8X8_HAVE_HW_SPI
#include "SPI.h"
#endif
#ifdef U8X8_HAVE_HW_I2C
#include "Wire.h"
#endif
typedef void (*pFunction)(void); // bootloader jump function
// using namespace TMC2208_n;       // Allows the TMC2209 to use functions out of tmc2208 required
#define DRIVER_ADDRESS 0b00
#define BOOTLOADER_FLAG_VALUE 0xDEADBEEF
#define BOOTLOADER_FLAG_OFFSET 100
#define BOOTLOADER_ADDRESS 0x1FFF0000
#define DOCTOPUS_BOARD
#define DOCTOPUS_BOARD_FROM_HSE

extern char _estack;
uint32_t *bootloader_flag;
pFunction JumpToApplication;
uint32_t JumpAddress;

// HardwareSerial Serial2(PD9, PD8);  // Second serial instance for the wifi
HardwareSerial Serial3(PE14, PE8); // Third serial instance for the rs484 encoders. This can be treated as a simplex, the only time we write out is to initilize.

// u8g2 lcd
U8G2_ST7920_128X64_F_SW_SPI gU8G2(U8G2_R0, /* clock=*/PE13, /* data=*/PE15, /* CS=*/PE14, /* reset=*/PE10);

//// Setpper Driver Initilization
// TMC Stepper Class
TMC2209Stepper gDriverX(PC4, PA6, .11f, DRIVER_ADDRESS); // (RX, TX,RSENSE, Driver address) Software serial X axis
TMC2209Stepper gDriverX2(PE1, PA6, .11f, DRIVER_ADDRESS);
TMC2209Stepper gDriverY0(PD11, PA6, .11f, DRIVER_ADDRESS);
TMC2209Stepper gDriverY1(PC6, PA6, .11f, DRIVER_ADDRESS);
TMC2209Stepper gDriverY2(PD3, PA6, .11f, DRIVER_ADDRESS);
TMC2209Stepper gDriverY3(PC7, PA6, .11f, DRIVER_ADDRESS);
TMC2209Stepper gDriverAOAT(PF2, PA6, .11f, DRIVER_ADDRESS);
TMC2209Stepper gDriverAOAB(PE4, PA6, .11f, DRIVER_ADDRESS);

// Speedy Stepper Class     // Octopus board plug.
SpeedyStepper gX0Stepper;   // motor 0
SpeedyStepper gY0Stepper;   // motor 1
SpeedyStepper gY1Stepper;   // motor 2_1 2_2 is mirriored of this axis but doesnt work?
SpeedyStepper gY3Stepper;   // motor 3
SpeedyStepper gAoaTStepper; // motor 4
SpeedyStepper gAoaBStepper; // motor 5
SpeedyStepper gY2Stepper;   // motor 6
SpeedyStepper gX1Stepper;   // motor 7

// Packetized Serial Trasfer
// SerialTransfer esp32_Com;
// SerialTransfer usb_Com;

// encoder classes
Amt21Encoder gAoaTEncoder(Serial3, Amt21Encoder::i14BIT, Amt21Encoder::i54, RS485_READ_ENABLE, RS485_WRITE_ENABLE);
Amt21Encoder gAoaBEncoder(Serial3, Amt21Encoder::i14BIT, Amt21Encoder::i74, RS485_READ_ENABLE, RS485_WRITE_ENABLE);

// init lua
lua_State *gLUA_STATE = luaL_newstate();

void Lua_output(const char *s)
{
  Serial.print(s);
}

// void  vStringSendingTask(void *pvParameters){
//   char *pcStringToSend;
//   const size_t xMaxStringLength = 50;
//   BaseType_t xStringNumber = 0;
//   for (;;){
//     /* Obtain a buffer that is at least xMaxStringLength characters big. The implementation
//     of prvGetBuffer() is not shown – it might obtain the buffer from a pool of pre-allocated
//     buffers, or just allocate the buffer dynamically. */
//     pcStringToSend = (char *)prvGetBuffer(xMaxStringLength);
//     /* Write a string into the buffer. */
//     snprintf(pcStringToSend, xMaxStringLength, "String number %d\r\n", xStringNumber);
//     /* Increment the counter so the string is different on each iteration of this task. */
//     xStringNumber++;
//     /* Send the address of the buffer to the queue that was created in Listing 52. The
//     address of the buffer is stored in the pcStringToSend variable.*/
//     xQueueSend(xPointerQueue,   /* The handle of the queue. */
//                 &pcStringToSend, /* The address of the pointer that points to the buffer. */
//                 portMAX_DELAY);
//   }
// }
// void vStringReceivingTask( void *pvParameters ){
//   char *pcReceivedString;
//   for( ;; ){
//     /* Receive the address of a buffer. */
//     xQueueReceive( xPointerQueue, /* The handle of the queue. */
//     &pcReceivedString, /* Store the buffer’s address in pcReceivedString. */
//     portMAX_DELAY );
//     /* The buffer holds a string, print it out. */
//     vPrintString( pcReceivedString );
//     /* The buffer is not required any more - release it so it can be freed, or re-used. */
//     prvReleaseBuffer( pcReceivedString );
//   }
// }


// ~~~~~~~~~Serial Read Functions~~~~~~~~~~~~~~~~~~~
bool recvWithStartEndMarkers(String *charInputBuffer)
{
  const int numChars = 256;
  char receivedChars[numChars] = {};
  bool newData = false;
  static bool printedMsg = 0; // Debug
  if (printedMsg == 0)        // Debug
  {
    Serial.println("Got to revrecvWithStartEndMarkers()\n"); // Debug output only print once
    printedMsg = 1;                                          // Debug
  }
  // Serial.println("Got to revrecvWithStartEndMarkers()\n");
  static bool recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;
  while (Serial.available() > 0 && newData == false)
  {
    Serial.println("Not done?");
    // Serial.println("Got to while (Serial.available() > 0 && newData == false) in recvWithStartEndMarkers()"); // Debug output
    rc = Serial.read();         // Look at the next character
    if (recvInProgress == true) // if we are recording
    {
      if (rc != endMarker) // And we are not at the end marker
      {
        receivedChars[ndx] = rc; // Throw the current char into the array
        ndx++;                   // increment index forward.
        if (ndx >= numChars)     // If we exceed the max continue to read and just throw the data into the last postition
        {
          ndx = numChars - 1;
        }
      }
      else
      {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;    // stop recording
        ndx = 0;                   // set index back to zero (formaility not truly required)
        newData = true;            // Let the program know that there is data wating for the parser.
      }
    }
    else if (rc == startMarker) // If Rc is the start marker we are getting good data
    {
      recvInProgress = true; // Start recording
    }
  }
  Serial.println("done?");
  Serial.println(*charInputBuffer);
  Serial.println(receivedChars);
  Serial.println(*receivedChars);
  *charInputBuffer = String(receivedChars);
  Serial.println(*charInputBuffer);
  Serial.println(receivedChars);
  Serial.println(*receivedChars);
  return newData;
}

void setup()
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
    HAL_RCC_DeInit();             // Kill the clocks they are configured wrong. (HSI DEFAULTS NO PERIFIERIALS)
    SystemClock_Config();         // Reconfigure the system clock to HSE
    HAL_Init();                   // Reinit the HAL LAYER
    __HAL_RCC_GPIOC_CLK_ENABLE(); // Enable the clocks
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
  *bootloader_flag = 0; // So next boot won't be affecteed // Fall through the boot code section and set the boot flag to 0 if everything goes good.
  ////////////////////////////////////////// Begin normal setup operation /////////////////////////////////////////////////
  // RTOS Stuff
  portBASE_TYPE s1, s2;
  
  // put the initlization code here.
  pin_setup();
  driver_setup();
  gX0Stepper.connectToPins(MOTOR0_STEP_PIN, MOTOR0_DIRECTION_PIN);
  gY0Stepper.connectToPins(MOTOR1_STEP_PIN, MOTOR1_DIRECTION_PIN);
  gY1Stepper.connectToPins(MOTOR2_STEP_PIN, MOTOR2_DIRECTION_PIN);
  gY3Stepper.connectToPins(MOTOR3_STEP_PIN, MOTOR3_DIRECTION_PIN);
  gAoaTStepper.connectToPins(MOTOR4_STEP_PIN, MOTOR4_DIRECTION_PIN);
  gAoaBStepper.connectToPins(MOTOR5_STEP_PIN, MOTOR5_DIRECTION_PIN);
  gX1Stepper.connectToPins(MOTOR6_STEP_PIN, MOTOR6_DIRECTION_PIN);
  if (DevConstants::SWD_PROGRAMING_MODE == false) // Disable Motor 7 to enable SWD
  {
    gY2Stepper.connectToPins(MOTOR7_STEP_PIN, MOTOR7_DIRECTION_PIN);
  }
  /// setup Queues
  QueueHandle_t qLuaScriptQueue;
  qLuaScriptQueue = xQueueCreate(2, sizeof(String *)); // Max of 2 scripts.
  QueueHandle_t qOutputQueue;
  qOutputQueue = xQueueCreate(2, sizeof(String *));
  //QueueHandle_t qErrorQueue;
  //qErrorQueue = xQueueCreate(2, sizeof(String *));

  // Define OS TASKS Here
  // create sensor task at priority two
  s1 = xTaskCreate(Task1, NULL, configMINIMAL_STACK_SIZE, NULL, 2, &sens);

  // create SD write task at priority one
  s2 = xTaskCreate(Task2, NULL, configMINIMAL_STACK_SIZE + 200, NULL, 1, NULL);


  // initilise External Coms
  Serial.begin(115200); // usb C coms
  // usb_Com.begin(Serial2);  // hand off the serial instance to serial transfer
  // Serial.begin(9600);     // ESP32 COMS
  // esp32_Com.begin(Serial); // hand off the serial instance to serial transfer
  // Serial3.begin(9600);    // rs485 encoders start serial

  luaopen_base(gLUA_STATE);
  luaL_openlibs(gLUA_STATE);
  luaL_dostring(gLUA_STATE, "print(\"> Version:\",_VERSION)");
  // LUA Based Home all call here
}

void loop()
{
  // Dont use
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