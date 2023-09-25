// Amt 21 arduino driver
/***********************************************************/

#ifndef _AMT21_DRIVER_H
#define _AMT21_DRIVER_H

#include <Arduino.h>

// Characteristics
// #define AMT21_RESOLUTION (12u)  // 12-bit or 14-bit (see datasheet, page 5)
#define AMT21_RESOLUTION (14u) // 12-bit or 14-bit (see datasheet, page 5)
// #define AMT21_SINGLE_TURN   // Uncomment if single-turn (see datasheet, page 5)
// Delays
#define AMT21_START_UP_TIME_MS ((uint8_t)200u)
#define AMT21_TURNAROUND_TIME_US ((uint8_t)30u)

/*******************************************************************************
 * PUBLIC PROTOTYPES
 *******************************************************************************/
class Amt21Encoder
{
public:
    enum Resolution
    {
        i14BIT = 14,
        i12BIT = 12,
    };
    enum NodeAddress
    {
        i54 = 54,
        i74 = 74,
        // maybe expand the list?
    };
    int m_turn_around_time;
    int m_rx_enable_pin;
    int m_tx_enable_pin;
    Resolution m_amt_resolution;
    NodeAddress m_amt_node_address;
    Stream &m_port_ptr;
    // Init
    Amt21Encoder(Stream &port, Resolution resolution, NodeAddress nodeaddress, int rx, int tx)
        :  m_port_ptr(port)
    {
        m_rx_enable_pin = rx;
        m_tx_enable_pin = tx;
        m_amt_resolution = resolution;
        m_amt_node_address = nodeaddress;

        delay(AMT21_START_UP_TIME_MS);

        while (port.available())
        {
            port.read(); // throw away anything thats in there
        }
    }

    // Read commands
    uint16_t amt_get_pos();
    int amt_get_turns();

    // Write commands
    void amt_reset_enc();

    #ifdef AMT_SINGLE_TURN
        void amt_set_zero_pos();
    #endif
private:
    void state_rs485_state(uint8_t);
    bool check_parity(uint16_t);
};

#endif // _AMT21_DRIVER_H
