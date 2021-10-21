// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Absolute Positioing Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float ABS_POS( float input, int selection) { // input array is [Postion,Current Selection(Axis)->] 1: AoA Top 2: AoA Bottom 3: X Pos 4: Y Pos 
  float calcpos;
  if (selection == 1 || selection == 2 ) {
    // AoA Top & bottom
    //Serial.print("AoA Pos If statement"); Debug
    if (input > AOA_MAX) {
      Angle_Error(selection);// Display Error message
      return 0; //Dont Move
    }
    else {
      // If input is =
      if (input == CurrentPositions[selection - 1] || input == 360) {
        return 0;
      }
      // If input is graeater than
      if (input > CurrentPositions[selection - 1]) {
        calcpos = input - CurrentPositions[selection - 1]; // Ammount you have to move
        CurrentPositions[selection - 1] = CurrentPositions[selection - 1] + calcpos;
        return calcpos;
      }
      if (input < CurrentPositions[selection - 1]) {
        calcpos = input - CurrentPositions[selection - 1]; // needs to be negative to move backwards
        //if (calcpos > 0){
        CurrentPositions[selection - 1] = CurrentPositions[selection - 1] + calcpos;
        return calcpos;
      }
    }
  }
  if (selection == 3) {
    // X pos
    if (input > Xpos_MAX) {                                  // MAX X POS
      Angle_Error(3);// Display Error message
      return 0; //Dont Move
    }
    else {
      // If input is =
      if (input == CurrentPositions[selection - 1]) {
        return 0;
      }
      // If input is graeater than
      if (input > CurrentPositions[selection - 1]) {
        calcpos = input - CurrentPositions[selection - 1];
        CurrentPositions[selection - 1] = CurrentPositions[selection - 1] + calcpos;
        return calcpos;
      }
      // If input is less than
      if (input < CurrentPositions[selection - 1]) {
        calcpos = input - CurrentPositions[selection - 1]; // needs to be negative to move backwards
        CurrentPositions[selection - 1] = CurrentPositions[selection - 1] + calcpos;
        return calcpos;
      }
    }
  }
  if (selection == 4) {
    // Y pos
    if (input > Ypos_MAX) {                                  // MAX Y POS
      Angle_Error(4);// Display Error message
      return 0; //Dont Move
    }
    else {
      // If input is =
      if (input == CurrentPositions[selection - 1]) {
        return 0;
      }
      // If input is graeater than
      if (input > CurrentPositions[selection - 1]) {
        calcpos = input - CurrentPositions[selection - 1];
        CurrentPositions[selection - 1] = CurrentPositions[selection - 1] + calcpos;
        return calcpos;
      }
      // If input is less than
      if (input < CurrentPositions[selection - 1]) {
        calcpos = input - CurrentPositions[selection - 1]; // needs to be negative to move backwards
        CurrentPositions[selection - 1] = CurrentPositions[selection - 1] + calcpos;
        return calcpos;
      }
    }
  }
  else {
    // Return Error Message
    Somthing_Error();
    return 0; // since nothing was set just head back because the steppers wont move
  }
  return NULL;
}
