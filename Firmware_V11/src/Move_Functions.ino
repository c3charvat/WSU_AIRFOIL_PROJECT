#include <Arduino.h>
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Movement Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  "Static\n" 1
//  "LCD Trigger\n"2
//  "LCD External Trigger\n"3
//  "Serial Cont.\n" 4            // 2 and 4 are the exact same just moce togther as soon as you ge the command
//  "Serial Cont. Ext.T.\n"; 5        // 3 and 5 are the same
// const int Motor0LimitSw =PG6;
// const int Motor1LimitSw =PG12;
// const int Motor2LimitSw =PG9;
// const int Motor3LimitSw =PG13;
// const int Motor4LimitSw =PG10;
// const int Motor5LimitSw =PG14;
// const int Motor6LimitSw =PG11;
// const int Motor7LimitSw =PG15;

void MOVE_FUNCTION(void)
{ // Selection =0
  movevar[0] = 0; // clear out any previous data that may or maynot have been canceled 
  movevar[1] = 0;
  movevar[2] = 0;
  movevar[3] = 0;
  Serial.println("I got to \"MOVE_FUNCTION()\".");

  if (Com_selection != 2)
  { // If we arent in LCD Mode This avoids onyl 1 stepper moving
    Serial.println("Motion_selection != 2");
    // Parse Out The Data into the correct move variable
    movevar[0] = ABS_POS(Xpos, 0);   // X Move
    movevar[1] = ABS_POS(Ypos, 1);   // Y and Z Move  // Pull Data From LCD MENU VARIBLES
    movevar[2] = ABS_POS(AoA[0], 2); // E0 Move AoA Top
    movevar[3] = ABS_POS(AoA[1], 3); // E1 Move AoA Bottom
    gui_output_function();
  }

  if (Motion_selection == 2 && Com_selection == 2)
  { // If in Normal LCD Mode with motion in non trigger mode and  
    Serial.println("Motion_selection == 2 Com select == 2");
    // Normal Mode LCD
    Serial.println("Steps to move");
    Serial.println(movevar[0] * 6400);
    X_stepper.setupRelativeMoveInSteps(movevar[0] * 6400); // Future: Make these iun terms of MM
    Y0_stepper.setupRelativeMoveInSteps(movevar[1] * 6400);
    Y12_stepper.setupRelativeMoveInSteps(movevar[1] * 6400);
    Y3_stepper.setupRelativeMoveInSteps(movevar[1] * 6400);
    AOAT_stepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2]));
    AOAB_stepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3]));
    // Call A Blocking Function that Stops the Machine from doing anything else while the stepper is moving  This is desired since we aren not updating mid move.
    Serial.println("Entering while loop");
    while ((!X_stepper.motionComplete()) || (!Y0_stepper.motionComplete()) || (!Y12_stepper.motionComplete()) || (!Y3_stepper.motionComplete()) || (!AOAT_stepper.motionComplete()) || (!AOAB_stepper.motionComplete()))
    {
      X_stepper.processMovement();
      Y0_stepper.processMovement();
      Y12_stepper.processMovement(); // moving the Steppers here was a simple soltuion to having to do system interups and blah blah.
      Y3_stepper.processMovement();
      AOAT_stepper.processMovement();
      AOAB_stepper.processMovement();
    }
    Serial.println("Exited While Loop");

    if (Motion_selection == 1) // Think this is old error stuff leaving this in for now
    {
      Abs_pos_error = false;
      MAIN_MENU();
    }
    else
    {
      Abs_pos_error = false;
      return;
    }
    // return;
  }
  
  // if (Motion_selection == 2) // old lcd trigger mode 
  //   // LCD Trigger mode
  //   // Let the UI command issue move commands and just keep issuing this string untill the desired settings are made
  //   // We need to make sure that this doesnt log multiple moves

  //   Go_Pressed = 1; // There is new data to move to this disables the Trigger button to prevent multiple moves uless the entire sequence has been completeted
  //   return;
  // }

  if (Motion_selection == 1 && Com_selection == 2)
  {
    // LCD External Trigger mode and Serial External Trigger
    Serial.println("Steps to move");
    Serial.println(movevar[0] * X_mm_to_micro);
    X_stepper.setupRelativeMoveInSteps(movevar[0] * 6400); // Future: Make these iun terms of MM
    Y0_stepper.setupRelativeMoveInSteps(movevar[1] * 6400);
    Y12_stepper.setupRelativeMoveInSteps(movevar[1] * 6400);
    Y3_stepper.setupRelativeMoveInSteps(movevar[1] * 6400);
    AOAT_stepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2]));
    AOAB_stepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3]));
    //    E2stepper.setupRelativeMoveInSteps((movevar[*axis*] / Degree_per_step[4]) * Micro_stepping[4]);
    //      Serial.print("Waiting in Motion MODE 3: LCD Continous with Trigger"); // Debug Stuff
    //     Serial.print("Ready For Trigger");
    // digitalWrite(27, HIGH);    // Sound Buzzer That The Control Borad is waiting on user
    // delay(10); 
    // digitalWrite(27, LOW);  // Turn the buzzor onff
    while(Go_Pressed =! true){
      // do nothing here and wait for the interrupt 
      delayMicroseconds(1);
    }
    Go_Pressed = false;
    while ((!X_stepper.motionComplete()) || (!Y0_stepper.motionComplete()) || (!Y12_stepper.motionComplete()) || (!Y3_stepper.motionComplete()) || (!AOAT_stepper.motionComplete()) || (!AOAB_stepper.motionComplete()))
    {
      X_stepper.processMovement();
      Y0_stepper.processMovement();
      Y12_stepper.processMovement(); // moving the Steppers here was a simple soltuion to having to do system interups and blah blah.
      Y3_stepper.processMovement();
      AOAT_stepper.processMovement();
      AOAB_stepper.processMovement();
    }
    Abs_pos_error = false;
    // return;
  }
} // End Function

