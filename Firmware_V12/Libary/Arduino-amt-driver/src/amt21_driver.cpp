/*******************************************************************************
*
* CUI AMT21XX-V Encoder driver for PSoC
* Copyright (C) 2020, Alexandre Bernier
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*******************************************************************************/
// Ported to ardunio 


#include <Arduino.h>
#include "amt21_driver.hpp"

// Delays
#define AMT21_START_UP_TIME_MS (200u)
#define AMT21_TURNAROUND_TIME_US (30u)

// Commands
#define AMT21_RESPONSE_LENGTH ((uint8_t)2u)
#define AMT21_CMD_LENGTH ((uint8_t)1u)
#define AMT21_EXT_CMD_LENGTH ((uint8_t)2u)
#define AMT21_READ_POS ((uint8_t)(AMT21_NODE_ADDR + 0x00u))
#define AMT21_READ_TURNS ((uint8_t)(AMT21_NODE_ADDR + 0x01u))
#define AMT21_EXT_CMD ((uint8_t)(AMT21_NODE_ADDR + 0x02u))
#define AMT21_RESET_ENC ((uint8_t)(0x75u))

#ifdef AMT21_SINGLE_TURN
#define AMT21_SET_ZERO_POS ((uint8_t)(0x5Eu))  // Single turn encoders only
#endif


/*******************************************************************************
* PRIVATE PROTOTYPES
*******************************************************************************/
//void _uart_read(uint8_t *data, uint8_t length);   // replaced by arduino built in
//void _uart_write(uint8_t *data, uint8_t length); // replaced by ardunio built in
bool _check_parity(uint16_t *response);


/*******************************************************************************
* PUBLIC FUNCTIONS
*******************************************************************************/
/*******************************************************************************
* Function Name: amt21_driver_init
********************************************************************************
* Summary:
*  Start UART component and wait until the encoder as started.
*  Should be called once before the infinite loop in your main.
*   
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void amt21_init(Stream &port)
{    
    delay(AMT21_START_UP_TIME_MS);
    while(port.available()){
        char t = port.read(); // throw away anything thats in there
    }
}

/*******************************************************************************
* Function Name: amt21_get_pos
********************************************************************************
* Summary:
*  Read the absolute position from the encoder.
*   
* Parameters:
*  None.
*
* Return:
*  uint16_t: Absolute position of the encoder.
*
*******************************************************************************/
uint16_t amt21_get_pos(Stream &port,int NODE_ADDR)
{
    // Send READ_POS command
    uint8_t cmd = ((uint8_t)(NODE_ADDR + 0x00u));
    //_uart_write(&cmd, AMT21_CMD_LENGTH);
    port.write(&cmd, AMT21_CMD_LENGTH);

    // Wait for encoder to reply
    delayMicroseconds(AMT21_TURNAROUND_TIME_US);
    
    // Read response
    uint8_t response_raw[AMT21_RESPONSE_LENGTH];
    port.readBytes(response_raw, AMT21_RESPONSE_LENGTH);
    
    uint16_t pos = (response_raw[1] << 8u) | response_raw[0];
    
    // Check parity and extract value
    if(!_check_parity(&pos))
        return 0;
    
    // Remove the parity check bits
    pos = (pos & 0x3FFFu);
    
    // Convert value to match encoder's resolution (see AMT21_RESOLUTION)
    if(AMT21_RESOLUTION == 12u)
        pos = pos >> 2u;

    // Return the position
    return pos;
}

/*******************************************************************************
* Function Name: amt21_get_turns
********************************************************************************
* Summary:
*  Read the turn counter's value from the encoder.
*   
* Parameters:
*  None.
*
* Return:
*  int16: Turn counter value of the encoder.
*
*******************************************************************************/
int16_t amt21_get_turns(Stream &port,int NODE_ADDR)
{
    // Send READ_TURNS command
    uint8_t cmd = ((uint8_t)(NODE_ADDR + 0x01u));
    port.write(&cmd, AMT21_CMD_LENGTH);
    
    // Wait for encoder to reply
    delayMicroseconds(AMT21_TURNAROUND_TIME_US);
    
    // Read response
    uint8_t response_raw[AMT21_RESPONSE_LENGTH];
    port.readBytes(response_raw, AMT21_RESPONSE_LENGTH);
    
    uint16_t turns_unsigned = (response_raw[1] << 8u) | response_raw[0];
    
    // Check parity and extract value
    if(!_check_parity(&turns_unsigned))
        return 0;
    
    // Remove the parity check bits (k0 and k1)
    turns_unsigned = (turns_unsigned & 0x3FFFu);
    
    // Replace k0 and k1 with the MSB of the 14-bit int)
    bool negative = (turns_unsigned >> 13) & 0x01u;
    if(negative) 
        turns_unsigned = turns_unsigned | 0xC000;

    // Return the number of turns
    return (int16_t)turns_unsigned;
}

