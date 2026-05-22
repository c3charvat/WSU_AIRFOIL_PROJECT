/**
 * @file Arduino.h
 * @brief Minimal Arduino compatibility shim for STM32Cube builds.
 *
 * Provides the constants, timing functions, and stub classes that
 * TMCStepper references so the library compiles without the Arduino framework.
 * Hardware SPI (used by TMC2130/TMC2660) is stubbed as no-ops; those drivers
 * are not used by this firmware.  TMC2208/TMC2209 (UART) work via HWSerial.
 */
#pragma once
#include <stdint.h>
#include "stm32f4xx_hal.h"

// ---------------------------------------------------------------------------
// GPIO constants
// ---------------------------------------------------------------------------
#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW  0x0
#endif
#ifndef OUTPUT
#define OUTPUT 0x1
#endif
#ifndef INPUT
#define INPUT  0x0
#endif
#ifndef INPUT_PULLUP
#define INPUT_PULLUP 0x2
#endif

// ---------------------------------------------------------------------------
// GPIO stub functions (SSwitch & SW_SPI are not actively used)
// ---------------------------------------------------------------------------
inline void pinMode(uint16_t /*pin*/, uint8_t /*mode*/) {}
inline void digitalWrite(uint16_t /*pin*/, uint8_t /*val*/) {}
inline int  digitalRead(uint16_t /*pin*/) { return 0; }

// ---------------------------------------------------------------------------
// Timing (map to STM32 HAL tick, which runs at 1 kHz by default)
// ---------------------------------------------------------------------------
#ifndef DELAY_DEFINED
#define DELAY_DEFINED
inline void     delay(uint32_t ms) { HAL_Delay(ms); }
#endif
inline uint32_t millis()           { return HAL_GetTick(); }

// ---------------------------------------------------------------------------
// SPI stubs – TMC2130 / TMC2660 source files reference these but those
// drivers are not instantiated in this firmware.
// ---------------------------------------------------------------------------
#ifndef MSBFIRST
#define MSBFIRST 0
#endif
#ifndef SPI_MODE3
#define SPI_MODE3 3
#endif

struct SPISettings {
    SPISettings(uint32_t /*clock*/, uint8_t /*bitOrder*/, uint8_t /*dataMode*/) {}
};

class SPIClass {
public:
    void begin() {}
    void end() {}
    void beginTransaction(SPISettings /*s*/) {}
    void endTransaction() {}
    uint8_t transfer(uint8_t /*data*/) { return 0; }
    uint16_t transfer16(uint16_t /*data*/) { return 0; }
};

// Global SPI instance expected by TMC library source files
extern SPIClass SPI;
