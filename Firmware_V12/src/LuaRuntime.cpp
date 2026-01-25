/**
 * @file LuaRuntime.cpp
 * @brief Lua runtime implementation with FreeRTOS thread and abort support
 */

#include "LuaRuntime.hpp"
#include <cstring>

// External serial instance (defined in main.cpp)
extern HalSerial Serial;

// ============================================================================
// Static Member Initialization
// ============================================================================

volatile bool LuaRuntime::sAbortRequested = false;
cpp_freertos::MutexStandard* LuaRuntime::sAbortMutex = nullptr;

// ============================================================================
// LuaRuntime Implementation
// ============================================================================

LuaRuntime::LuaRuntime() 
    : mLuaState(nullptr)
    , mIsRunning(false) 
{
}

LuaRuntime::~LuaRuntime() {
    close();
}

void LuaRuntime::initAbortMutex() {
    if (!sAbortMutex) {
        sAbortMutex = new cpp_freertos::MutexStandard();
    }
}

void LuaRuntime::requestAbort() {
    if (sAbortMutex) {
        sAbortMutex->Lock();
        sAbortRequested = true;
        sAbortMutex->Unlock();
    }
}

void LuaRuntime::clearAbort() {
    if (sAbortMutex) {
        sAbortMutex->Lock();
        sAbortRequested = false;
        sAbortMutex->Unlock();
    }
}

bool LuaRuntime::isAbortRequested() {
    bool result = false;
    if (sAbortMutex) {
        sAbortMutex->Lock();
        result = sAbortRequested;
        sAbortMutex->Unlock();
    }
    return result;
}

void LuaRuntime::luaAbortHook(lua_State* L, lua_Debug* ar) {
    (void)ar; // Unused parameter
    
    if (sAbortRequested) {
        // Raise a Lua error to abort execution
        luaL_error(L, "Script aborted by user request");
    }
}

bool LuaRuntime::init() {
    mLuaState = luaL_newstate();
    if (!mLuaState) {
        Serial.println("[LUA] Failed to create Lua state");
        return false;
    }
    
    // Open standard libraries
    luaL_openlibs(mLuaState);
    
    // Register custom C functions
    registerCustomFunctions();
    
    Serial.println("[LUA] Runtime initialized");
    return true;
}

void LuaRuntime::close() {
    if (mLuaState) {
        lua_close(mLuaState);
        mLuaState = nullptr;
    }
    mIsRunning = false;
}

bool LuaRuntime::execute(const char* script) {
    if (!mLuaState) {
        Serial.println("[LUA] Runtime not initialized");
        return false;
    }
    
    // Clear any previous abort request
    clearAbort();
    mIsRunning = true;
    
    // Install debug hook for abort checking
    // LUA_MASKCOUNT: call hook every N instructions
    lua_sethook(mLuaState, luaAbortHook, LUA_MASKCOUNT, 100);
    
    Serial.println("[LUA] Executing script...");
    
    // Load and execute the script
    int loadResult = luaL_loadstring(mLuaState, script);
    if (loadResult != LUA_OK) {
        const char* err = lua_tostring(mLuaState, -1);
        Serial.print("[LUA] Load error: ");
        Serial.println(err ? err : "unknown error");
        lua_pop(mLuaState, 1);
        mIsRunning = false;
        return false;
    }
    
    // Execute with protected call
    int execResult = lua_pcall(mLuaState, 0, LUA_MULTRET, 0);
    
    // Remove debug hook
    lua_sethook(mLuaState, nullptr, 0, 0);
    mIsRunning = false;
    
    if (execResult != LUA_OK) {
        const char* err = lua_tostring(mLuaState, -1);
        if (isAbortRequested()) {
            Serial.println("[LUA] Script aborted");
        } else {
            Serial.print("[LUA] Execution error: ");
            Serial.println(err ? err : "unknown error");
        }
        lua_pop(mLuaState, 1);
        return false;
    }
    
    Serial.println("[LUA] Script completed successfully");
    return true;
}

