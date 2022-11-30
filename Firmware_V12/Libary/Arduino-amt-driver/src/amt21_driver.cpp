// Ported to ardunio 


#include <Arduino.h>
//#include <cstdint>
#include <bitset>
//#include <iostream>
#include "amt21_driver.hpp"
using namespace std;



// To do make these into a class that we
// Delays
#define AMT21_START_UP_TIME_MS ((uint8_t)200u)
#define AMT21_TURNAROUND_TIME_US ((uint8_t)30u)

// Commands
#define AMT21_RESPONSE_LENGTH ((uint8_t)3u) // number of bytes we got
#define AMT21_CMD_LENGTH ((uint8_t)1u)      // number of bytes to send
#define AMT21_EXT_CMD_LENGTH ((uint8_t)2u) // number of bytes to send
#define AMT21_RESET_ENC ((uint8_t)(0x75u))

#ifdef AMT21_SINGLE_TURN
#define AMT21_SET_ZERO_POS ((uint8_t)(0x5E))  // Single turn encoders only
#endif


bool check_parity(uint16_t *response);
unsigned int reverseBits(unsigned int n, unsigned int byte_size, unsigned int num_to_truncate);
int getSign(unsigned int n);


void amt21_init(Stream &port)
{    
    delay(AMT21_START_UP_TIME_MS);
    while(port.available()){
        port.read(); // throw away anything thats in there
    }
}

uint16_t amt_get_pos(Stream &port,int NODE_ADDR)
{
    // Send READ_POS command
    uint8_t cmd = ((uint8_t)(NODE_ADDR + 0x00u));
    port.write(&cmd, AMT21_CMD_LENGTH);

    // Wait for encoder to reply
    delayMicroseconds(AMT21_TURNAROUND_TIME_US);
    
    // Read response
    uint8_t response_raw[AMT21_RESPONSE_LENGTH];
    port.readBytes(response_raw, AMT21_RESPONSE_LENGTH);
    
    uint16_t pos = (response_raw[1] << 8u) | response_raw[0];
    
    // Check parity and extract value
    if(!check_parity(&pos))
    {
        return 0;
    }
    
    // Convert value to match encoder's resolution (see AMT21_RESOLUTION)
    if(AMT21_RESOLUTION == 12u)
    {
        pos = pos >> 2u;
    }
    // Return the position
    return pos;
}

int amt_get_turns(Stream &port,int NODE_ADDR)
{
    // Send READ_TURNS command
    uint8_t cmd = ((uint8_t)(NODE_ADDR + 0x01u));
    port.write(&cmd, AMT21_CMD_LENGTH);
    
    // Wait for encoder to reply
    delayMicroseconds(AMT21_TURNAROUND_TIME_US);
    
    // Read response
    uint8_t response_raw[AMT21_RESPONSE_LENGTH];
    port.readBytes(response_raw, AMT21_RESPONSE_LENGTH);
    
    uint16_t turns = (response_raw[2] << 8u) | response_raw[1]; // bit shift it ledt 

    // Check parity and extract value
    if(!check_parity(&turns))
    {
        return 0;
    }

    turns = (turns & 0x3FFFu);

    // Replace k0 and k1 with the MSB of the 14-bit int)
    bool negative = (turns >> 13) & 0x01u;
    if(negative)
    {
        turns = turns | 0xC000;
    }
    // Return the number of turns
    return (int)turns;
}

void amt_reset_enc(Stream &port,int NODE_ADDR)
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
* Parameters:
*   Stream &port -> The serial instace the device is connected to
*   NODE_ADDR    -> The node address of the rs485 device 
*
* Return:
*  None.
*
*******************************************************************************/
void amt_set_zero_pos(Stream &port,int NODE_ADDR)
{
    // Send AMT21_SET_ZERO_POS command
    uint8_t cmd[AMT21_EXT_CMD_LENGTH] = {((uint8_t)(NODE_ADDR + 0x02u)), AMT21_SET_ZERO_POS};
    port.write(cmd, AMT21_EXT_CMD_LENGTH);
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
    if(!response)
    {
        return false;
    }
    
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
    {
        return false;
    }
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
    {
        return false;
    }
    return true;
}


