#include <Arduino.h>
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Movement Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  "Static\n" 1
//  "LCD Trigger\n"2
//  "LCD External Trigger\n"3
//  "Serial Cont.\n" 4            // 2 and 4 are the exact same just moce togther as soon as you ge the command
//  "Serial Cont. Ext.T.\n"; 5        // 3 and 5 are the same
const int Motor0LimitSw =PG6;
const int Motor1LimitSw =PG12;
const int Motor2LimitSw =PG9;
const int Motor3LimitSw =PG13;
const int Motor4LimitSw =PG10;
const int Motor5LimitSw =PG14;
const int Motor6LimitSw =PG11;
const int Motor7LimitSw =PG15;

void MOVE_FUNCTION(void)
{ // Selection =0
movevar[0]=0;
movevar[1]=0;
movevar[2]=0;
movevar[3]=0;
  Serial.println("I got to \"MOVE_FUNCTION()\".");
  if (Motion_selection != 2)
  { // If we arent in LCD Mode This avoids onyl 1 stepper moving
    Serial.println("Motion_selection != 2");
      // Parse Out The Data into the correct move variable
      movevar[0] = ABS_POS(Xpos, 0);   // X Move
      movevar[1] = ABS_POS(Ypos, 1);   // Y and Z Move  // Pull Data From LCD MENU VARIBLES
      movevar[2] = ABS_POS(AoA[0], 2); // E0 Move AoA Top
      movevar[3] = ABS_POS(AoA[1], 3); // E1 Move AoA Bottom
      gui_output_function();
  }
  // End parsing out data
  if (Motion_selection == 1 || Motion_selection == 4)
  { // if in static mode
    Serial.println("Motion_selection == 1 or 4");
    // Normal Mode LCD
    Xstepper.setupRelativeMoveInSteps (movevar[0]*6400);
    Serial.println("Steps to move");
    Serial.println(movevar[0]*6400);
    Ystepper.setupRelativeMoveInSteps(movevar[1]*6400);
    Zstepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[2] / Micro_stepping[2]));
    E0stepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2])); // Future: Make these in terms of degrees
    E1stepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3]));
    // Call A Blocking Function that Stops the Machine from doing anything else while the stepper is moving  This is desired since we aren not updating mid move.
    Serial.println("Entering while loop");
    while ((!E0stepper.motionComplete()) || (!E1stepper.motionComplete()) || (!Zstepper.motionComplete()) || (!Xstepper.motionComplete()) || (!Ystepper.motionComplete()))
    {
      Xstepper.processMovement();
      Ystepper.processMovement();
      Zstepper.processMovement();
      E0stepper.processMovement();
      E1stepper.processMovement();
    }
    Serial.println("Exited While Loop");
    if(Motion_selection == 1)
    {
      Abs_pos_error=false;
      MAIN_MENU();
    }
    else
    {
      Abs_pos_error=false;
      return;
    }
    //return;
  }
  if (Motion_selection == 2)
  {
    // LCD Trigger mode
    // Let the UI command issue move commands and just keep issuing this string untill the desired settings are made
    // We need to make sure that this doesnt log multiple moves

    Go_Pressed = 1; // There is new data to move to this disables the Trigger button to prevent multiple moves uless the entire sequence has been completeted
    return;
  }

  if (Motion_selection == 3 || Motion_selection == 5)
  {
    // LCD External Trigger mode and Serial External Trigger
    Xstepper.setupRelativeMoveInSteps(movevar[0]*6400); // Future: Make these iun terms of MM
    Serial.println("Steps to move");
    Serial.println(movevar[0]*X_mm_to_micro);
    Ystepper.setupRelativeMoveInSteps(movevar[1]*Y_mm_to_micro);
    Zstepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
    E0stepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2])); // Pre Process Stepper Moves
    E1stepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3]));
    //    E2stepper.setupRelativeMoveInSteps((movevar[*axis*] / Degree_per_step[4]) * Micro_stepping[4]);
    //      Serial.print("Waiting in Motion MODE 3: LCD Continous with Trigger"); // Debug Stuff
    //     Serial.print("Ready For Trigger");
    digitalWrite(27, HIGH);    // Sound Buzzer That The Control Borad is waiting on user
    TRIGGER_WAIT(TRIGGER_PIN); //call trigger wait function and pass in the trigger pin waity here till the trigger is hit
    digitalWrite(27, LOW);     // turn the anoying thing off
    while ((!E0stepper.motionComplete()) || (!E1stepper.motionComplete()) || (!Zstepper.motionComplete()) || (!Xstepper.motionComplete()) || (!Ystepper.motionComplete()))
    { // While they arent all complete
      Xstepper.processMovement();
      Ystepper.processMovement();
      Zstepper.processMovement();
      E0stepper.processMovement();
      E1stepper.processMovement();
    }
    Abs_pos_error=false;
    //return;
  }
} // End Function

