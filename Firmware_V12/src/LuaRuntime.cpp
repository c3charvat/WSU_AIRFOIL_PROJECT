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
osMutexId_t   LuaRuntime::sAbortMutex     = nullptr;

// Forward declaration: flag polled by move_function to stop motors mid-move.
extern volatile bool gMotionAbortFlag;

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
        sAbortMutex = osMutexNew(NULL);
    }
}

void LuaRuntime::requestAbort() {
    if (sAbortMutex) {
        osMutexAcquire(sAbortMutex, osWaitForever);
        sAbortRequested = true;
        osMutexRelease(sAbortMutex);
    }
    gMotionAbortFlag = true;
}

void LuaRuntime::clearAbort() {
    if (sAbortMutex) {
        osMutexAcquire(sAbortMutex, osWaitForever);
        sAbortRequested = false;
        osMutexRelease(sAbortMutex);
    }
    gMotionAbortFlag = false;
}

bool LuaRuntime::isAbortRequested() {
    bool result = false;
    if (sAbortMutex) {
        osMutexAcquire(sAbortMutex, osWaitForever);
        result = sAbortRequested;
        osMutexRelease(sAbortMutex);
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

    // Register motion control functions
    MovementLua::registerFunctions(mLuaState);
}

int LuaRuntime::lua_serial_print(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Serial.println(msg);
    return 0;
}

int LuaRuntime::lua_delay_ms(lua_State* L) {
    int ms = luaL_checkinteger(L, 1);
    osDelay(ms);
    return 0;
}

// ============================================================================
// LuaExecutorThread Implementation
// ============================================================================

LuaExecutorThread::LuaExecutorThread(osMessageQueueId_t scriptQueue, osMutexId_t serialLock)
    : mScriptQueue(scriptQueue)
    , mSerialLock(serialLock)
    , mScriptsExecuted(0)
    , mScriptsAborted(0)
{
    osThreadAttr_t attr = {};
    attr.name       = "LuaExecutor";
    attr.stack_size = 2048 * 4;  // larger stack for Lua
    attr.priority   = osPriorityBelowNormal;
    mHandle = osThreadNew(threadEntry, this, &attr);
}

void LuaExecutorThread::threadEntry(void* arg) {
    static_cast<LuaExecutorThread*>(arg)->run();
}

void LuaExecutorThread::run() {
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
        if (osMessageQueueGet(mScriptQueue, &msg, NULL, osWaitForever) == osOK) {
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
        osMutexAcquire(mSerialLock, osWaitForever);
        Serial.println("[LUA] Starting script execution");
        osMutexRelease(mSerialLock);
    }
    
    bool success = mLuaRuntime.execute(script);
    
    {
        osMutexAcquire(mSerialLock, osWaitForever);
        if (success) {
            mScriptsExecuted++;
        } else if (LuaRuntime::isAbortRequested()) {
            mScriptsAborted++;
        }
        osMutexRelease(mSerialLock);
    }
}

void LuaExecutorThread::handleAbort() {
    {
        osMutexAcquire(mSerialLock, osWaitForever);
        Serial.println("[LUA] Processing ABORT command");
        osMutexRelease(mSerialLock);
    }
    
    LuaRuntime::requestAbort();
    
    LuaQueueMessage discardMsg;
    int clearedCount = 0;
    
    while (osMessageQueueGet(mScriptQueue, &discardMsg, NULL, 0) == osOK) {
        clearedCount++;
    }
    
    {
        osMutexAcquire(mSerialLock, osWaitForever);
        Serial.print("[LUA] Cleared ");
        Serial.print(clearedCount);
        Serial.println(" pending scripts from queue");
        Serial.println("[LUA] ABORT complete");
        osMutexRelease(mSerialLock);
    }
    
    // Clear abort flag for next script
    LuaRuntime::clearAbort();
}

void LuaExecutorThread::printStatus() {
    osMutexAcquire(mSerialLock, osWaitForever);
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
    osMutexRelease(mSerialLock);
}
