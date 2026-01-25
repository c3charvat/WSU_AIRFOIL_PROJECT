/**
 * @file main.cpp
 * @brief Main firmware entry point with FreeRTOS-based Lua script queue system
 * 
 * Architecture:
 * - SerialInputThread: Reads serial commands, parses them, routes to appropriate queue
 * - LuaExecutorThread: Executes Lua scripts from queue with abort capability via debug hooks
 * - Script Queue: Holds complete Lua scripts waiting to be executed
 * - Abort mechanism: Special command clears queue and aborts running script
 */

#include "stm32f4xx_hal.h"
#include <STM32FreeRTOS.h>
#include <Seeed_Arduino_ooFreeRTOS.h>
#include "task.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"
#include "luapatch.h"
#include "HalSerial.hpp"
#include "HalGpio.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

using namespace cpp_freertos;

// ============================================================================
// Global Serial Instance
// ============================================================================

// USART3 on PD8 (TX) and PD9 (RX) - exposed on Raspberry Pi header
HalSerial Serial(USART3, GPIOD, GPIO_PIN_8, GPIO_PIN_9, GPIO_AF7_USART3);

// ============================================================================
// Bootloader Support
// ============================================================================

// Bootloader flag in no-init section (survives warm boot)
__attribute__((section(".noinit"))) volatile uint32_t gBootloaderFlag;
#define BOOTLOADER_FLAG_VALUE   0xDEADBEEF
#define BOOTLOADER_ADDRESS      0x1FFF0000
}

// ============================================================================
// Constants and Configuration
// ============================================================================

static const int MAX_SCRIPT_SIZE = 1024;      // Maximum Lua script size in bytes
static const int SCRIPT_QUEUE_DEPTH = 5;      // Number of scripts that can be queued
static const int COMMAND_BUFFER_SIZE = 256;   // Serial input buffer size

// Command markers for serial protocol
static const char START_MARKER = '<';
static const char END_MARKER = '>';

// Special command prefixes
static const char* CMD_ABORT = "ABORT";
static const char* CMD_SCRIPT = "SCRIPT:";
static const char* CMD_STATUS = "STATUS";
static const char* CMD_BOOTLOADER = "BOOTLOADER";

// ============================================================================
// Message Types for Queue System
// ============================================================================

enum class MessageType : uint8_t {
    SCRIPT,         // Lua script to execute
    ABORT,          // Abort current script and clear queue
    STATUS_REQUEST  // Request system status
};

struct QueueMessage {
    MessageType type;
    char payload[MAX_SCRIPT_SIZE];
    uint16_t payloadLength;
};

// ============================================================================
// Global State for Lua Abort Mechanism
// ============================================================================

// Volatile flag checked by Lua debug hook to trigger abort
static volatile bool gAbortRequested = false;

// Mutex to protect abort flag access
static MutexStandard* gAbortMutex = nullptr;

// ============================================================================
// Lua Debug Hook for Abort Support
// ============================================================================

/**
 * @brief Lua debug hook called periodically during script execution
 * 
 * This hook is installed when a script starts and checks the abort flag.
 * When abort is requested, it raises a Lua error to terminate execution.
 */
static void luaAbortHook(lua_State* L, lua_Debug* ar) {
    (void)ar; // Unused parameter
    
    if (gAbortRequested) {
        // Raise a Lua error to abort execution
        luaL_error(L, "Script aborted by user request");
    }
}

/**
 * @brief Set the abort flag (thread-safe)
 */
static void requestAbort() {
    if (gAbortMutex) {
        gAbortMutex->Lock();
        gAbortRequested = true;
        gAbortMutex->Unlock();
    }
}

/**
 * @brief Clear the abort flag (thread-safe)
 */
static void clearAbort() {
    if (gAbortMutex) {
        gAbortMutex->Lock();
        gAbortRequested = false;
        gAbortMutex->Unlock();
    }
}

