#include "stm32f4xx_hal.h"
#include "HalGpio.hpp"
#include "HalSerial.hpp"
#include <SpeedyStepper.h>
#include <Settings.hpp>
#include <Pin_Setup.hpp>

// External Serial object from main.cpp
extern HalSerial Serial;

/*
This file sets up the Pin Modes
It its important to note that Stepper driver 7 shares pins with the swd interface...
So if you are not programming with dfu mode nothing will actuall program and wverthing ill result in errors
The SwD interface will not work untill a full power off and the SwD pins have been freed from any use in code.
 https://www.st.com/resource/en/application_note/cd00167594-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf
 Page 54 for more information
*/

// ============================================================================
// Pin Definitions - Using GPIO port and pin directly
// ============================================================================

// LCD SETUP
#define BUTTON_PORT       GPIOE
#define BUTTON_PIN        GPIO_PIN_7
#define BEEPER_PORT       GPIOE
#define BEEPER_PIN        GPIO_PIN_8
#define ENCODER_RT_PORT   GPIOE
#define ENCODER_RT_PIN    GPIO_PIN_9
#define ENCODER_LT_PORT   GPIOE
#define ENCODER_LT_PIN    GPIO_PIN_12

// RS485
#define RS485_RE_PORT     GPIOE
#define RS485_RE_PIN      GPIO_PIN_9
#define RS485_WE_PORT     GPIOE
#define RS485_WE_PIN      GPIO_PIN_12

// Fans
#define FAN0_PORT         GPIOA
#define FAN0_PIN          GPIO_PIN_8
#define FAN1_PORT         GPIOE
#define FAN1_PIN          GPIO_PIN_5
#define FAN2_PORT         GPIOD
#define FAN2_PIN          GPIO_PIN_12

// Motor 0 (X axis)
#define M0_STEP_PORT      GPIOF
#define M0_STEP_PIN       GPIO_PIN_13
#define M0_DIR_PORT       GPIOF
#define M0_DIR_PIN        GPIO_PIN_12
#define M0_EN_PORT        GPIOF
#define M0_EN_PIN         GPIO_PIN_14

// Motor 1 (Y axis)
#define M1_STEP_PORT      GPIOG
#define M1_STEP_PIN       GPIO_PIN_0
#define M1_DIR_PORT       GPIOG
#define M1_DIR_PIN        GPIO_PIN_1
#define M1_EN_PORT        GPIOF
#define M1_EN_PIN         GPIO_PIN_15

// Motor 2 (Z/Y1)
#define M2_STEP_PORT      GPIOF
#define M2_STEP_PIN       GPIO_PIN_11
#define M2_DIR_PORT       GPIOG
#define M2_DIR_PIN        GPIO_PIN_3
#define M2_EN_PORT        GPIOG
#define M2_EN_PIN         GPIO_PIN_5

// Motor 3 (E0/AoAt)
#define M3_STEP_PORT      GPIOG
#define M3_STEP_PIN       GPIO_PIN_4
#define M3_DIR_PORT       GPIOC
#define M3_DIR_PIN        GPIO_PIN_1
#define M3_EN_PORT        GPIOA
#define M3_EN_PIN         GPIO_PIN_0

// Motor 4 (E1/AoAB)
#define M4_STEP_PORT      GPIOF
#define M4_STEP_PIN       GPIO_PIN_9
#define M4_DIR_PORT       GPIOF
#define M4_DIR_PIN        GPIO_PIN_10
#define M4_EN_PORT        GPIOG
#define M4_EN_PIN         GPIO_PIN_2

// Motor 5
#define M5_STEP_PORT      GPIOC
#define M5_STEP_PIN       GPIO_PIN_13
#define M5_DIR_PORT       GPIOF
#define M5_DIR_PIN        GPIO_PIN_0
#define M5_EN_PORT        GPIOF
#define M5_EN_PIN         GPIO_PIN_1

// Motor 6
#define M6_STEP_PORT      GPIOE
#define M6_STEP_PIN       GPIO_PIN_2
#define M6_DIR_PORT       GPIOE
#define M6_DIR_PIN        GPIO_PIN_3
#define M6_EN_PORT        GPIOD
#define M6_EN_PIN         GPIO_PIN_4

// Motor 7 (shares SWD pins)
#define M7_STEP_PORT      GPIOE
#define M7_STEP_PIN       GPIO_PIN_6
#define M7_DIR_PORT       GPIOA
#define M7_DIR_PIN        GPIO_PIN_14
#define M7_EN_PORT        GPIOE
#define M7_EN_PIN         GPIO_PIN_0