unsigned int reverseBits(unsigned int n, unsigned int byte_size, unsigned int num_to_truncate)
{
   unsigned int rev = 0;
   for(unsigned int i =0; i<num_to_truncate;i++)
   {
       n >>= 1; // skip over this one
   }
 
    // traversing bits of 'n' from the right
    for(unsigned int i = 0; i<(byte_size-num_to_truncate);i++)
    {
        // bitwise left shift
        // 'rev' by 1
        //std::cout << std::bitset<8>(n);
        //printf("\n");
        rev <<= 1;
 
        // if current bit is '1'
        if ((n & 1 )== 1)
            rev ^= 1;
        // bitwise right shift
        // 'n' by 1
        n >>= 1;
    }
        // required number
    //std::cout << std::bitset<8>(rev);
    //printf("\n");
    return rev;
}

int getSign(unsigned int n){
    for(unsigned int i =0; i<2;i++) // get past the parity bits
   {
       n >>= 1; // skip over this one
   }
   if ((n & 1 )== 1){ // if the current value is 1 then it is negitive
        return -1;
   }
   else{
    return 1;
   }
}



/*


#include <cstdint>
#include <bitset>
#include <iostream>
using namespace std;

#define bytes_to_u16(MSB,LSB) (((unsigned int) ((unsigned char) MSB)) & 255)<<8 | (((unsigned char) LSB)&255)

#include <stdio.h>


unsigned int reverseBits(unsigned int n, unsigned int byte_size, unsigned int num_to_truncate)
{
   unsigned int rev = 0;
   for(unsigned int i =0; i<num_to_truncate;i++)
   {
       n >>= 1; // skip over this one
   }
 
    // traversing bits of 'n' from the right
    for(unsigned int i = 0; i<(byte_size-num_to_truncate);i++)
    {
        // bitwise left shift
        // 'rev' by 1
        //std::cout << std::bitset<8>(n);
        //printf("\n");
        rev <<= 1;
 
        // if current bit is '1'
        if (n & 1 == 1)
            rev ^= 1;
        // bitwise right shift
        // 'n' by 1
        n >>= 1;
    }
 
    // required number
    //std::cout << std::bitset<8>(rev);
    //printf("\n");
    return rev;
}


int main()
{
  char buf[2];
  char bufa[2];
  bufa[0]=0b00100111;
  bufa[1]=0b10011111;

    unsigned int a = reverseBits(bufa[0],8,0);
    unsigned int b = reverseBits(bufa[1],8,2);
  std::cout << std::bitset<8>(a);
  printf("\n");
  std::cout << std::bitset<8>(b);

    buf[0]=0b11100100;
    buf[1]=0b111001;   // These are the desired forms




  // Fill buf with example numbers
  //buf[0]=2; // (Least significant byte)
  //buf[1]=2; // (Most significant byte)
  // If endian is other way around swap bytes!

  unsigned int port=bytes_to_u16(buf[1],buf[0]);

  printf("port = %u \n",port);

  uint16_t turns_unsigned =(((unsigned int) ((unsigned char) buf[1])&255 )<< 8u) | (((unsigned char) buf[0])&255);

    printf("%u \n", turns_unsigned);

    uint16_t turns_unsigned2 =(((unsigned int) ((unsigned char) b)&255 )<< 8u) | (((unsigned char) a)&255);

    printf("%u \n", turns_unsigned2);
    
    turns_unsigned = (turns_unsigned & 0x3FFFu);

    bool negative = (turns_unsigned >> 13) & 0x01u;
    
    if(negative) 
        turns_unsigned = turns_unsigned | 0xC000;

    printf("%u \n", turns_unsigned);

  

  return 0;
}


*/