/**
 * @brief Check if abort was requested (thread-safe)
 */
static bool isAbortRequested() {
    bool result = false;
    if (gAbortMutex) {
        gAbortMutex->Lock();
        result = gAbortRequested;
        gAbortMutex->Unlock();
    }
    return result;
}

// ============================================================================
// Bootloader Jump
// ============================================================================

/**
 * @brief Reboot into DFU bootloader mode
 */
static void rebootToBootloader(void) {
    gBootloaderFlag = BOOTLOADER_FLAG_VALUE;
    NVIC_SystemReset();
}

/**
 * @brief Check bootloader flag and jump if set (call early in startup)
 */
static void checkBootloaderFlag(void) {
    if (gBootloaderFlag == BOOTLOADER_FLAG_VALUE) {
        gBootloaderFlag = 0;
        
        // Deinit HAL and clocks
        HAL_RCC_DeInit();
        HAL_DeInit();
        
        // Disable SysTick
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;
        
        // Get bootloader address
        uint32_t bootloaderAddr = BOOTLOADER_ADDRESS;
        uint32_t jumpAddr = *(__IO uint32_t*)(bootloaderAddr + 4);
        
        // Set MSP and jump
        __set_MSP(*(__IO uint32_t*)bootloaderAddr);
        void (*jumpToBootloader)(void) = (void (*)(void))jumpAddr;
        jumpToBootloader();
        
        // Should never reach here
        while (1);
    }
}

// ============================================================================
// Lua Runtime Wrapper
// ============================================================================

/**
 * @brief Wrapper class that manages Lua state with abort capability
 */
class LuaRuntime {
public:
    LuaRuntime() : L(nullptr), isRunning(false) {}
    
    ~LuaRuntime() {
        close();
    }
    
    /**
     * @brief Initialize Lua state and standard libraries
     * @return true if successful
     */
    bool init() {
        L = luaL_newstate();
        if (!L) {
            Serial.println("[LUA] Failed to create Lua state");
            return false;
        }
        
        // Open standard libraries
        luaL_openlibs(L);
        
        // Register custom C functions here
        registerCustomFunctions();
        
        Serial.println("[LUA] Runtime initialized");
        return true;
    }
    
    /**
     * @brief Close Lua state
     */
    void close() {
        if (L) {
            lua_close(L);
            L = nullptr;
        }
        isRunning = false;
    }
    
    /**
     * @brief Execute a Lua script with abort support
     * @param script The Lua script to execute
     * @return true if script completed successfully, false if error or aborted
     */
    bool execute(const char* script) {
        if (!L) {
            Serial.println("[LUA] Runtime not initialized");
            return false;
        }
        
        // Clear any previous abort request
        clearAbort();
        isRunning = true;
        
        // Install debug hook for abort checking
        // LUA_MASKCOUNT: call hook every N instructions
        lua_sethook(L, luaAbortHook, LUA_MASKCOUNT, 100);
        
        Serial.println("[LUA] Executing script...");
        
        // Load and execute the script
        int loadResult = luaL_loadstring(L, script);
        if (loadResult != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            Serial.print("[LUA] Load error: ");
            Serial.println(err ? err : "unknown error");
            lua_pop(L, 1);
            isRunning = false;
            return false;
        }
        
        // Execute with protected call
        int execResult = lua_pcall(L, 0, LUA_MULTRET, 0);
        
        // Remove debug hook
        lua_sethook(L, nullptr, 0, 0);
        isRunning = false;
        
        if (execResult != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            if (isAbortRequested()) {
                Serial.println("[LUA] Script aborted");
            } else {
                Serial.print("[LUA] Execution error: ");
                Serial.println(err ? err : "unknown error");
            }
            lua_pop(L, 1);
            return false;
        }
        
        Serial.println("[LUA] Script completed successfully");
        return true;
    }
    
