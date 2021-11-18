// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Absolute Positioing Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float ABS_POS(float input, int selection)
{ // input array is [Postion,Current Selection(Axis)->] 1: AoA Top 2: AoA Bottom 3: X Pos 4: Y Pos
/*
The Goal of this function is to take in a absolute postion entered by the user and convert it to a relative 
motion used by the end user. This was implmented because i wrote this with minimizing reliance on the stepper 
libaray in mind. This leaves room open in the future to implent flexy Stepper and change acceleration and speed 
on the fly in the middle of a move.
*/
  float calcpos;
  if (selection == 2 || selection == 3)
  {
    // AoA Top & bottom
    //Serial.print("AoA Pos If statement"); //Debug
    if (input > AOA_MAX)
    {
      Angle_Error(selection); // Display Error message
      return 0;               //Dont Move
    }
    else
    {
      // If input is =
      if (input == CurrentPositions[selection] || input == 360) // if we are at 360 or at the same postion we were before
      {
        return 0;
      }
      // If input is graeater than
      if (input > CurrentPositions[selection]) // if input is greater than where we are at
      {
        calcpos = input - CurrentPositions[selection]; // Ammount you have to move
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }
      if (input < CurrentPositions[selection]) // if input is less than where we are at
      {
        calcpos = input - CurrentPositions[selection]; // needs to be negative to move backwards
        //if (calcpos > 0){
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos; // set the new current position
        return calcpos;
      }
    }
  }
  if (selection == 0)
  {
    // X pos
    if (input > Xpos_MAX)
    {                 // MAX X POS
      Angle_Error(3); // Display Error message
      return 0;       //Dont Move
    }
    else
    {
      // If input is =
      if (input == CurrentPositions[selection])
      {
        return 0;
      }
      // If input is graeater than
      if (input > CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection];
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }
      // If input is less than
      if (input < CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection]; // needs to be negative to move backwards
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }
    }
  }
  if (selection == 1)
  {
    // Y pos
    if (input > Ypos_MAX)
    {                 // MAX Y POS
      Angle_Error(4); // Display Error message
      return 0;       //Dont Move
    }
    else
    {
      // If input is =
      if (input == CurrentPositions[selection])
      {
        return 0;
      }
      // If input is graeater than
      if (input > CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection];
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }
      // If input is less than
      if (input < CurrentPositions[selection])
      {
        calcpos = input - CurrentPositions[selection]; // needs to be negative to move backwards
        CurrentPositions[selection] = CurrentPositions[selection] + calcpos;
        return calcpos;
      }
    }
  }
  else
  {
    // Return Error Message
    Somthing_Error(); // If we get here we have fell through a function we were not suposed to fall through which is a major issue
    return 0; // since nothing was set just head back because the steppers wont move
  }
  return 0; // if it ever gets here this would be a big issue but i cant find a case that actually reaches here
}
