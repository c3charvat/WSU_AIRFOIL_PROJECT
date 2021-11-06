// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Movement Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  "Static\n" 1
//  "LCD Trigger\n"2
//  "LCD External Trigger\n"3
//  "Serial Cont.\n" 4            // 2 and 4 are the exact same just moce togther as soon as you ge the command
//  "Serial Cont. Ext.T.\n"; 5        // 3 and 5 are the same

void MOVE_FUNCTION(void)
{ // Selection =0
  Serial.println("I got to \"MOVE_FUNCTION()\".");
  if (Motion_selection != 2)
  { // If we arent in LCD Mode This avoids onyl 1 stepper moving
    Serial.println("Motion_selection != 2");
    if (Com_selection == 2)
    { // If in LCD MODE
      Serial.println("Com_selection == 2");
      // Parse Out The Data into the correct move variable
      movevar[0] = ABS_POS(Xpos, 0);   // X Move
      movevar[1] = ABS_POS(Ypos, 1);   // Y and Z Move  // Pull Data From LCD MENU VARIBLES
      movevar[2] = ABS_POS(AoA[0], 2); // E0 Move AoA Top
      movevar[3] = ABS_POS(AoA[1], 3); // E1 Move AoA Bottom
    }
    else // Serial Mode
    {
      Serial.println("Com_selection != 2");
      movevar[0] = ABS_POS(Position_Data[0], 0); // X Pos 
      movevar[1] = ABS_POS(Position_Data[1], 1); // Y pos
      movevar[2] = ABS_POS(Position_Data[2], 2); // AoA Top  pos // We are in Serial Mode PULL in pharsed data
      movevar[3] = ABS_POS(Position_Data[3], 3); // AoA Bottom
    }
  }
  // End parsing out data
  if (Motion_selection == 1 || Motion_selection == 4)
  { // if in static mode
    Serial.println("Motion_selection == 1 or 4");
    // Normal Mode LCD
    Xstepper.setupRelativeMoveInSteps(movevar[0] / (Degree_per_step[0] / Micro_stepping[0])); // Future: Make these iun terms of MM
    Ystepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
    Zstepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
    E0stepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2])); // Future: Make these in terms of degrees
    E1stepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3]));
    // Call A Blocking Function that Stops the Machine from doing anything else while the stepper is moving
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
      MAIN_MENU();
    }
    else
    {
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
    //return;
  }

  if (Motion_selection == 3 || Motion_selection == 5)
  {
    // LCD External Trigger mode and Serial External Trigger
    Xstepper.setupRelativeMoveInSteps(movevar[0] / (Degree_per_step[0] / Micro_stepping[0]));
    Ystepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
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
    //return;
  }
} // End Function