/*******************************************************************************
* Function Name: amt21_reset_enc
********************************************************************************
* Summary:
*  Reset the encoder.
*   
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void amt21_reset_enc(Stream &port,int NODE_ADDR)
{
    // Send RESET_ENC command
    uint8_t cmd[AMT21_EXT_CMD_LENGTH] = {((uint8_t)(NODE_ADDR + 0x02u)), AMT21_RESET_ENC};
    port.write(cmd, AMT21_EXT_CMD_LENGTH);
    
    // Wait for encoder to reset
    delay(AMT21_START_UP_TIME_MS);
}

#ifdef AMT21_SINGLE_TURN
/*******************************************************************************
* Function Name: amt21_set_zero_pos
********************************************************************************
* Summary:
*  Set zero to the actual position.
*   
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void amt21_set_zero_pos(Stream &port,int NODE_ADDR)
{
    // Send AMT21_SET_ZERO_POS command
    uint8_t cmd[AMT21_EXT_CMD_LENGTH] = {((uint8_t)(NODE_ADDR + 0x02u)), AMT21_SET_ZERO_POS};
    port.write(cmd, AMT21_EXT_CMD_LENGTH);
}
#endif


/*******************************************************************************
* PRIVATE FUNCTIONS
*******************************************************************************/
/*******************************************************************************
* Function Name: _uart_read
********************************************************************************
* Summary:
*  Read all available bytes from UART.
*   
* Parameters:
*  [out] data: Array containing the read bytes.
*  [in] length: Number of bytes to read.
*
* Return:
*  uint8_t: Number of bytes read.
*
*******************************************************************************/
// void _uart_read(uint8_t *data, uint8_t length)
// {
//     // Verify parameters
//     if(!data || length <= 0)
//         return;
    
//     // Read bytes
//     for(int i=0; i<length; i++) {
//         data[i] = UART_AMT21_GetChar(); // i replaced this with the ardunio built in
//     }
// }

/*******************************************************************************
* Function Name: _uart_write
********************************************************************************
* Summary:
*  Write bytes to UART.
*   
* Parameters:
*  data: Array containing the bytes to write.
*  length: Number of bytes to write.
*
* Return:
*  None.
*
*******************************************************************************/
// void _uart_write(uint8_t *data, uint8_t length)
// {
//     // Verify parameters
//     if(!data || length <= 0)
//         return;
    
//     // Write bytes
//     UART_AMT21_PutArray(data, length); // replaced with the ardunio built in
// }

/*******************************************************************************
* Function Name: _check_parity
********************************************************************************
* Summary:
*  Check parity and return the value embedded in the response.
*
* Details:
*  Checksum
*  The AMT21 encoder uses a checksum calculation for detecting transmission
*  errors. The upper two bits of every response from the encoder are check bits.
*  Those values are shown in the examples below as K1 and K0. The check bits are
*  odd parity; K1 for the odd bits in the response, and K0 for the even bits in
*  the response. These check bits are not part of the position, but are used to
*  verify its validity. The remaining lower 14 bits are the useful data. Here is
*  an example of how to calculate the checkbits for a 16-bit response, from a
*  14-bit encoder.
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
*******************************************************************************/
bool _check_parity(uint16_t *response)
{
    // Verify parameters
    if(!response)
        return false;
    
    // Check odd bits parity (odd parity)
    bool k1 = (*response >> 15u) & 0x01u;
    bool odd_checkbit = !(((*response >> 1u) & 0x01u)  ^
                          ((*response >> 3u) & 0x01u)  ^
                          ((*response >> 5u) & 0x01u)  ^
                          ((*response >> 7u) & 0x01u)  ^
                          ((*response >> 9u) & 0x01u)  ^
                          ((*response >> 11u) & 0x01u) ^
                          ((*response >> 13u) & 0x01u));
    if(odd_checkbit != k1)
        return false;
    
    // Check even bits parity (odd parity)
    bool k0 = (*response >> 14u) & 0x01u;
    uint8_t even_checkbit = !(((*response >> 0u) & 0x01u)  ^
                            ((*response >> 2u) & 0x01u)  ^
                            ((*response >> 4u) & 0x01u)  ^
                            ((*response >> 6u) & 0x01u)  ^
                            ((*response >> 8u) & 0x01u)  ^
                            ((*response >> 10u) & 0x01u) ^
                            ((*response >> 12u) & 0x01u));
    if(even_checkbit != k0)
        return false;
        
    return true;
}

/* [] END OF FILE */
