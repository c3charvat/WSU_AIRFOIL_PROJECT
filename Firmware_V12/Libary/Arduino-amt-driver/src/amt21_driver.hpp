// Amt 21 arduino driver
/***********************************************************/

#ifndef _AMT21_DRIVER_H
#define _AMT21_DRIVER_H

#include <Arduino.h>

// Characteristics
//#define AMT21_RESOLUTION (12u)  // 12-bit or 14-bit (see datasheet, page 5)
#define AMT21_RESOLUTION (14u)  // 12-bit or 14-bit (see datasheet, page 5)
//#define AMT21_SINGLE_TURN   // Uncomment if single-turn (see datasheet, page 5)


/*******************************************************************************
* PUBLIC PROTOTYPES
*******************************************************************************/
// Init
void amt_init(Stream &port);

// Read commands
uint16_t amt_get_pos(Stream &port,int NODE_ADDR);
int16_t amt_get_turns(Stream &port,int NODE_ADDR);

// Write commands
void amt_reset_enc(Stream &port,int NODE_ADDR);

#ifdef AMT_SINGLE_TURN
void amt_set_zero_pos(Stream &port,int NODE_ADDR);
#endif

#endif // _AMT21_DRIVER_H
