/**
 * @file HalSerial.cpp
 * @brief HAL-based Serial UART class implementation for STM32
 */

#include "HalSerial.hpp"
#include <cstdio>
#include <cstring>
#include <cstdarg>

HalSerial::HalSerial(USART_TypeDef* usart, GPIO_TypeDef* gpio_port, 
                     uint16_t tx_pin, uint16_t rx_pin, uint8_t alternate)
    : mUsart(usart)
    , mGpioPort(gpio_port)
    , mTxPin(tx_pin)
    , mRxPin(rx_pin)
    , mAlternate(alternate)
    , mInitialized(false)
{
    memset(&mHandle, 0, sizeof(mHandle));
}

void HalSerial::begin(uint32_t baudRate) {
    // Enable clocks
    enableClocks();
    
    // Configure GPIO pins
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = mTxPin | mRxPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = mAlternate;
    HAL_GPIO_Init(mGpioPort, &GPIO_InitStruct);
    
    // Configure UART
    mHandle.Instance = mUsart;
    mHandle.Init.BaudRate = baudRate;
    mHandle.Init.WordLength = UART_WORDLENGTH_8B;
    mHandle.Init.StopBits = UART_STOPBITS_1;
    mHandle.Init.Parity = UART_PARITY_NONE;
    mHandle.Init.Mode = UART_MODE_TX_RX;
    mHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    mHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&mHandle) == HAL_OK) {
        mInitialized = true;
    }
}

void HalSerial::end() {
    if (mInitialized) {
        HAL_UART_DeInit(&mHandle);
        mInitialized = false;
    }
}

bool HalSerial::available() const {
    if (!mInitialized) return false;
    return __HAL_UART_GET_FLAG(&mHandle, UART_FLAG_RXNE);
}

char HalSerial::read() {
    uint8_t c = 0;
    if (mInitialized) {
        HAL_UART_Receive(&mHandle, &c, 1, HAL_MAX_DELAY);
    }
    return (char)c;
}

int HalSerial::readNonBlocking() {
    if (!mInitialized || !available()) {
        return -1;
    }
    uint8_t c;
    if (HAL_UART_Receive(&mHandle, &c, 1, 0) == HAL_OK) {
        return (int)c;
    }
    return -1;
}

size_t HalSerial::readBytes(uint8_t* buffer, size_t length, uint32_t timeout) {
    if (!mInitialized || buffer == nullptr || length == 0) {
        return 0;
    }
    if (HAL_UART_Receive(&mHandle, buffer, length, timeout) == HAL_OK) {
        return length;
    }
    return 0;
}

size_t HalSerial::write(uint8_t c) {
    if (!mInitialized) return 0;
    if (HAL_UART_Transmit(&mHandle, &c, 1, HAL_MAX_DELAY) == HAL_OK) {
        return 1;
    }
    return 0;
}

size_t HalSerial::write(const uint8_t* buffer, size_t length) {
    if (!mInitialized || buffer == nullptr || length == 0) {
        return 0;
    }
    if (HAL_UART_Transmit(&mHandle, const_cast<uint8_t*>(buffer), length, HAL_MAX_DELAY) == HAL_OK) {
        return length;
    }
    return 0;
}

size_t HalSerial::print(const char* str) {
    if (!str) return 0;
    return write((const uint8_t*)str, strlen(str));
}

size_t HalSerial::println(const char* str) {
    size_t n = print(str);
    n += print("\r\n");
    return n;
}

size_t HalSerial::print(int32_t value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%ld", value);
    return print(buf);
}

size_t HalSerial::println(int32_t value) {
    size_t n = print(value);
    n += print("\r\n");
    return n;
}

size_t HalSerial::print(uint32_t value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", value);
    return print(buf);
}

size_t HalSerial::println(uint32_t value) {
    size_t n = print(value);
    n += print("\r\n");
    return n;
}

size_t HalSerial::print(double value, int decimals) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", decimals, value);
    return print(buf);
}

size_t HalSerial::println(double value, int decimals) {
    size_t n = print(value, decimals);
    n += print("\r\n");
    return n;
}

size_t HalSerial::printf(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    
    if (len > 0) {
        return write((const uint8_t*)buf, len);
    }
    return 0;
}

void HalSerial::flush() {
    // In polling mode, transmit is synchronous, nothing to flush
}

bool HalSerial::isInitialized() const {
    return mInitialized;
}

UART_HandleTypeDef* HalSerial::getHandle() {
    return &mHandle;
}

void HalSerial::enableClocks() {
    // Enable GPIO clock
    if (mGpioPort == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (mGpioPort == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (mGpioPort == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (mGpioPort == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (mGpioPort == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (mGpioPort == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (mGpioPort == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
    
    // Enable USART clock
    if (mUsart == USART1) __HAL_RCC_USART1_CLK_ENABLE();
    else if (mUsart == USART2) __HAL_RCC_USART2_CLK_ENABLE();
    else if (mUsart == USART3) __HAL_RCC_USART3_CLK_ENABLE();
    else if (mUsart == UART4) __HAL_RCC_UART4_CLK_ENABLE();
    else if (mUsart == UART5) __HAL_RCC_UART5_CLK_ENABLE();
    else if (mUsart == USART6) __HAL_RCC_USART6_CLK_ENABLE();
}