// home function
void HomeAll(void)
{
  // Move all the axis 3 mm forward (Yes This lends itself to the potential of the axis moving beyond what is specified )
  // This ensures that all the axis are not allready on their limit swtiches
  X_stepper.setupRelativeMoveInSteps(10 * 6400); // Future: Make these iun terms of MM
  Y0_stepper.setupRelativeMoveInSteps(10 * 6400);
  Y12_stepper.setupRelativeMoveInSteps(10 * 6400);
  Y3_stepper.setupRelativeMoveInSteps(10 * 6400);
  AOAT_stepper.setupRelativeMoveInSteps(10 / (Degree_per_step[2] / Micro_stepping[2]));
  AOAB_stepper.setupRelativeMoveInSteps(10/ (Degree_per_step[3] / Micro_stepping[3]));
  while ((!X_stepper.motionComplete()) || (!Y0_stepper.motionComplete()) || (!Y12_stepper.motionComplete()) || (!Y3_stepper.motionComplete()) || (!AOAT_stepper.motionComplete()) || (!AOAB_stepper.motionComplete()))
  {
    X_stepper.processMovement();
    Y0_stepper.processMovement();
    Y12_stepper.processMovement(); // moving the Steppers here was a simple soltuion to having to do system interups and blah blah.
    Y3_stepper.processMovement();
    AOAT_stepper.processMovement();
    AOAB_stepper.processMovement();
  }
  Serial.print("got through the firt part of home all");
  xhome = false; // we are now garenteed to be at least 5 off the axis
  yhome = false;
  aoathome = false;
  aoabhome = false;
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

  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
  LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_13);
  LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_0);
  LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_11);
  LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_4);
  LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_9); // reset pins to default state
  while (xhome == false || yhome == false || aoathome == false || aoabhome == false)
  { // While they arent hit the end stop we move the motors
    if (xhome == false)
    {
      // The X axis is home
      // motorgpiof=motorgpiof-0b0010000000000000; // remove the 13th digit
      LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_13);
    }
    if (yhome == false)
    {
      // motorgpiog=motorgpiog-0b0000000000000001; // remove pg0
      // motorgpiof=motorgpiof-0b0000100000000000; // remove pf11
      LL_GPIO_TogglePin(GPIOG, LL_GPIO_PIN_0);
      LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_11);
      LL_GPIO_TogglePin(GPIOG, LL_GPIO_PIN_4);
    }
    if (aoathome == false)
    {
      // motorgpiog=motorgpiog-0b0000000000010000;
      LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_9);
    }
    if (aoabhome == false)
    {
      // motorgpiof=motorgpiof-0b0000001000000000;
      LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
    }
    delayMicroseconds(2); // delay between high and low (Aka how long the pin is high)
    // reset pins to default state (Low), if it wastn triggered to high above it will remain at low
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
    LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_13);
    LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_0);
    LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_11);
    LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_4);
    LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_9);
    delayMicroseconds(70);        // delay between high states, how long between step signals
    //Serial.print("Hl");// debug to make sure it got here // kept short to minimize time 
  }
  //
  Xpos = 0;
  Ypos = 0;
  AoA[0] = 0;
  AoA[1] = 0;
  CurrentPositions[0] = 0;
  CurrentPositions[1] = 0;
  CurrentPositions[2] = 0;
  CurrentPositions[3] = 0;
  volatile bool xhome=false;
  volatile bool yhome=false;
  volatile bool aoathome=false; // second it leaves this function we assume its not home 
  volatile bool aoabhome=false;
}
