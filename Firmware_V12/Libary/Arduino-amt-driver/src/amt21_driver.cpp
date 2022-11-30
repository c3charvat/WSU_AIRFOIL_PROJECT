// Ported to ardunio

#include <Arduino.h>
//#include <bitset>

#include "amt21_driver.hpp"
using namespace std;


// Commands
#define AMT21_RESPONSE_LENGTH ((uint8_t)3u) // number of bytes we got
#define AMT21_CMD_LENGTH ((uint8_t)1u)      // number of bytes to send
#define AMT21_EXT_CMD_LENGTH ((uint8_t)2u)  // number of bytes to send
#define AMT21_RESET_ENC ((uint8_t)(0x75u))

#ifdef AMT21_SINGLE_TURN
#define AMT21_SET_ZERO_POS ((uint8_t)(0x5E)) // Single turn encoders only
#endif

// private defintions 
bool check_parity(uint16_t *response);

uint16_t Amt21Encoder::amt_get_pos()
{
    // Send READ_POS command
    uint8_t cmd = ((uint8_t)( m_amt_node_address + 0x00u));
    m_port_ptr.write(&cmd, AMT21_CMD_LENGTH);

    // Wait for encoder to reply
    delayMicroseconds(AMT21_TURNAROUND_TIME_US);

    // Read response
    uint8_t response_raw[AMT21_RESPONSE_LENGTH];
    m_port_ptr.readBytes(response_raw, AMT21_RESPONSE_LENGTH);

    uint16_t pos = (response_raw[1] << 8u) | response_raw[0];

    // Check parity and extract value
    if (!check_parity(&pos))
    {
        return 0;
    }

    // Convert value to match encoder's resolution (see AMT21_RESOLUTION)
    if (AMT21_RESOLUTION == 12u)
    {
        pos = pos >> 2u;
    }
    // Return the position
    return pos;
}

int Amt21Encoder::amt_get_turns()
{
    // Send READ_TURNS command
    uint8_t cmd = ((uint8_t)( m_amt_node_address + 0x01u));
    m_port_ptr.write(&cmd, AMT21_CMD_LENGTH);

    // Wait for encoder to reply
    delayMicroseconds(AMT21_TURNAROUND_TIME_US);

    // Read response
    uint8_t response_raw[AMT21_RESPONSE_LENGTH];
    m_port_ptr.readBytes(response_raw, AMT21_RESPONSE_LENGTH);

    uint16_t turns = (response_raw[2] << 8u) | response_raw[1]; // bit shift it ledt

    // Check parity and extract value
    if (!check_parity(&turns))
    {
        return 0;
    }

    turns = (turns & 0x3FFFu);

    // Replace k0 and k1 with the MSB of the 14-bit int)
    bool negative = (turns >> 13) & 0x01u;
    if (negative)
    {
        turns = turns | 0xC000;
    }
    // Return the number of turns
    return (int)turns;
}

void Amt21Encoder::amt_reset_enc()
{
    // Send RESET_ENC command
    uint8_t cmd[AMT21_EXT_CMD_LENGTH] = {((uint8_t)(m_amt_node_address + 0x02u)), AMT21_RESET_ENC};
    m_port_ptr.write(cmd, AMT21_EXT_CMD_LENGTH);

    // Wait for encoder to reset
    delay(AMT21_START_UP_TIME_MS);
}

#ifdef AMT21_SINGLE_TURN

void Amt21Encoder::amt_set_zero_pos()
{
    // Send AMT21_SET_ZERO_POS command
    uint8_t cmd[AMT21_EXT_CMD_LENGTH] = {((uint8_t)(NODE_ADDR + 0x02u)), AMT21_SET_ZERO_POS};
    m_port_ptr.write(cmd, AMT21_EXT_CMD_LENGTH);
}
#endif

/*
 *
 *  Full response: 0x61AB
 *  14-bit position: 0x21AB (8619 decimal)
 *
 *  Checkbit Formula
 *  Odd: K1 = !(H5^H3^H1^L7^L5^L3^L1)
 *  Even: K0 = !(H4^H2^H0^L6^L4^L2^L0)
 *
 *  From the above response 0x61AB:
 *  Odd: 0 = !(1^0^0^1^1^1^1) = correct
 *  Even: 1 = !(0^0^1^0^0^0^1) = correct
 *
 * Parameters:
 *  response: Message received from the encoder.
 *
 * Return:
 *  bool: True if checksum is valid.
 *
 */

bool check_parity(uint16_t *response)
{
    // Verify parameters
    if (!response)
    {
        return false;
    }

    // Check odd bits parity (odd parity)
    bool k1 = (*response >> 15u) & 0x01u;
    bool odd_checkbit = !(((*response >> 1u) & 0x01u) ^
                          ((*response >> 3u) & 0x01u) ^
                          ((*response >> 5u) & 0x01u) ^
                          ((*response >> 7u) & 0x01u) ^
                          ((*response >> 9u) & 0x01u) ^
                          ((*response >> 11u) & 0x01u) ^
                          ((*response >> 13u) & 0x01u));
    if (odd_checkbit != k1)
    {
        return false;
    }
    // Check even bits parity (odd parity)
    bool k0 = (*response >> 14u) & 0x01u;
    uint8_t even_checkbit = !(((*response >> 0u) & 0x01u) ^
                              ((*response >> 2u) & 0x01u) ^
                              ((*response >> 4u) & 0x01u) ^
                              ((*response >> 6u) & 0x01u) ^
                              ((*response >> 8u) & 0x01u) ^
                              ((*response >> 10u) & 0x01u) ^
                              ((*response >> 12u) & 0x01u));
    if (even_checkbit != k0)
    {
        return false;
    }
    return true;
}