    /**
     * @brief Check if a script is currently running
     */
    bool scriptRunning() const {
        return isRunning;
    }
    
private:
    lua_State* L;
    volatile bool isRunning;
    
    /**
     * @brief Register custom C functions available to Lua scripts
     */
    void registerCustomFunctions() {
        // Example: Register a print function that goes to UART
        lua_register(L, "serial_print", lua_serial_print);
        lua_register(L, "delay_ms", lua_delay_ms);
        // Add more custom functions as needed for motor control, etc.
    }
    
    // Custom Lua functions
    static int lua_serial_print(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        Serial.println(msg);
        return 0;
    }
    
    static int lua_delay_ms(lua_State* L) {
        int ms = luaL_checkinteger(L, 1);
        vTaskDelay(pdMS_TO_TICKS(ms));
        return 0;
    }
};

// ============================================================================
// Serial Input Thread
// ============================================================================

/**
 * @brief Thread that handles serial input and routes commands to appropriate queues
 */
class SerialInputThread : public Thread {
public:
    SerialInputThread(Queue& scriptQueue, Mutex& serialLock)
        : Thread("SerialInput", 512, 2),
          mScriptQueue(scriptQueue),
          mSerialLock(serialLock)
    {
        Start();
    }

protected:
    virtual void Run() override {
        Serial.println("[INPUT] Serial input thread started");
        
        char receiveBuffer[COMMAND_BUFFER_SIZE] = {0};
        int bufferIndex = 0;
        bool receiving = false;
        
        while (true) {
            // Small delay to prevent tight loop
            Delay(Ticks::MsToTicks(10));
            
            LockGuard guard(mSerialLock);
            
            int c;
            while ((c = Serial.readNonBlocking()) >= 0) {
                char ch = (char)c;
                
                if (ch == START_MARKER) {
                    // Start of new message
                    receiving = true;
                    bufferIndex = 0;
                    memset(receiveBuffer, 0, COMMAND_BUFFER_SIZE);
                }
                else if (ch == END_MARKER && receiving) {
                    // End of message - process it
                    receiving = false;
                    receiveBuffer[bufferIndex] = '\0';
                    processCommand(receiveBuffer);
                }
                else if (receiving) {
                    // Accumulate characters
                    if (bufferIndex < COMMAND_BUFFER_SIZE - 1) {
                        receiveBuffer[bufferIndex++] = ch;
                    }
                }
            }
        }
    }

private:
    Queue& mScriptQueue;
    Mutex& mSerialLock;
    
