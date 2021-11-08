# WSU_AIRFOIL_PROJECT
Source code for the WSU Dual Airfoil Senior Design project.\
Problem Statement:\
Wright State graduate and undergraduate students would like to research\
the affects of dual airfoils on lift and pressure.\
The current set up for this system enables the user to set the angle\
of attack (AoA) on each of the airfoils as well as set gap and stagger between them.\
The gap and stagger motion is provided by electrical actuators moving on plastic \
linear berrings driven by a Texas Instruments DAQ. Programed in LabVIEW.\
The AoA was set manual via an arbitrary sharpy scale on a mental plate.

This presented three major issues:\
Year to year no one could actually program, or control this system.\
This system proved to be non-repeatable. (A fundemtal characteristic of an experiment)\
Friction / system stiffness of the linear berrings further reduced the repeatability.\
Using arbitrary scale made the system qualitative.

Key project Objectives:\
*Document all decisions, reasoning, choices, code and files for year to year use.\
*Enable repeatability with a resolution of .1mm gap and stagger and .05* AoA.\
*Use readily available parts to enable maintenance and keep the price down.

The function of this project is to drive 5 steppers in the desired modes.\
They are:\
LCD static- Motor move as soon as data is available from LCD\
LCD Trigger - Motors wait on a "go" button to be pressed in the LCD\
LCD W. Ext. T. - Motors wait on external physical trigger.\
Serial - motors move as soon as serial data is a available.\
Serial W. Ext. T. - Motors move on the press of an physical switch.


Key features:\
Custom G code implementation.\
Implemtiaton of Uart stepper control.\
Native 256 and interpolated 256 micro stepping capable.\
Selectable Spread Cycle or Silent Steptick enabled.\
Low cost use of a 3d printer motherboard.\
Written in Arduino for ease of use for non code centered users.\
PlatformIO configured with nessary libraries provided.\
(For year to year support of Senior design teams).\
(remember to change the path of the libraries in platform.ini)\
Full LCD implementation using SPI.\
Full System UI, with user input enabled through an encoder.\
Prosessor and IDE:\
ARM Stm32f446ZE Branch with Custom PlatformIO implantation.

Written by:\
Collin C. Charvat\
charvat.3@wright.edu\
c3charvat@gmail.com\
B.S. Mechanical Engineering, Wright State University. 22'\
College of Engineering and Computer Science.