// Limit switches
#define LIM0_PORT         GPIOG
#define LIM0_PIN          GPIO_PIN_6
#define LIM1_PORT         GPIOG
#define LIM1_PIN          GPIO_PIN_12
#define LIM2_PORT         GPIOG
#define LIM2_PIN          GPIO_PIN_9
#define LIM3_PORT         GPIOG
#define LIM3_PIN          GPIO_PIN_13
#define LIM4_PORT         GPIOG
#define LIM4_PIN          GPIO_PIN_10
#define LIM5_PORT         GPIOG
#define LIM5_PIN          GPIO_PIN_14
#define LIM6_PORT         GPIOG
#define LIM6_PIN          GPIO_PIN_11
#define LIM7_PORT         GPIOG
#define LIM7_PIN          GPIO_PIN_15

// Legacy pin constants for compatibility with other code
const int BUTTON = 0;  // Placeholder - use HAL directly
const int BEEPER = 0;
const int ENCODER_RT = 0;
const int ENCODER_LT = 0;
const int RS485_READ_ENABLE = 0;
const int RS485_WRITE_ENABLE = 0;
const int FAN0 = 0;
const int FAN1 = 0;
const int FAN2 = 0;
const int MOTOR0_STEP_PIN = 0;
const int MOTOR0_DIRECTION_PIN = 0;
const int MOTOR0_ENABLE = 0;
const int MOTOR1_STEP_PIN = 0;
const int MOTOR1_DIRECTION_PIN = 0;
const int MOTOR1_ENABLE = 0;
const int MOTOR2_STEP_PIN = 0;
const int MOTOR2_DIRECTION_PIN = 0;
const int MOTOR2_ENABLE = 0;
const int MOTOR3_STEP_PIN = 0;
const int MOTOR3_DIRECTION_PIN = 0;
const int MOTOR3_ENABLE = 0;
const int MOTOR4_STEP_PIN = 0;
const int MOTOR4_DIRECTION_PIN = 0;
const int MOTOR4_ENABLE = 0;
const int MOTOR5_STEP_PIN = 0;
const int MOTOR5_DIRECTION_PIN = 0;
const int MOTOR5_ENABLE = 0;
const int MOTOR6_STEP_PIN = 0;
const int MOTOR6_DIRECTION_PIN = 0;
const int MOTOR6_ENABLE = 0;
const int MOTOR7_STEP_PIN = 0;
const int MOTOR7_DIRECTION_PIN = 0;
const int MOTOR7_ENABLE = 0;
const int Motor0LimitSw = 0;
const int Motor1LimitSw = 0;
const int Motor2LimitSw = 0;
const int Motor3LimitSw = 0;
const int Motor4LimitSw = 0;
const int Motor5LimitSw = 0;
const int Motor6LimitSw = 0;
const int Motor7LimitSw = 0;

// Interrupts Variable declarations
volatile bool x0home = false;
volatile bool x1home = false;
volatile bool y0home = false;
volatile bool y1home = false;
volatile bool y2home = false;
volatile bool y3home = false;
volatile bool aoathome = false;
volatile bool aoabhome = false;

// ============================================================================
// TMC2209 driver objects
// R_SENSE = 0.11 ohm (standard BTT Octopus sense resistors)
// All use slave address 0; NullStream until real UART pins are wired in.
// TODO: Replace gTmcStream with a HalUartStream pointing to the board's
//       TMC UART peripheral once the pin mapping is confirmed.
// ============================================================================
static NullStream gTmcStream;

TMC2209Stepper gDriverX (&gTmcStream, 0.11f, 0);
TMC2209Stepper gDriverX2(&gTmcStream, 0.11f, 0);
TMC2209Stepper gDriverY0(&gTmcStream, 0.11f, 0);
TMC2209Stepper gDriverY1(&gTmcStream, 0.11f, 0);
TMC2209Stepper gDriverY2(&gTmcStream, 0.11f, 0);
TMC2209Stepper gDriverY3(&gTmcStream, 0.11f, 0);
TMC2209Stepper gDriverAOAT(&gTmcStream, 0.11f, 0);
TMC2209Stepper gDriverAOAB(&gTmcStream, 0.11f, 0);

