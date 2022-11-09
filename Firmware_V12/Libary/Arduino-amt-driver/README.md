
# References
AMT21 Series encoder product page: https://www.cuidevices.com/product/motion/rotary-encoders/absolute/modular/amt21-series

AMT21 Series encoder datasheet: https://www.cuidevices.com/product/resource/amt21.pdf.

# Example (main.c)
The following code shows how to read the absolute position and the number of turns of the encoder.

    #include <project.h>
    #include "amt21_driver.h"
    
    int main(void)
    {
        uint16 absolute_position = 0;
        int16 number_of_turns = 0
        
        // Initializations
        CyGlobalIntEnable;  // Enable global interrupts
        amt21_init();       // Start and setup the AMT21 driver
        
        // Application
        for(;;) {
            // Get absolute position
            absolute_position = amt21_get_pos();
            
            // Get number of turns
            number_of_turns = amt21_get_turns();
        }
    }

# Setup

## UART component configuration
Configure tab:
* Mode = "Full UART (TX + RX)"
* Bits per second = see Datasheet page 5 (ignore if data rate is 2Mbps)
* Data bits = "8"
* Parity type = "None"
* Stop bits = "1"
* Flow control = "None"

Advanced tab:
* Clock selection = "External clock" if data rate is 2Mbsp, "Internal clock" otherwise
* RS-485 Configuration options = "Hardware TX-Enable" ticked
* Oversampling rate = "16x"

## Macros (see amt21_driver.h)
* AMT21_RESOLUTION = Resolution of your encoder as per its part number (see datasheet, page 5)
* AMT21_NODE_ADDR = RS485 node address of your encoder (see datasheet, page 3)
* AMT21_SINGLE_TURN = Uncomment if your encoder is single-turn (see datasheet, page 5)