    /**
     * @brief Process a received command and route to appropriate handler
     */
    void processCommand(const char* cmd) {
        Serial.print("[INPUT] Received: ");
        Serial.println(cmd);
        
        QueueMessage msg;
        memset(&msg, 0, sizeof(msg));
        
        // Check for BOOTLOADER command
        if (strncmp(cmd, CMD_BOOTLOADER, strlen(CMD_BOOTLOADER)) == 0) {
            Serial.println("[INPUT] BOOTLOADER command - rebooting to DFU mode...");
            HAL_Delay(100);  // Let message transmit
            rebootToBootloader();
            return;
        }
        
        // Check for ABORT command
        if (strncmp(cmd, CMD_ABORT, strlen(CMD_ABORT)) == 0) {
            Serial.println("[INPUT] ABORT command received");
            msg.type = MessageType::ABORT;
            msg.payloadLength = 0;
            
            // Send abort message - this has highest priority
            if (!mScriptQueue.Enqueue(&msg, Ticks::MsToTicks(100))) {
                // If queue is full, force abort anyway
                requestAbort();
            }
            return;
        }
        
        // Check for STATUS command
        if (strncmp(cmd, CMD_STATUS, strlen(CMD_STATUS)) == 0) {
            Serial.println("[INPUT] STATUS command received");
            msg.type = MessageType::STATUS_REQUEST;
            msg.payloadLength = 0;
            mScriptQueue.Enqueue(&msg, Ticks::MsToTicks(100));
            return;
        }
        
        // Check for SCRIPT command
        if (strncmp(cmd, CMD_SCRIPT, strlen(CMD_SCRIPT)) == 0) {
            const char* scriptStart = cmd + strlen(CMD_SCRIPT);
            size_t scriptLen = strlen(scriptStart);
            
            if (scriptLen > 0 && scriptLen < MAX_SCRIPT_SIZE) {
                Serial.println("[INPUT] SCRIPT command received");
                msg.type = MessageType::SCRIPT;
                strncpy(msg.payload, scriptStart, MAX_SCRIPT_SIZE - 1);
                msg.payloadLength = scriptLen;
                
                if (mScriptQueue.Enqueue(&msg, Ticks::MsToTicks(100))) {
                    Serial.println("[INPUT] Script queued successfully");
                } else {
                    Serial.println("[INPUT] ERROR: Script queue full");
                }
            } else {
                Serial.println("[INPUT] ERROR: Script too large or empty");
            }
            return;
        }
        
        // Unknown command - treat as direct Lua script for convenience
        size_t cmdLen = strlen(cmd);
        if (cmdLen > 0 && cmdLen < MAX_SCRIPT_SIZE) {
            msg.type = MessageType::SCRIPT;
            strncpy(msg.payload, cmd, MAX_SCRIPT_SIZE - 1);
            msg.payloadLength = cmdLen;
            
            if (mScriptQueue.Enqueue(&msg, Ticks::MsToTicks(100))) {
                Serial.println("[INPUT] Direct script queued");
            } else {
                Serial.println("[INPUT] ERROR: Script queue full");
            }
        }
    }
};

// ============================================================================
// Lua Executor Thread
// ============================================================================

/**
 * @brief Thread that executes Lua scripts from the queue with abort support
 */
class LuaExecutorThread : public Thread {
public:
    LuaExecutorThread(Queue& scriptQueue, Mutex& serialLock)
        : Thread("LuaExecutor", 2048, 1),  // Larger stack for Lua
          mScriptQueue(scriptQueue),
          mSerialLock(serialLock),
          mScriptsExecuted(0),
          mScriptsAborted(0)
    {
        Start();
    }

protected:
    virtual void Run() override {
        Serial.println("[LUA] Lua executor thread started");
        
        // Initialize Lua runtime
        if (!mLuaRuntime.init()) {
            Serial.println("[LUA] FATAL: Failed to initialize Lua runtime");
            // Thread will exit - this is a fatal error
            return;
        }
        
        QueueMessage msg;
        
        while (true) {
            // Wait for message from queue
            if (mScriptQueue.Dequeue(&msg, portMAX_DELAY)) {
                processMessage(msg);
            }
        }
    }

private:
    Queue& mScriptQueue;
    Mutex& mSerialLock;
    LuaRuntime mLuaRuntime;
    uint32_t mScriptsExecuted;
    uint32_t mScriptsAborted;
    
    /**
     * @brief Process a message from the queue
     */
    void processMessage(QueueMessage& msg) {
        switch (msg.type) {
            case MessageType::SCRIPT:
                executeScript(msg.payload);
                break;
                
            case MessageType::ABORT:
                handleAbort();
                break;
                
            case MessageType::STATUS_REQUEST:
                printStatus();
                break;
        }
    }
    
    /**
     * @brief Execute a Lua script
     */
    void executeScript(const char* script) {
        {
            LockGuard guard(mSerialLock);
            Serial.println("[LUA] Starting script execution");
        }
        
        bool success = mLuaRuntime.execute(script);
        
        {
            LockGuard guard(mSerialLock);
            if (success) {
                mScriptsExecuted++;
            } else if (isAbortRequested()) {
                mScriptsAborted++;
            }
        }
    }
    
