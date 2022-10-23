
#include <Arduino.h> // Include Github links here
#include <U8g2lib.h>
#include <SpeedyStepper.h>
#include <TMCStepper.h>
#include <TMCStepper_UTILITY.h>
#include <stm32yyxx_ll_gpio.h>

#include "Pin_Setup.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
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
// Dev Settings
bool Endstop_Bypass_enable = true;
bool Verbose_mode = true;
// Jump to bootloader stuff:
extern int _estack;
uint32_t *bootloader_flag;
pFunction JumpToApplication;
uint32_t JumpAddress;



//n pre delcared functions 
void setup();
void xHomeIsr();
void x2HomeIsr();
void y1HomeIsr();
void y2HomeIsr();
void y3HomeIsr();
void y4HomeIsr();
void aoatHomeIsr();
void aoabHomeIsr();
void motionTriggerIsr();
void estopIsr();


int main()
{
    // Run bootloader code
    // This is hella dangerous messing with the stack here but hey gotta learn somehow
  bootloader_flag = (uint32_t *)(&_estack - BOOTLOADER_FLAG_OFFSET); // below top of stack
  if (*bootloader_flag == BOOTLOADER_FLAG_VALUE)
  {

    *bootloader_flag = 1;
    /* Jump to system memory bootloader */
    HAL_SuspendTick();
    HAL_RCC_DeInit();
    HAL_DeInit();
    JumpAddress = *(__IO uint32_t *)(BOOTLOADER_ADDRESS + 4);
    JumpToApplication = (pFunction)JumpAddress;
    //__ASM volatile ("movs r3, #0\nldr r3, [r3, #0]\nMSR msp, r3\n" : : : "r3", "sp");
    JumpToApplication();
  }
  if (*bootloader_flag = 1)
  {
    //__memory_changed(void);
    HAL_RCC_DeInit();
    SystemClock_Config();
    HAL_Init();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
  *bootloader_flag = 0; // So next boot won't be affecteed
    ////// Begin normal run
    /// Global Varibles
    /// run setup 
    setup();
    // initilise External Coms
    // handle usb connection setup
    // handle wifi connection setup
    // check connection status - if there is somthing connected or trying to connect. 
    // set connection status 
    // if there is somthing connected Auto change to that Com. 

    // jump into main application
    // infinite loop
    for ( ; ; ) { // run the main application 
    // statement(s)


    }
}

void setup(){
    // put the initlization code here.
    /// Setup Innterupts
    attachInterrupt(digitalPinToInterrupt(Motor0LimitSw), xHomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor1LimitSw), y1HomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor2LimitSw), y2HomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor3LimitSw), y3HomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor4LimitSw), y4HomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor5LimitSw), aoatHomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor6LimitSw), aoabHomeIsr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Motor7LimitSw), x2HomeIsr, CHANGE);
    //// Setpper Driver Initilization
    // TMC Stepper Class
    TMC2209Stepper driverX(PC4, PA6, .11f, DRIVER_ADDRESS);    // (RX, TX,RSENSE, Driver address) Software serial X axis
    TMC2209Stepper driverX2(PE1, PA6, .11f, DRIVER_ADDRESS);   // (RX, TX,RSENSE, Driver address) Software serial X axis
    TMC2209Stepper driverY0(PD11, PA6, .11f, DRIVER_ADDRESS);  // (RX, TX,RSENSE, Driver address) Software serial X axis
    TMC2209Stepper driverY1(PC6, PA6, .11f, DRIVER_ADDRESS);   // (RX, TX,RSENSE, Driver address) Software serial X axis
    TMC2209Stepper driverY2(PD3, PA6, .11f, DRIVER_ADDRESS);   // (RX, TX,RSENSE, Driver address) Software serial X axis
    TMC2209Stepper driverY3(PC7, PA6, .11f, DRIVER_ADDRESS);   // (RX, TX,RSENSE, Driver Address) Software serial X axis
    TMC2209Stepper driverAOAT(PF2, PA6, .11f, DRIVER_ADDRESS); // (RX, TX,RESENSE, Driver address) Software serial X axis
    TMC2209Stepper driverAOAB(PE4, PA6, .11f, DRIVER_ADDRESS); // (RX, TX,RESENSE, Driver address) Software serial X axis
    // TMC2209Stepper driverE3(PE1, PA6, .11f, DRIVER_ADDRESS ); // (RX, TX,RESENSE, Driver address) Software serial X axis
    
    //Speedy Stepper            // Octopus board plug. 
    SpeedyStepper X_stepper;    // motor 0
    SpeedyStepper Y0_stepper;   // motor 1
    SpeedyStepper Y1_stepper;   // motor 2_1
    SpeedyStepper Y3_stepper;   // motor 3
    SpeedyStepper AOAT_stepper; // motor 4
    SpeedyStepper AOAB_stepper; // motor 5
    SpeedyStepper Y2_stepper;   // motor 6
    SpeedyStepper X2_stepper;   // motor 7

    X_stepper.connectToPins(MOTOR0_STEP_PIN, MOTOR0_DIRECTION_PIN);
    Y0_stepper.connectToPins(MOTOR1_STEP_PIN, MOTOR1_DIRECTION_PIN);
    Y1_stepper.connectToPins(MOTOR2_STEP_PIN, MOTOR2_DIRECTION_PIN);
    Y3_stepper.connectToPins(MOTOR3_STEP_PIN, MOTOR3_DIRECTION_PIN);
    AOAT_stepper.connectToPins(MOTOR4_STEP_PIN, MOTOR4_DIRECTION_PIN);
    AOAB_stepper.connectToPins(MOTOR5_STEP_PIN, MOTOR5_DIRECTION_PIN);
    X2_stepper.connectToPins(MOTOR6_STEP_PIN, MOTOR6_DIRECTION_PIN);
//Y2_stepper.connectToPins(MOTOR7_STEP_PIN, MOTOR7_DIRECTION_PIN);

}



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
void y4HomeIsr()
{
  y4home = !y4home;
}
void aoatHomeIsr()
{
  aoathome = !aoathome;
}
void aoabHomeIsr()
{
  aoabhome = !aoabhome;
}
void motionTriggerIsr()
{
  Go_Pressed = true;
}
void estopIsr()
{
  NVIC_SystemReset(); // use a software reset to kill the board
}


/* For As long As the Octopus Board is used under no circustances should this ever be modified !!!*/
/*
This section of code determines how the system clock is cofigured this is important for the
STM32F446ZET6 in this case our board runs at 168 MHz not the 8Mhz external clock the board expects by default
No need to understand, attempt to or even try to.
Include it in every version that is compiled form this platfrom IO envrioment/stm32dunio envrioment 
*/

extern "C" void SystemClock_Config(void)
{
//#ifdef OCTOPUS_BOARD
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