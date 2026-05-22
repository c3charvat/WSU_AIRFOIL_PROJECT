/**
 * @file HalGpio.hpp
 * @brief Minimal HAL utilities - only what HAL doesn't provide
 * 
 * For GPIO: Use HAL directly (HAL_GPIO_Init, HAL_GPIO_WritePin, HAL_GPIO_ReadPin)
 * For delays: Use HAL_Delay(ms) and delayMicroseconds(us) from this file
 * For interrupts: Use HAL_GPIO_Init with GPIO_MODE_IT_* and HAL_NVIC_EnableIRQ
 */

#ifndef HAL_GPIO_HPP
#define HAL_GPIO_HPP

#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_gpio.h"
#include <cstdint>
#include <string>

// ============================================================================
// String type alias for code compatibility
// ============================================================================
using String = std::string;

// ============================================================================
// DWT Initialization (call once at startup for microsecond timing)
// ============================================================================
inline void initDWT() {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// ============================================================================
// delayMicroseconds() - HAL doesn't provide this
// ============================================================================
inline void delayMicroseconds(uint32_t us) {
    uint32_t cycles = (SystemCoreClock / 1000000) * us;
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles) {}
}

// ============================================================================
// Convenience alias for HAL_Delay
// ============================================================================
inline void delay(uint32_t ms) {
    HAL_Delay(ms);
}

// ============================================================================
// micros() - microseconds since startup using DWT cycle counter
// initDWT() must be called once at startup before using this
// ============================================================================
inline uint32_t micros() {
    return DWT->CYCCNT / (SystemCoreClock / 1000000);
}

#endif // HAL_GPIO_HPP
