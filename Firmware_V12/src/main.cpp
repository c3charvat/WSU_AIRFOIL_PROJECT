/**
 * @file main.cpp
 * @brief Main firmware entry point with FreeRTOS-based Lua script queue system
 * 
 * Architecture:
 * - CommandHandlerThread: Reads serial commands, parses them, routes to Lua queue
 * - LuaExecutorThread: Executes Lua scripts from queue with abort capability
 * - Script Queue: Holds complete Lua scripts waiting to be executed
 * - Abort mechanism: Special command clears queue and aborts running script
 * 
 * See CommandHandler.hpp and LuaRuntime.hpp for thread implementations.
 */

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "HalSerial.hpp"
#include "HalGpio.hpp"
#include "CommandHandler.hpp"
#include "LuaRuntime.hpp"

// ============================================================================
// Global Serial Instance
// ============================================================================

// USART3 on PD8 (TX) and PD9 (RX) - exposed on Raspberry Pi header
HalSerial Serial(USART3, GPIOD, GPIO_PIN_8, GPIO_PIN_9, GPIO_AF7_USART3);

// ============================================================================
// Global Motion State
// ============================================================================

// Tracks the machine's current position as moves complete.
// Updated by move_function() after every move; valid at any inter-move abort point.
PositionStruct gCurrentPosition = {0.0f, 0.0f, 0.0f, 0.0f, nullptr};
Error gError;

// AOA encoder counts recorded at the end of the last home_all().
// Used by sync_aoa_position() to convert raw counts to degrees without re-homing.
uint16_t gAoatEncoderZero = 0;
uint16_t gAoabEncoderZero = 0;

// Set by LuaRuntime::requestAbort(); polled inside move_function's stepper loop.
// Allows a running move to be interrupted without waiting for it to finish.
volatile bool gMotionAbortFlag = false;
// Set true when move_function halts a move early due to gMotionAbortFlag.
// Cleared by move_function at the start of each new move.
volatile bool gMotionWasAborted = false;
// Abort sequence:
// User sends <ABORT>
//   → CommandHandlerThread::handleAbort()
//   → LuaRuntime::requestAbort()
//       sets sAbortRequested = true       ← catches abort between Lua instructions (existing)
//       sets gMotionAbortFlag = true      ← NEW: catches abort inside a running move
  
//   Inside move_function's stepper loop:
//       detects gMotionAbortFlag
//       calls setupStop() on all 8 axes  ← controlled deceleration, no mechanical shock
//       polls processMovement() until decel complete
//       computes X/Y displacement from actual step counts
//       sets gMotionWasAborted = true
//       returns early
  
//   Back in lua_move_absolute / lua_move_relative:
//       sees gMotionWasAborted → calls luaL_error()
//       lua_pcall() catches the error → script terminates cleanly


// ============================================================================
// System Clock Configuration (Forward Declaration)
// ============================================================================

extern "C" void SystemClock_Config(void);
extern "C" void Error_Handler(void);

// ============================================================================
// Main Entry Point
// ============================================================================

int main(void) {
    // ========================================================================
    // Early Initialization (before any peripheral init)
    // ========================================================================
    
    // Enable FPU (Cortex-M4 with FPU) - MUST be done before any float operations
    // Set CP10 and CP11 to full access
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));
    __DSB();  // Data Synchronization Barrier
    __ISB();  // Instruction Synchronization Barrier
    
    // Check if we need to jump to bootloader (before any init)
    checkBootloaderFlag();
    
    // ========================================================================
    // HAL and Clock Initialization
    // ========================================================================
    
    HAL_Init();
    SystemClock_Config();
    
    // Initialize DWT for microsecond timing
    initDWT();
    
    // ========================================================================
    // Enable GPIO Clocks
    // ========================================================================
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    
    // ========================================================================
    // Initialize Serial Communication
    // ========================================================================
    
    Serial.begin(115200);
    HAL_Delay(100);
    
    Serial.println("");
    Serial.println("******************************");
    Serial.println("   Lua Script Queue System    ");
    Serial.println("******************************");
    Serial.println("");
    
    // ========================================================================
    // Create RTOS Objects
    // ========================================================================
    
    // Initialize CMSIS-RTOS v2 kernel (must be done before any osXxx calls)
    osKernelInitialize();

    // Initialize Lua abort mutex (must be done before threads start)
    LuaRuntime::initAbortMutex();

    // Create script queue and serial mutex
    static osMessageQueueId_t scriptQueue = osMessageQueueNew(SCRIPT_QUEUE_DEPTH, sizeof(LuaQueueMessage), NULL);
    static osMutexId_t        serialMutex = osMutexNew(NULL);
    
    // ========================================================================
    // Create Threads
    // ========================================================================
    
    // Command handler thread - parses serial input and routes to Lua queue
    static CommandHandlerThread cmdHandler(scriptQueue, serialMutex);
    
    // Lua executor thread - executes scripts from queue
    static LuaExecutorThread luaExecutor(scriptQueue, serialMutex);
    
    // Suppress unused variable warnings
    (void)cmdHandler;
    (void)luaExecutor;
    
    // ========================================================================
    // Print Help and Start Scheduler
    // ========================================================================
    
    printHelp();
    Serial.println("******************************");
    Serial.println("      Starting Scheduler      ");
    Serial.println("******************************");
    
    HAL_Delay(500);
    osKernelStart();
    
    // Should never reach here
    Serial.println("FATAL ERROR: Scheduler ended - Restart required");
    while (1);
}

// ============================================================================
// System Clock Configuration (DO NOT MODIFY)
// ============================================================================

/* 
 * For As long As the Octopus Board is used under no circumstances 
 * should this ever be modified!
 *
 * This section configures the system clock for the STM32F446ZET6.
 * The board runs at 168 MHz not the 8MHz external clock expected by default.
 */

extern "C" void SystemClock_Config(void) {
#ifdef OCTOPUS_BOARD_FROM_HSI
    /* boot from HSI, internal 16MHz RC, to 168MHz. **NO USB POSSIBLE**, needs HSE! */
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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | 
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
#else
    /* boot from HSE, crystal oscillator (12MHz) */
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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | 
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
    
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        Error_Handler();
    }
#endif
}

// ============================================================================
// Error Handler
// ============================================================================

extern "C" void Error_Handler(void) {
    __disable_irq();
    while (1) {
        // Hang here on error
    }
}