void LuaRuntime::registerCustomFunctions() {
    // Register a print function that goes to UART
    lua_register(mLuaState, "serial_print", lua_serial_print);
    lua_register(mLuaState, "delay_ms", lua_delay_ms);
    
    // TODO: Add more custom functions for motor control, etc.
}

int LuaRuntime::lua_serial_print(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Serial.println(msg);
    return 0;
}

int LuaRuntime::lua_delay_ms(lua_State* L) {
    int ms = luaL_checkinteger(L, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    return 0;
}

// ============================================================================
// LuaExecutorThread Implementation
// ============================================================================

LuaExecutorThread::LuaExecutorThread(cpp_freertos::Queue& scriptQueue, cpp_freertos::Mutex& serialLock)
    : Thread("LuaExecutor", 2048, 1)  // Larger stack for Lua
    , mScriptQueue(scriptQueue)
    , mSerialLock(serialLock)
    , mScriptsExecuted(0)
    , mScriptsAborted(0)
{
    Start();
}

void LuaExecutorThread::Run() {
    Serial.println("[LUA] Lua executor thread started");
    
    // Initialize Lua runtime
    if (!mLuaRuntime.init()) {
        Serial.println("[LUA] FATAL: Failed to initialize Lua runtime");
        // Thread will exit - this is a fatal error
        return;
    }
    
    LuaQueueMessage msg;
    
    while (true) {
        // Wait for message from queue
        if (mScriptQueue.Dequeue(&msg, portMAX_DELAY)) {
            processMessage(msg);
        }
    }
}

void LuaExecutorThread::processMessage(LuaQueueMessage& msg) {
    switch (msg.type) {
        case LuaMessageType::SCRIPT:
            executeScript(msg.payload);
            break;
            
        case LuaMessageType::ABORT:
            handleAbort();
            break;
            
        case LuaMessageType::STATUS_REQUEST:
            printStatus();
            break;
    }
}

void LuaExecutorThread::executeScript(const char* script) {
    {
        cpp_freertos::LockGuard guard(mSerialLock);
        Serial.println("[LUA] Starting script execution");
    }
    
    bool success = mLuaRuntime.execute(script);
    
    {
        cpp_freertos::LockGuard guard(mSerialLock);
        if (success) {
            mScriptsExecuted++;
        } else if (LuaRuntime::isAbortRequested()) {
            mScriptsAborted++;
        }
    }
}

void LuaExecutorThread::handleAbort() {
    {
        cpp_freertos::LockGuard guard(mSerialLock);
        Serial.println("[LUA] Processing ABORT command");
    }
    
    // Request abort of current script
    LuaRuntime::requestAbort();
    
    // Clear all pending scripts from queue
    LuaQueueMessage discardMsg;
    int clearedCount = 0;
    
    while (mScriptQueue.Dequeue(&discardMsg, 0)) {
        clearedCount++;
    }
    
    {
        cpp_freertos::LockGuard guard(mSerialLock);
        Serial.print("[LUA] Cleared ");
        Serial.print(clearedCount);
        Serial.println(" pending scripts from queue");
        Serial.println("[LUA] ABORT complete");
    }
    
    // Clear abort flag for next script
    LuaRuntime::clearAbort();
}

void LuaExecutorThread::printStatus() {
    cpp_freertos::LockGuard guard(mSerialLock);
    Serial.println("");
    Serial.println("=== LUA STATUS ===");
    Serial.print("Scripts executed: ");
    Serial.print(mScriptsExecuted);
    Serial.println("");
    Serial.print("Scripts aborted: ");
    Serial.print(mScriptsAborted);
    Serial.println("");
    Serial.print("Script running: ");
    Serial.println(mLuaRuntime.scriptRunning() ? "YES" : "NO");
    Serial.println("==================");
    Serial.println("");
}
