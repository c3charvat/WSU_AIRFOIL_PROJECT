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
        i12BIT = 12
    };
    enum NodeAddress
    {
        i54 = 54,
        i74 = 74
        // maybe expand the list?
    };
    int rx_pin;
    int tx_enable_pin;
    Resolution amt_resolution;
    NodeAddress amt_node_address;
    Stream * port;

    // Init
    void amt_init(Amt21Encoder::Resolution,Amt21Encoder::NodeAddress);
        // make setters here 

};


// Read commands
uint16_t amt_get_pos(Stream &port, int NODE_ADDR);
int amt_get_turns(Stream &port, int NODE_ADDR);

// Write commands
void amt_reset_enc(Stream &port, int NODE_ADDR);

#ifdef AMT_SINGLE_TURN
void amt_set_zero_pos(Stream &port, int NODE_ADDR);
#endif

#endif // _AMT21_DRIVER_H
