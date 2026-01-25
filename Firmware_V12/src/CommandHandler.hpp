/**
 * @file CommandHandler.hpp
 * @brief Serial command handler with FreeRTOS thread
 * 
 * Handles serial input parsing and routes commands to appropriate handlers:
 * - Script commands to Lua executor queue
 * - System commands (STATUS, ABORT, BOOTLOADER) handled directly
 */

#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include "stm32f4xx_hal.h"
#include <STM32FreeRTOS.h>
#include <Seeed_Arduino_ooFreeRTOS.h>
#include "thread.hpp"
#include "queue.hpp"
#include "ticks.hpp"
#include "HalSerial.hpp"
#include "LuaRuntime.hpp"

// ============================================================================
// Constants for Serial Protocol
// ============================================================================

static const int COMMAND_BUFFER_SIZE = 256;   // Serial input buffer size

// Command markers for serial protocol
static const char START_MARKER = '<';
static const char END_MARKER = '>';

// Command string constants
static const char* const CMD_ABORT = "ABORT";
static const char* const CMD_SCRIPT = "SCRIPT:";
static const char* const CMD_STATUS = "STATUS";
static const char* const CMD_BOOTLOADER = "BOOTLOADER";

// ============================================================================
// Bootloader Support
// ============================================================================

// Bootloader flag in no-init section (survives warm boot)
extern volatile uint32_t gBootloaderFlag;

#define BOOTLOADER_FLAG_VALUE   0xDEADBEEF
#define BOOTLOADER_ADDRESS      0x1FFF0000

/**
 * @brief Reboot into DFU bootloader mode
 */
void rebootToBootloader(void);

/**
 * @brief Check bootloader flag and jump if set (call early in startup)
 */
void checkBootloaderFlag(void);

// ============================================================================
// Command Handler Thread
// ============================================================================

/**
 * @brief Thread that handles serial input and routes commands to appropriate queues
 */
class CommandHandlerThread : public cpp_freertos::Thread {
public:
    /**
     * @brief Construct and start the command handler thread
     * @param scriptQueue Queue for sending script messages to Lua executor
     * @param serialLock Mutex for serial output synchronization
     */
    CommandHandlerThread(cpp_freertos::Queue& scriptQueue, cpp_freertos::Mutex& serialLock);

protected:
    virtual void Run() override;

private:
    cpp_freertos::Queue& mScriptQueue;
    cpp_freertos::Mutex& mSerialLock;
    
    // Receive buffer
    char mReceiveBuffer[COMMAND_BUFFER_SIZE];
    int mBufferIndex;
    bool mReceiving;
    
    /**
     * @brief Process a received command and route to appropriate handler
     * @param cmd The received command string (without markers)
     */
    void processCommand(const char* cmd);
    
    /**
     * @brief Handle ABORT command
     */
    void handleAbort();
    
    /**
     * @brief Handle STATUS command
     */
    void handleStatus();
    
    /**
     * @brief Handle SCRIPT command
     * @param scriptStart Pointer to start of script text (after "SCRIPT:")
     */
    void handleScript(const char* scriptStart);
    
    /**
     * @brief Handle direct Lua script (no prefix)
     * @param script The script text
     */
    void handleDirectScript(const char* script);
    
    /**
     * @brief Print help/welcome message
     */
    void printHelp();
};

#endif // COMMAND_HANDLER_HPP
