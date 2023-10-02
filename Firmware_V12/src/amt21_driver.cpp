// Ported to ardunio

#include <Arduino.h>
// #include <bitset>

#include "amt21_driver.hpp"
using namespace std;

// Commands
#define RS485_T_TX ((uint8_t)0u)            //transmit: receiver off, driver on
#define RS485_T_RX ((uint8_t)1u)           //receiver: driver off, transmit on
#define AMT21_RESPONSE_LENGTH ((uint8_t)3u) // number of bytes we got
#define AMT21_CMD_LENGTH ((uint8_t)1u)      // number of bytes to send
#define AMT21_EXT_CMD_LENGTH ((uint8_t)2u)  // number of bytes to send
#define AMT21_RESET ((uint8_t)0x75)
#define AMT21_ZERO ((uint8_t)0x5E)
#define AMT21_ENC0 ((uint8_t)0x54)
#define AMT21_ENC1 ((uint8_t)0x58)
#define AMT21_POS ((uint8_t)0x00) // this is unnecessary to use but it helps visualize the process for using the other modifiers
#define AMT21_TURNS ((uint8_t)0x01)
#define AMT21_EXT ((uint8_t)0x02)

#ifdef AMT21_SINGLE_TURN
#define AMT21_SET_ZERO_POS ((uint8_t)(0x5E)) // Single turn encoders only
#endif

// private defintions
bool check_parity(uint16_t *response);

uint16_t Amt21Encoder::amt_get_pos()
{
    // Send READ_POS command
    state_rs485_state(RS485_T_TX);
    delayMicroseconds(10);

    uint8_t cmd = ((uint8_t)(m_amt_node_address | 0x00));
    m_port_ptr.write(&cmd, AMT21_CMD_LENGTH);

    m_port_ptr.flush();// wait for the command to complete
    state_rs485_state(RS485_T_RX); // switch back to read

    // Wait for encoder to reply
    delayMicroseconds(AMT21_TURNAROUND_TIME_US);

    // Read response
    uint8_t bytes_received = m_port_ptr.available();
    uint8_t i=0;
    while(bytes_received<2 || i<10){ // wait for the complete message to come in
      delayMicroseconds(5);
      bytes_received = m_port_ptr.available();
      i=i+1;
      if (i==10){
        return 0; //Error
      }
    }
    uint8_t response_raw[AMT21_RESPONSE_LENGTH];
    m_port_ptr.readBytes(response_raw, AMT21_RESPONSE_LENGTH);

    uint16_t pos = response_raw[1] | (response_raw[2] << 8u); // bit shift it together

    // Check parity and extract value
    if (!check_parity(pos))
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
    state_rs485_state(RS485_T_TX);
    delayMicroseconds(10);

    uint8_t cmd = ((uint8_t)(m_amt_node_address | 0x01u));
    m_port_ptr.write(&cmd, AMT21_CMD_LENGTH);

    m_port_ptr.flush(); // wait for the command to complete
    state_rs485_state(RS485_T_RX); // switch back to read

    // Wait for encoder to reply
    delayMicroseconds(AMT21_TURNAROUND_TIME_US);

    // Read response
    uint8_t bytes_received = m_port_ptr.available();
    uint8_t i=0;
    while(bytes_received<2 || i<10){ // wait for the complete message to come in
      delayMicroseconds(5);
      bytes_received = m_port_ptr.available();
      i=i+1;
      if (i==10){
        return 0; //Error
      }
    };
    uint8_t response_raw[AMT21_RESPONSE_LENGTH];
    m_port_ptr.readBytes(response_raw, AMT21_RESPONSE_LENGTH);

    uint16_t turns = response_raw[1] | (response_raw[2] << 8u); // bit shift it together

    // Check parity and extract value
    if (!check_parity(turns))
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
    uint8_t cmd[AMT21_EXT_CMD_LENGTH] = {((uint8_t)(m_amt_node_address + 0x02u)), AMT21_RESET};
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

bool Amt21Encoder::check_parity(uint16_t message)
{
    // using the equation on the datasheet we can calculate the checksums and then make sure they match what the encoder sent
    // checksum is invert of XOR of bits, so start with 0b11, so things end up inverted
    uint16_t checksum = 0x3;
    for (int i = 0; i < 14; i += 2)
    {
        checksum ^= (message >> i) & 0x3;
    }
    return checksum == (message >> 14);
}

void Amt21Encoder::state_rs485_state(uint8_t state)
{
    // switch case to find the mode we want
    switch (state)
    {
    case RS485_T_TX:
        digitalWrite(m_rx_enable_pin, HIGH);
        digitalWrite(m_tx_enable_pin, HIGH);
        break;
    case RS485_T_RX:
        digitalWrite(m_rx_enable_pin, LOW);
        digitalWrite(m_tx_enable_pin, LOW);
        break;
    default:
        digitalWrite(m_rx_enable_pin, HIGH);
        digitalWrite(m_tx_enable_pin, LOW);
        break;
    }
}