/**
 * @file HalStream.hpp
 * @brief Minimal Arduino-compatible Stream abstract class for TMCStepper.
 *
 * TMCStepper's Stream* constructor stores this pointer and calls
 * available() / read() / write() for UART register access.
 *
 * TODO: Replace NullStream with a real HalUartStream once the TMC UART
 *       peripheral and pin assignments are known for this board.
 *       Until then the drivers run in standalone mode (step/dir only).
 */

#ifndef HAL_STREAM_HPP
#define HAL_STREAM_HPP

#include <stdint.h>
#include <stddef.h>

// ---------------------------------------------------------------------------
// Abstract Stream base – satisfies TMCStepper's Stream* constructor
// ---------------------------------------------------------------------------
class Stream {
public:
    virtual int    available()         = 0;
    virtual int    read()              = 0;
    virtual size_t write(uint8_t byte) = 0;
    virtual ~Stream() = default;
};

// ---------------------------------------------------------------------------
// NullStream – no-op implementation; TMC drivers work in standalone mode.
// ---------------------------------------------------------------------------
class NullStream : public Stream {
public:
    int    available()         override { return 0;  }
    int    read()              override { return -1; }
    size_t write(uint8_t /*b*/) override { return 0;  }
};

#endif // HAL_STREAM_HPP
