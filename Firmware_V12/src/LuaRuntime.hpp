/**
 * @file LuaRuntime.hpp
 * @brief Lua runtime wrapper with FreeRTOS thread and abort support
 * 
 * Provides a thread-safe Lua execution environment with:
 * - Script queue for sequential execution
 * - Debug hook-based abort mechanism
 * - Custom C function registration
 */

#ifndef LUA_RUNTIME_HPP
#define LUA_RUNTIME_HPP

#include "stm32f4xx_hal.h"
#include <STM32FreeRTOS.h>
#include <Seeed_Arduino_ooFreeRTOS.h>
#include "thread.hpp"
#include "queue.hpp"
#include "ticks.hpp"
#include "HalSerial.hpp"

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

// ============================================================================
// Message Types for Lua Queue
// ============================================================================

static const int MAX_SCRIPT_SIZE = 1024;      // Maximum Lua script size in bytes
static const int SCRIPT_QUEUE_DEPTH = 5;      // Number of scripts that can be queued

enum class LuaMessageType : uint8_t {
    SCRIPT,         // Lua script to execute
    ABORT,          // Abort current script and clear queue
    STATUS_REQUEST  // Request system status
};

struct LuaQueueMessage {
    LuaMessageType type;
    char payload[MAX_SCRIPT_SIZE];
    uint16_t payloadLength;
};

// ============================================================================
// Lua Runtime Class
// ============================================================================

/**
 * @brief Wrapper class that manages Lua state with abort capability
 */
class LuaRuntime {
public:
    LuaRuntime();
    ~LuaRuntime();
    
    /**
     * @brief Initialize Lua state and standard libraries
     * @return true if successful
     */
    bool init();
    
    /**
     * @brief Close Lua state and free resources
     */
    void close();
    
    /**
     * @brief Execute a Lua script with abort support
     * @param script The Lua script to execute
     * @return true if script completed successfully, false if error or aborted
     */
    bool execute(const char* script);
    
    /**
     * @brief Check if a script is currently running
     */
    bool scriptRunning() const { return mIsRunning; }
    
    /**
     * @brief Request abort of currently running script (thread-safe)
     */
    static void requestAbort();
    
    /**
     * @brief Clear the abort flag (thread-safe)
     */
    static void clearAbort();
    
    /**
     * @brief Check if abort was requested (thread-safe)
     */
    static bool isAbortRequested();
    
    /**
     * @brief Initialize the abort mutex (call once at startup)
     */
    static void initAbortMutex();

private:
    lua_State* mLuaState;
    volatile bool mIsRunning;
    
    // Static abort mechanism shared across instances
    static volatile bool sAbortRequested;
    static cpp_freertos::MutexStandard* sAbortMutex;
    
    /**
     * @brief Register custom C functions available to Lua scripts
     */
    void registerCustomFunctions();
    
    /**
     * @brief Lua debug hook for abort checking
     */
    static void luaAbortHook(lua_State* L, lua_Debug* ar);
    
    // Custom Lua C functions
    static int lua_serial_print(lua_State* L);
    static int lua_delay_ms(lua_State* L);
    // Add more custom function declarations here
};

// ============================================================================
// Lua Executor Thread
// ============================================================================

/**
 * @brief Thread that executes Lua scripts from the queue with abort support
 */
class LuaExecutorThread : public cpp_freertos::Thread {
public:
    /**
     * @brief Construct and start the Lua executor thread
     * @param scriptQueue Queue for receiving script messages
     * @param serialLock Mutex for serial output synchronization
     */
    LuaExecutorThread(cpp_freertos::Queue& scriptQueue, cpp_freertos::Mutex& serialLock);
    
    /**
     * @brief Get number of scripts successfully executed
     */
    uint32_t getScriptsExecuted() const { return mScriptsExecuted; }
    
    /**
     * @brief Get number of scripts aborted
     */
    uint32_t getScriptsAborted() const { return mScriptsAborted; }
    
    /**
     * @brief Check if Lua runtime is currently executing a script
     */
    bool isScriptRunning() const { return mLuaRuntime.scriptRunning(); }

protected:
    virtual void Run() override;

private:
    cpp_freertos::Queue& mScriptQueue;
    cpp_freertos::Mutex& mSerialLock;
    LuaRuntime mLuaRuntime;
    uint32_t mScriptsExecuted;
    uint32_t mScriptsAborted;
    
    void processMessage(LuaQueueMessage& msg);
    void executeScript(const char* script);
    void handleAbort();
    void printStatus();
};

#endif // LUA_RUNTIME_HPP
