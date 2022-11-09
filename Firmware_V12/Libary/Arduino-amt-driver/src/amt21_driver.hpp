// Amt 21 driver
/***********************************************************/

#ifndef _AMT21_DRIVER_H
#define _AMT21_DRIVER_H


//#include <sys/param.h>
//#include <stdbool.h>
#include <Arduino.h>
//#include <HardwareSerial.h>
//#include <SoftwareSerial.h>

/*******************************************************************************
* MACROS
*******************************************************************************/
// Characteristics
//#define AMT21_RESOLUTION (12u)  // 12-bit or 14-bit (see datasheet, page 5)
#define AMT21_RESOLUTION (14u)  // 12-bit or 14-bit (see datasheet, page 5)
//#define AMT21_NODE_ADDR (0x54u) // Default is 0x54 moved to a paramter to support more than one node
//#define AMT21_SINGLE_TURN   // Uncomment if single-turn (see datasheet, page 5)


/*******************************************************************************
* PUBLIC PROTOTYPES
*******************************************************************************/
// Init
void amt21_init(Stream &port);

// Read commands
uint16_t amt21_get_pos(Stream &port,int NODE_ADDR);
int16_t amt21_get_turns(Stream &port,int NODE_ADDR);

// Write commands
void amt21_reset_enc(Stream &port,int NODE_ADDR);

#ifdef AMT21_SINGLE_TURN
void amt21_set_zero_pos(Stream &port,int NODE_ADDR);
#endif

#endif // _AMT21_DRIVER_H
/* [] END OF FILE */
