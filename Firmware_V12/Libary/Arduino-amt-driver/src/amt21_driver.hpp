// Amt 21 arduino driver
/***********************************************************/

#ifndef _AMT21_DRIVER_H
#define _AMT21_DRIVER_H

#include <Arduino.h>

// Characteristics
// #define AMT21_RESOLUTION (12u)  // 12-bit or 14-bit (see datasheet, page 5)
#define AMT21_RESOLUTION (14u) // 12-bit or 14-bit (see datasheet, page 5)
// #define AMT21_SINGLE_TURN   // Uncomment if single-turn (see datasheet, page 5)

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
private:
    int m_turn_around_time;
    int m_rx_enable_pin;
    int m_tx_enable_pin;
    Resolution m_amt_resolution;
    NodeAddress m_amt_node_address;
    Stream &m_port_ptr;
public:
    // Init
    void amt_init(Stream &port, Resolution resolution, NodeAddress nodeaddress, int rx, int tx);

    // Read commands
    uint16_t amt_get_pos();
    int amt_get_turns();

    // Write commands
    void amt_reset_enc();

    #ifdef AMT_SINGLE_TURN
        void amt_set_zero_pos();
    #endif


};

#endif // _AMT21_DRIVER_H