void HomeAll(void)
{
  // Move all the axis 3 mm forward (Yes This lends itself to the potential of the axis moving beyond what is specified )
  // This ensures that all the axis are not allready on their limit swtiches
Xstepper.setupRelativeMoveInSteps (5*6400); // set up the move the axis 5 mm forward
Ystepper.setupRelativeMoveInSteps(5*6400);
Zstepper.setupRelativeMoveInSteps(5/ (Degree_per_step[2] / Micro_stepping[2]));
E0stepper.setupRelativeMoveInSteps(5/ (Degree_per_step[2] / Micro_stepping[2])); // Future: Make these in terms of degrees
E1stepper.setupRelativeMoveInSteps(5/ (Degree_per_step[3] / Micro_stepping[3]));
while ((!E0stepper.motionComplete()) || (!E1stepper.motionComplete()) || (!Zstepper.motionComplete()) || (!Xstepper.motionComplete()) || (!Ystepper.motionComplete()))
  { // While they arent all complete move them 
    Xstepper.processMovement();
    Ystepper.processMovement();
    Zstepper.processMovement();
    E0stepper.processMovement();
    E1stepper.processMovement();
  }
volatile bool xhome=false;
volatile bool yhome=false;
volatile bool aoathome=false;
volatile bool aoabhome=false;
// Refrencing the block diagram of the stm32f446 on page 16 of the pfd to understand the ports refrenced below
// a quick guide can be found here: https://gist.github.com/iwalpola/6c36c9573fd322a268ce890a118571ca#brr---bit-reset-register
/*
PinPrefix -> Port Name -
PA -> GPIO port A 
PB -> GPIO port B
PC -> GPIO port C
PD -> GPIO port D
PE -> GPIO port E 
*/
// This function must be rediculsuy fast thus the use of direct port maipulation 
// Set the direction of all the steppers:
GPIOA->ODR |= 0b0100000000000000; // set motor 7 (pa14) dir without affecting other pins 
GPIOC->ODR |= 0b0000000000000010; // set motor 3 (pc1)
GPIOE->ODR |= 0b0011000000001000; // set motor 6 (pe6)
GPIOF->ODR |= 0b0001010000000001; // set motor 0,4,5 (pf12)(pf10)(pf0)
GPIOG->ODR |= 0b0001010000001010; // set motor 1,3 (PG1)(PG3)
// Set inital states for motors
int motorgpioc=0b0010000000000000;// binary number for set motor 5 pc13 step pin HIGH leaving the rest alon
int motorgpioe=0b0000000001000100;// binary number for set motor 5 pc13 step pin HIGH leaving the rest alon
int motorgpiof=0b0010101000000000;// binary number for set motor 0,2,4 (pf13)(pf11)(pf9)
int motorgpiog=0b0010000000010001;// binary number for set motor 1,3 (pg0)(pg4)
// This code needs to run really fast thus it is written in binary and uses interrupts and binary math.
while(xhome==false || yhome==false || aoathome==false || aoabhome == false)
  { // While they arent hit the end stop we move the motors
  if(xhome==true){
    // The X axis is home
    motorgpiof=motorgpiof-0b0010000000000000; // remove the 13th digit
  }
  if(yhome==true){
    motorgpiog=motorgpiog-0b0000000000000001; // remove pg0
    motorgpiof=motorgpiof-0b0000100000000000; // remove pf11
  }
  if(aoathome==true){
    motorgpiog=motorgpiog-0b0000000000010000;
  }
  if(aoabhome==true){
    motorgpiof=motorgpiof-0b0000001000000000;
  }
  GPIOC->BSRR = motorgpioc<< 16; // set motor 5 pc13 step pin HIGH leaving the rest alone
  GPIOE->BSRR = motorgpioe<< 16; // set motor 5 pc13 step pin HIGH leaving the rest alon
  GPIOF->BSRR = motorgpiof<< 16; // set motor 0,2,4 (pf13)(pf11)(pf9)
  GPIOG->BSRR = motorgpiog<< 16; // set motor 1,3 (pg0)(pg4)
  delayMicroseconds(500);
  GPIOC->BSRR = motorgpioc; // set motor 5 pc13 step pin Low leaving the rest alone
  GPIOE->BSRR = motorgpioe; // set motor 6 and 7 (pe2)(pe6)
  GPIOF->BSRR = motorgpiof; // set motor 0,2,4 (pf13)(pf11)(pf9)
  GPIOG->BSRR = motorgpiog; // set motor 1,3 (pg0)(pg4)
  }
//
//   Xpos=0;
//   Ypos=0;
//   AoA[0]=0;
//   AoA[1]=0;
//   CurrentPositions[] = {0, 0, 0, 0, 0};
}