    /**
     * @brief Handle abort command - clear queue and set abort flag
     */
    void handleAbort() {
        {
            LockGuard guard(mSerialLock);
            Serial.println("[LUA] Processing ABORT command");
        }
        
        // Request abort of current script
        requestAbort();
        
        // Clear all pending scripts from queue
        QueueMessage discardMsg;
        int clearedCount = 0;
        
        while (mScriptQueue.Dequeue(&discardMsg, 0)) {
            clearedCount++;
        }
        
        {
            LockGuard guard(mSerialLock);
            Serial.print("[LUA] Cleared ");
            Serial.print(clearedCount);
            Serial.println(" pending scripts from queue");
            Serial.println("[LUA] ABORT complete");
        }
        
        // Clear abort flag for next script
        clearAbort();
    }
    
    /**
     * @brief Print system status
     */
    void printStatus() {
        LockGuard guard(mSerialLock);
        Serial.println("");
        Serial.println("=== SYSTEM STATUS ===");
        Serial.print("Scripts executed: ");
        Serial.print(mScriptsExecuted);
        Serial.println("");
        Serial.print("Scripts aborted: ");
        Serial.print(mScriptsAborted);
        Serial.println("");
        Serial.print("Script running: ");
        Serial.println(mLuaRuntime.scriptRunning() ? "YES" : "NO");
        Serial.print("Free heap: ");
        Serial.print(xPortGetFreeHeapSize());
        Serial.println("");
        Serial.println("=====================");
        Serial.println("");
    }
};

// ============================================================================
// Global RTOS Objects
// ============================================================================

static Queue* gScriptQueue = nullptr;
static MutexStandard* gSerialMutex = nullptr;

// ============================================================================
// Main Entry Point
// ============================================================================

int main(void) {
    // Enable FPU (Cortex-M4 with FPU) - MUST be done before any float operations
    // Set CP10 and CP11 to full access
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));
    __DSB();  // Data Synchronization Barrier
    __ISB();  // Instruction Synchronization Barrier
    
    // Check if we need to jump to bootloader (before any init)
    checkBootloaderFlag();
    
    // Initialize HAL
    HAL_Init();
    
    // Configure system clock
    SystemClock_Config();
    
    // Initialize DWT for microsecond timing
    initDWT();
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    
    // Initialize UART
    Serial.begin(115200);
    HAL_Delay(100);
    
    Serial.println("");
    Serial.println("******************************");
    Serial.println("   Lua Script Queue System    ");
    Serial.println("******************************");
    Serial.println("");
    
    // Create RTOS objects
    gScriptQueue = new Queue(SCRIPT_QUEUE_DEPTH, sizeof(QueueMessage));
    gSerialMutex = new MutexStandard();
    gAbortMutex = new MutexStandard();
    
    if (!gScriptQueue || !gSerialMutex || !gAbortMutex) {
        Serial.println("FATAL: Failed to create RTOS objects");
        while (1) { HAL_Delay(1000); }
    }
    
    // Create threads
    static SerialInputThread* inputThread = new SerialInputThread(*gScriptQueue, *gSerialMutex);
    static LuaExecutorThread* luaThread = new LuaExecutorThread(*gScriptQueue, *gSerialMutex);
    
    (void)inputThread;  // Suppress unused variable warning
    (void)luaThread;
    
    Serial.println("");
    Serial.println("Commands:");
    Serial.println("  <SCRIPT:lua code here> - Queue a Lua script");
    Serial.println("  <ABORT>                - Abort current script and clear queue");
    Serial.println("  <STATUS>               - Show system status");
    Serial.println("  <BOOTLOADER>           - Reboot to DFU bootloader");
    Serial.println("  <lua code>             - Direct Lua execution");
    Serial.println("");
    Serial.println("******************************");
    Serial.println("      Starting Scheduler      ");
    Serial.println("******************************");
    
    HAL_Delay(500);
    Thread::StartScheduler();
    
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
