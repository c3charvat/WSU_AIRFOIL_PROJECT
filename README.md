# WSU_AIRFOIL_PROJECT
Source code for the WSU Dual Airfoil Senior Design project.
The function of this project is to drive 5 steppers in the dsired mode.
They are:
LCD static- Motor move as soon as data is available from LCD
LCD Trigger - Motors wait on a "go" button to be pressed in the LCD
LCD W. Ext. T. - Motors wait on external physical trigger.
Serial - motors move as soon as serial data is a available 
Serial W. Ext. T. - Motors move on the press of an physical switch.
Key features:
Custom G code implementation.
Implemtiaton of Uart stepper control.
Native 256 and interpolated 256 micro stepping capable.
Spread Cycle or Silent Steptick enabled.
Low cost use of a 3d printer motherboard.
Written in Arduino for ease of use for non code centered users.
PlatformIO configured with nessary libraries installed 
(remember to change the path of the libraries in platform.ini)
Spread-cycle or Silent Steptick enabled.
Full LCD implementation using SPI.
Full System UI, with user input enabled through an encoder.
FUTURE:
ARM Stm32f446ZE Branch with Custom PlatformIO implantation.

