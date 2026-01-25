/**
 * @file HalSerial.hpp
 * @brief HAL-based Serial UART class for STM32
 * 
 * Provides Arduino-like Serial interface using STM32 HAL
 */

#ifndef HAL_SERIAL_HPP
#define HAL_SERIAL_HPP

#include "stm32f4xx_hal.h"
#include <cstdint>
#include <cstddef>

/**
 * @brief Serial UART class using STM32 HAL
 * 
 * Example usage:
 *   HalSerial Serial3(USART3, GPIOD, GPIO_PIN_8, GPIO_PIN_9, GPIO_AF7_USART3);
 *   Serial3.begin(115200);
 *   Serial3.println("Hello World!");
 */
class HalSerial {
public:
    /**
     * @brief Construct a HalSerial instance
     * @param usart USART peripheral (e.g., USART1, USART2, USART3)
     * @param gpio_port GPIO port for TX/RX pins
     * @param tx_pin TX pin (e.g., GPIO_PIN_8)
     * @param rx_pin RX pin (e.g., GPIO_PIN_9)
     * @param alternate Alternate function (e.g., GPIO_AF7_USART3)
     */
    HalSerial(USART_TypeDef* usart, GPIO_TypeDef* gpio_port, 
              uint16_t tx_pin, uint16_t rx_pin, uint8_t alternate);

    /**
     * @brief Initialize the UART with specified baud rate
     * @param baudRate Baud rate (default 115200)
     */
    void begin(uint32_t baudRate = 115200);

    /**
     * @brief Deinitialize the UART
     */
    void end();

    /**
     * @brief Check if data is available to read
     * @return true if data available
     */
    bool available() const;

    /**
     * @brief Read a single byte (blocking)
     * @return The byte read
     */
    char read();

    /**
     * @brief Read a single byte (non-blocking)
     * @return The byte read, or -1 if no data available
     */
    int readNonBlocking();

    /**
     * @brief Read bytes into buffer
     * @param buffer Buffer to read into
     * @param length Maximum bytes to read
     * @param timeout Timeout in ms (default HAL_MAX_DELAY)
     * @return Number of bytes read
     */
    size_t readBytes(uint8_t* buffer, size_t length, uint32_t timeout = HAL_MAX_DELAY);

    /**
     * @brief Write a single byte
     * @param c Byte to write
     * @return 1 on success, 0 on failure
     */
    size_t write(uint8_t c);

    /**
     * @brief Write bytes from buffer
     * @param buffer Buffer to write from
     * @param length Number of bytes to write
     * @return Number of bytes written
     */
    size_t write(const uint8_t* buffer, size_t length);

    /**
     * @brief Print a string (no newline)
     * @param str String to print
     * @return Number of bytes written
     */
    size_t print(const char* str);

    /**
     * @brief Print a string with newline
     * @param str String to print
     * @return Number of bytes written
     */
    size_t println(const char* str = "");

    /**
     * @brief Print an integer
     * @param value Integer to print
     * @return Number of bytes written
     */
    size_t print(int32_t value);

    /**
     * @brief Print an integer with newline
     * @param value Integer to print
     * @return Number of bytes written
     */
    size_t println(int32_t value);

    /**
     * @brief Print an unsigned integer
     * @param value Unsigned integer to print
     * @return Number of bytes written
     */
    size_t print(uint32_t value);

    /**
     * @brief Print an unsigned integer with newline
     * @param value Unsigned integer to print
     * @return Number of bytes written
     */
    size_t println(uint32_t value);

    /**
     * @brief Print a floating point number
     * @param value Float to print
     * @param decimals Number of decimal places (default 2)
     * @return Number of bytes written
     */
    size_t print(double value, int decimals = 2);

    /**
     * @brief Print a floating point number with newline
     * @param value Float to print
     * @param decimals Number of decimal places (default 2)
     * @return Number of bytes written
     */
    size_t println(double value, int decimals = 2);

    /**
     * @brief Printf-style formatted output
     * @param format Format string
     * @param ... Arguments
     * @return Number of bytes written
     */
    size_t printf(const char* format, ...);

    /**
     * @brief Flush any pending transmissions (no-op for polling mode)
     */
    void flush();

    /**
     * @brief Check if serial is initialized
     * @return true if initialized
     */
    bool isInitialized() const;

    /**
     * @brief Get the underlying HAL UART handle (for advanced use)
     * @return Pointer to UART handle
     */
    UART_HandleTypeDef* getHandle();

private:
    USART_TypeDef* mUsart;
    GPIO_TypeDef* mGpioPort;
    uint16_t mTxPin;
    uint16_t mRxPin;
    uint8_t mAlternate;
    bool mInitialized;
    UART_HandleTypeDef mHandle;

    /**
     * @brief Enable peripheral clocks based on USART instance
     */
    void enableClocks();
};

#endif // HAL_SERIAL_HPP