// ============================================================================
// Helper to configure output pin
// ============================================================================
static void configureOutputPin(GPIO_TypeDef* port, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

// ============================================================================
// Helper to configure input pin with EXTI interrupt
// ============================================================================
static void configureInputWithInterrupt(GPIO_TypeDef* port, uint16_t pin, IRQn_Type irqn) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    
    HAL_NVIC_SetPriority(irqn, 5, 0);
    HAL_NVIC_EnableIRQ(irqn);
}

void pin_setup()
{
    // RS485
    configureOutputPin(RS485_RE_PORT, RS485_RE_PIN);
    configureOutputPin(RS485_WE_PORT, RS485_WE_PIN);

    // Motor 0 (X)
    configureOutputPin(M0_STEP_PORT, M0_STEP_PIN);
    configureOutputPin(M0_DIR_PORT, M0_DIR_PIN);
    configureOutputPin(M0_EN_PORT, M0_EN_PIN);
    
    // Motor 1 (Y0)
    configureOutputPin(M1_STEP_PORT, M1_STEP_PIN);
    configureOutputPin(M1_DIR_PORT, M1_DIR_PIN);
    configureOutputPin(M1_EN_PORT, M1_EN_PIN);
    
    // Motor 2 (Y1)
    configureOutputPin(M2_STEP_PORT, M2_STEP_PIN);
    configureOutputPin(M2_DIR_PORT, M2_DIR_PIN);
    configureOutputPin(M2_EN_PORT, M2_EN_PIN);
    
    // Motor 3 (Y2)
    configureOutputPin(M3_STEP_PORT, M3_STEP_PIN);
    configureOutputPin(M3_DIR_PORT, M3_DIR_PIN);
    configureOutputPin(M3_EN_PORT, M3_EN_PIN);
    
    // Motor 4 (AoA Top)
    configureOutputPin(M4_STEP_PORT, M4_STEP_PIN);
    configureOutputPin(M4_DIR_PORT, M4_DIR_PIN);
    configureOutputPin(M4_EN_PORT, M4_EN_PIN);
    
    // Motor 5 (AoA Bottom)
    configureOutputPin(M5_STEP_PORT, M5_STEP_PIN);
    configureOutputPin(M5_DIR_PORT, M5_DIR_PIN);
    configureOutputPin(M5_EN_PORT, M5_EN_PIN);
    
    // Motor 6 (X2)
    configureOutputPin(M6_STEP_PORT, M6_STEP_PIN);
    configureOutputPin(M6_DIR_PORT, M6_DIR_PIN);
    configureOutputPin(M6_EN_PORT, M6_EN_PIN);
    
    // Motor 7 (Y4) - only if not in SWD mode
    if (DevConstants::SWD_PROGRAMING_MODE == false) {
        configureOutputPin(M7_STEP_PORT, M7_STEP_PIN);
        configureOutputPin(M7_DIR_PORT, M7_DIR_PIN);
        configureOutputPin(M7_EN_PORT, M7_EN_PIN);
    }

    // Beeper, Button, Encoder
    configureOutputPin(BEEPER_PORT, BEEPER_PIN);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = ENCODER_RT_PIN;
    HAL_GPIO_Init(ENCODER_RT_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = ENCODER_LT_PIN;
    HAL_GPIO_Init(ENCODER_LT_PORT, &GPIO_InitStruct);

    // Fans
    configureOutputPin(FAN0_PORT, FAN0_PIN);
    configureOutputPin(FAN1_PORT, FAN1_PIN);
    configureOutputPin(FAN2_PORT, FAN2_PIN);
    HAL_GPIO_WritePin(FAN0_PORT, FAN0_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FAN1_PORT, FAN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FAN2_PORT, FAN2_PIN, GPIO_PIN_SET);

    // Enable stepper drivers (active low)
    HAL_GPIO_WritePin(M0_EN_PORT, M0_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_EN_PORT, M1_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M2_EN_PORT, M2_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M3_EN_PORT, M3_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M4_EN_PORT, M4_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M5_EN_PORT, M5_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M6_EN_PORT, M6_EN_PIN, GPIO_PIN_RESET);
    if (DevConstants::SWD_PROGRAMING_MODE == false) {
        HAL_GPIO_WritePin(M7_EN_PORT, M7_EN_PIN, GPIO_PIN_RESET);
    }

    // Limit switch interrupts
    configureInputWithInterrupt(LIM0_PORT, LIM0_PIN, EXTI9_5_IRQn);   // PG6
    configureInputWithInterrupt(LIM1_PORT, LIM1_PIN, EXTI15_10_IRQn); // PG12
    configureInputWithInterrupt(LIM2_PORT, LIM2_PIN, EXTI9_5_IRQn);   // PG9
    configureInputWithInterrupt(LIM3_PORT, LIM3_PIN, EXTI15_10_IRQn); // PG13
    configureInputWithInterrupt(LIM4_PORT, LIM4_PIN, EXTI15_10_IRQn); // PG10
    configureInputWithInterrupt(LIM5_PORT, LIM5_PIN, EXTI15_10_IRQn); // PG14
    configureInputWithInterrupt(LIM6_PORT, LIM6_PIN, EXTI15_10_IRQn); // PG11
    configureInputWithInterrupt(LIM7_PORT, LIM7_PIN, EXTI15_10_IRQn); // PG15
}

void driver_setup()
{
  Serial.println("Driver X Enabled\n");
  gDriverX.begin();
  gDriverX.rms_current(1100); // mA
  gDriverX.microsteps(64);
  //gDriverX.en_spreadCycle(0); // Page 44 use stealth chop
  gDriverX.pwm_ofs_auto ();
  gDriverX.pwm_autograd(1);
  gDriverX.pwm_autoscale(1);
  gDriverX.toff(5);

  Serial.println("Driver X2 Enabled\n");
  gDriverX2.begin();
  gDriverX2.rms_current(1100); // mA
  gDriverX2.microsteps(64);
  gDriverX2.pwm_ofs_auto ();
  gDriverX2.pwm_autograd(1);
  gDriverX2.pwm_autoscale(1);
  gDriverX2.toff(5);

  Serial.println("Driver Y0 Enabled\n");
  gDriverY0.begin();
  gDriverY0.rms_current(900); // mA
  gDriverY0.microsteps(64);
  //gDriverY0.en_spreadCycle(0);
  gDriverY0.pwm_ofs_auto ();
  gDriverY0.pwm_autoscale(1);
  gDriverY0.pwm_autograd(1);
  gDriverY0.toff(5);

  Serial.println("Driver Y1 Enabled\n");
  gDriverY1.begin();
  gDriverY1.rms_current(900); // mA
  gDriverY1.microsteps(64);
  gDriverY1.pwm_ofs_auto ();
  gDriverY1.pwm_autograd(1);
  gDriverY1.pwm_autoscale(1);
  gDriverY1.toff(5);

  Serial.println("Driver Y2 Enabled\n");
  gDriverY2.begin();
  gDriverY2.rms_current(900); // mA
  gDriverY2.microsteps(64);
  gDriverY2.pwm_ofs_auto ();
  gDriverY2.pwm_autograd(1);
  gDriverY2.pwm_autoscale(1);
  gDriverY2.toff(5);

  Serial.println("Driver Y3 Enabled\n");
  gDriverY3.begin();
  gDriverY3.rms_current(850); // mA
  gDriverY3.microsteps(64);
  gDriverY3.pwm_ofs_auto ();
  gDriverY3.pwm_autograd(1);
  gDriverY3.pwm_autoscale(1);
  gDriverY3.toff(5);

  Serial.println("Driver AOAT Enabled\n");
  gDriverAOAT.begin();
  gDriverAOAT.rms_current(900); // ma
  gDriverAOAT.microsteps(64);
  gDriverAOAT.pwm_ofs_auto ();
  gDriverAOAT.pwm_autograd(1);
  gDriverAOAT.pwm_autoscale(1);
  gDriverAOAT.toff(5);

  Serial.println("Driver AOAB Enabled\n");
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

// ============================================================================
// EXTI Interrupt Handlers - dispatch to limit switch ISRs
// ============================================================================
extern "C" {

void EXTI9_5_IRQHandler(void) {
    // PG6 - Motor 0 limit (x0)
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
        x0HomeIsr();
    }
    // PG9 - Motor 2 limit (y1)
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_9) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);
        y1HomeIsr();
    }
}

void EXTI15_10_IRQHandler(void) {
    // PG10 - Motor 4 limit (y3)
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);
        y3HomeIsr();
    }
    // PG11 - Motor 6 limit (aoab)
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
        aoabHomeIsr();
    }
    // PG12 - Motor 1 limit (y0)
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
        y0HomeIsr();
    }
    // PG13 - Motor 3 limit (y2)
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
        y2HomeIsr();
    }
    // PG14 - Motor 5 limit (aoat)
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_14);
        aoatHomeIsr();
    }
    // PG15 - Motor 7 limit (x1)
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);
        x1HomeIsr();
    }
}

} // extern "C"