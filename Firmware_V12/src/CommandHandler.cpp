/**
 * @file CommandHandler.cpp
 * @brief Serial command handler implementation
 */

#include "CommandHandler.hpp"
#include <cstring>

// External serial instance (defined in main.cpp)
extern HalSerial Serial;

// ============================================================================
// Bootloader Flag (placed in .noinit section by linker)
// ============================================================================

__attribute__((section(".noinit"))) volatile uint32_t gBootloaderFlag;

// ============================================================================
// Bootloader Support Implementation
// ============================================================================

void rebootToBootloader(void) {
    gBootloaderFlag = BOOTLOADER_FLAG_VALUE;
    NVIC_SystemReset();
}

void checkBootloaderFlag(void) {
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
// CommandHandlerThread Implementation
// ============================================================================

CommandHandlerThread::CommandHandlerThread(osMessageQueueId_t scriptQueue, osMutexId_t serialLock)
    : mScriptQueue(scriptQueue)
    , mSerialLock(serialLock)
    , mBufferIndex(0)
    , mReceiving(false)
{
    memset(mReceiveBuffer, 0, COMMAND_BUFFER_SIZE);
    osThreadAttr_t attr = {};
    attr.name       = "CmdHandler";
    attr.stack_size = 512 * 4;
    attr.priority   = osPriorityNormal;
    mHandle = osThreadNew(threadEntry, this, &attr);
}

void CommandHandlerThread::threadEntry(void* arg) {
    static_cast<CommandHandlerThread*>(arg)->run();
}

void CommandHandlerThread::run() {
    Serial.println("[CMD] Command handler thread started");
    
    while (true) {
        // Small delay to prevent tight loop
        osDelay(10);
        
        osMutexAcquire(mSerialLock, osWaitForever);
        
        int c;
        while ((c = Serial.readNonBlocking()) >= 0) {
            char ch = (char)c;
            
            if (ch == START_MARKER) {
                // Start of new message
                mReceiving = true;
                mBufferIndex = 0;
                memset(mReceiveBuffer, 0, COMMAND_BUFFER_SIZE);
            }
            else if (ch == END_MARKER && mReceiving) {
                // End of message - process it
                mReceiving = false;
                mReceiveBuffer[mBufferIndex] = '\0';
                processCommand(mReceiveBuffer);
            }
            else if (mReceiving) {
                // Accumulate characters
                if (mBufferIndex < COMMAND_BUFFER_SIZE - 1) {
                    mReceiveBuffer[mBufferIndex++] = ch;
                }
            }
        }
        
        osMutexRelease(mSerialLock);
    }
}

void CommandHandlerThread::processCommand(const char* cmd) {
    Serial.print("[CMD] Received: ");
    Serial.println(cmd);
    
    // Check for BOOTLOADER command
    if (strncmp(cmd, CMD_BOOTLOADER, strlen(CMD_BOOTLOADER)) == 0) {
        Serial.println("[CMD] BOOTLOADER command - rebooting to DFU mode...");
        HAL_Delay(100);  // Let message transmit
        rebootToBootloader();
        return;
    }
    
    // Check for ABORT command
    if (strncmp(cmd, CMD_ABORT, strlen(CMD_ABORT)) == 0) {
        handleAbort();
        return;
    }
    
    // Check for STATUS command
    if (strncmp(cmd, CMD_STATUS, strlen(CMD_STATUS)) == 0) {
        handleStatus();
        return;
    }
    
    // Check for SCRIPT command
    if (strncmp(cmd, CMD_SCRIPT, strlen(CMD_SCRIPT)) == 0) {
        handleScript(cmd + strlen(CMD_SCRIPT));
        return;
    }
    
    // Unknown command - treat as direct Lua script for convenience
    handleDirectScript(cmd);
}

void CommandHandlerThread::handleAbort() {
    Serial.println("[CMD] ABORT command received");
    
    LuaQueueMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = LuaMessageType::ABORT;
    msg.payloadLength = 0;
    
    // Send abort message - this has highest priority
    if (osMessageQueuePut(mScriptQueue, &msg, 0, 100) != osOK) {
        // If queue is full, force abort anyway
        LuaRuntime::requestAbort();
    }
}

void CommandHandlerThread::handleStatus() {
    Serial.println("[CMD] STATUS command received");
    
    LuaQueueMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = LuaMessageType::STATUS_REQUEST;
    msg.payloadLength = 0;
    
    osMessageQueuePut(mScriptQueue, &msg, 0, 100);
    
    // Also print system-level status
    Serial.println("");
    Serial.println("=== SYSTEM STATUS ===");
    Serial.print("Free heap: ");
    Serial.print(static_cast<uint32_t>(xPortGetFreeHeapSize()));
    Serial.println(" bytes");
    Serial.println("=====================");
}

void CommandHandlerThread::handleScript(const char* scriptStart) {
    size_t scriptLen = strlen(scriptStart);
    
    if (scriptLen == 0) {
        Serial.println("[CMD] ERROR: Empty script");
        return;
    }
    
    if (scriptLen >= MAX_SCRIPT_SIZE) {
        Serial.println("[CMD] ERROR: Script too large");
        return;
    }
    
    Serial.println("[CMD] SCRIPT command received");
    
    LuaQueueMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = LuaMessageType::SCRIPT;
    strncpy(msg.payload, scriptStart, MAX_SCRIPT_SIZE - 1);
    msg.payloadLength = scriptLen;
    
    if (osMessageQueuePut(mScriptQueue, &msg, 0, 100) == osOK) {
        Serial.println("[CMD] Script queued successfully");
    } else {
        Serial.println("[CMD] ERROR: Script queue full");
    }
}

void CommandHandlerThread::handleDirectScript(const char* script) {
    size_t scriptLen = strlen(script);
    
    if (scriptLen == 0) {
        return;  // Ignore empty commands
    }
    
    if (scriptLen >= MAX_SCRIPT_SIZE) {
        Serial.println("[CMD] ERROR: Script too large");
        return;
    }
    
    LuaQueueMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = LuaMessageType::SCRIPT;
    strncpy(msg.payload, script, MAX_SCRIPT_SIZE - 1);
    msg.payloadLength = scriptLen;
    
    if (osMessageQueuePut(mScriptQueue, &msg, 0, 100) == osOK) {
        Serial.println("[CMD] Direct script queued");
    } else {
        Serial.println("[CMD] ERROR: Script queue full");
    }
}

void CommandHandlerThread::printHelp() {
    Serial.println("");
    Serial.println("Commands:");
    Serial.println("  <SCRIPT:lua code here> - Queue a Lua script");
    Serial.println("  <ABORT>                - Abort current script and clear queue");
    Serial.println("  <STATUS>               - Show system status");
    Serial.println("  <BOOTLOADER>           - Reboot to DFU bootloader");
    Serial.println("  <lua code>             - Direct Lua execution");
    Serial.println("");
}
