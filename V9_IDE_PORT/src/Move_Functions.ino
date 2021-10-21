// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Movement Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  "Static\n" 1
//  "LCD Trigger\n"2
//  "LCD External Trigger\n"3
//  "Serial Cont.\n" 4            // 2 and 4 are the exact same just moce togther as soon as you ge the command
//  "Serial Cont. Ext.T.\n"; 5        // 3 and 5 are the same

void MOVE_FUNCTION(void) { // Selection =0
  if (Com_selection == 2) { // If in LCD MODE
    // Parse Out The Data into the correct move variable
    movevar[0] = ABS_POS(AoA[0], 1);
    movevar[1] = ABS_POS(AoA[1], 2);
    movevar[2] = ABS_POS(Xpos, 3);
    movevar[3] = ABS_POS(Ypos, 4);
  }
  else {
    movevar[0] = ABS_POS(Position_Data[0], 1);
    movevar[1] = ABS_POS(Position_Data[1], 2);
    movevar[2] = ABS_POS(Position_Data[2], 3); // We are in Serial Mode PULL in pharsed data
    movevar[3] = ABS_POS(Position_Data[3], 4);
  }
  // End parsing out data
  if (Motion_selection == 1 || Motion_selection == 4 ) { // if in static mode
    // Normal Mode LCD
    Estepper.setupRelativeMoveInSteps(movevar[0] / (Degree_per_step[0] / Micro_stepping[0]));
    //Zstepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
   // Xstepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2])); // Change to mm // Just move the dam steppers.
    //Ystepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3])); // Change to mm
    //    E2stepper.setupRelativeMoveInSteps((movevar[*axis*] / Degree_per_step[4]) * Micro_stepping[4]);
    //      Serial.print("Waiting in Motion MODE 3: LCD Continous with Trigger"); // Debug Stuff
    //     Serial.print("Ready For Trigger");
    //digitalWrite(27, HIGH);// Sound Buzzer That The Control Borad is waiting on user
    //TRIGGER_WAIT(TRIGGER_PIN); //call trigger wait function and pass in the trigger pin waity here till the trigger is hit
    //digitalWrite(27, LOW);// turn the anoying thing off
  while ((!Estepper.motionComplete()) || (!Zstepper.motionComplete()) || (!Xstepper.motionComplete()) || (!Ystepper.motionComplete())) { // While they arent all complete
      Estepper.processMovement();
      //Zstepper.processMovement();
     //Xstepper.processMovement();
      //Ystepper.processMovement();
      //E2stepper.processMovement();
   }
    MAIN_MENU();
    //return; 
  }
  if (Motion_selection == 2) {
    // LCD Trigger mode
    Estepper.setupRelativeMoveInSteps(movevar[0] / (Degree_per_step[0] / Micro_stepping[0]));
    Zstepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
    Xstepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2])); // Change to mm // Just move the dam at once steppers.
    Ystepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3])); // Change to mm
    //    E2stepper.setupRelativeMoveInSteps((movevar[*axis*] / Degree_per_step[4]) * Micro_stepping[4]);
    // Let the UI command issue move commands to avoid having to do somew wierd wait that would freeze the system

    Go_Pressed = 1; // There is new data to move to this disables the Trigger button to prevent multiple moves uless the entire sequence has been completeted
    //return;
  }

  if (Motion_selection == 3 || Motion_selection == 5) {
    // LCD External Trigger mode and Serial External Trigger
    Estepper.setupRelativeMoveInSteps(movevar[0] / (Degree_per_step[0] / Micro_stepping[0]));
    Zstepper.setupRelativeMoveInSteps(movevar[1] / (Degree_per_step[1] / Micro_stepping[1]));
    Xstepper.setupRelativeMoveInSteps(movevar[2] / (Degree_per_step[2] / Micro_stepping[2])); // Change to mm // Just move the dam steppers.
    Ystepper.setupRelativeMoveInSteps(movevar[3] / (Degree_per_step[3] / Micro_stepping[3])); // Change to mm
    //    E2stepper.setupRelativeMoveInSteps((movevar[*axis*] / Degree_per_step[4]) * Micro_stepping[4]);
    //      Serial.print("Waiting in Motion MODE 3: LCD Continous with Trigger"); // Debug Stuff
    //     Serial.print("Ready For Trigger");
    digitalWrite(27, HIGH);// Sound Buzzer That The Control Borad is waiting on user
    TRIGGER_WAIT(TRIGGER_PIN); //call trigger wait function and pass in the trigger pin waity here till the trigger is hit
    digitalWrite(27, LOW);// turn the anoying thing off
    while ((!Estepper.motionComplete()) || (!Zstepper.motionComplete()) || (!Xstepper.motionComplete()) || (!Ystepper.motionComplete())) { // While they arent all complete
      Estepper.processMovement();
      Zstepper.processMovement();
      Xstepper.processMovement();
      Ystepper.processMovement();
      //E2stepper.processMovement();
    }
    //return;
  }
}  // End Function
