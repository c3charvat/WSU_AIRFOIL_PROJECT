#include <Arduino.h>

// ~~~~~~~~~Serial Read Functions~~~~~~~~~~~~~~~~~~~
void recvWithStartEndMarkers()
{
  static bool printedMsg = 0; // Debug
  if (printedMsg == 0) // Debug
  {
    Serial.println("Got to revrecvWithStartEndMarkers()\n");// Debug output only print once
    printedMsg = 1; // Debug
  }
  //Serial.println("Got to revrecvWithStartEndMarkers()\n");
  static bool recvInProgress = false; 
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;
  while (Serial.available() > 0 && newData == false)
  {
   // Serial.println("Got to while (Serial.available() > 0 && newData == false) in recvWithStartEndMarkers()"); // Debug output
    rc = Serial.read(); // Look at the next character 
    if (recvInProgress == true) // if we are recording 
    {
      if (rc != endMarker) // And we are not at the end marker
      {
        receivedChars[ndx] = rc; //Throw the current char into the array 
        ndx++; // increment index forward.
        if (ndx >= numChars) // If we exceed the max continue to read and just throw the data into the last postition
        {
          ndx = numChars - 1;
        }
      }
      else
      {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false; // stop recording 
        ndx = 0; // set index back to zero (fromaility not truly required)
        newData = true; // Let the program know that there is data wating for the parser.
      }
    }
    else if (rc == startMarker) // If Rc is the start marker we are getting good data
    {
      recvInProgress = true; // Start recording 
    }
  }
}

/*
The following is a custom G/M code implmentation
// We are pasing in Data like this:
// <G X###.## Y###.## AoAT###.## AoAB###.##>
// OR
// <M A X###.## Y###.## AoAT###.## AoAB###.##> or <M S X###.## Y###.## AoAT###.## AoAB###.##>
// Temp Charters (charter array) looks like:
// G X###.## Y###.## AoAT###.## AoAB###.##
// OR
// M A X###.## Y###.## AoAT###.## AoAB###.## 
// OR
// M S X###.## Y###.## AoAT###.## AoAB###.##
Any combination of capital/lowercase letters or single varibles are accepted and should be handled.
For Example:
// M S X#### OR M S Y ###### are acceptable inputs
// G X100 or G X100 Y 200 are acceptable too
For futher understanding, write out your own string and walk through the code below 
*/
bool parseData()
{ // split the data into its parts
  Serial.println("Got to parse data\n");
  char *strtokIndx; // this is used by strtok() as an index
  //char * LeadChar; // Getting the lead Charter of the command
  float Temp_Pos[5] = {Xpos, Ypos, AoA[0], AoA[1], NULL};
  int Setting_Num;      // Acelleration = 0 Speed = 1
  int Temp_Settings[5]; // only passed into the global varible if this function completes sucessfully

  //strtok(tempChars, " ");// get the first part - the string // This returns the first token "G"
  strtokIndx = strtok(tempChars, " "); // get the first part - the string // This returns the first token "G"
  Serial.println(strtokIndx[0]);
  if (strtokIndx[0] == 'R' || strtokIndx[0] == 'r')
  {
    // Simulated Estop AkA Just Kill power to the board
    digitalWrite(Reset,LOW); // Lmao This is one way to skin the cat rather than bothering with software
  }
  if (strtokIndx[0] == 'G' || strtokIndx[0] == 'g')
  {                                         // Begin G code parsing
    Serial.println("IT Starts With A G\n"); // G code Section
    // Movement parsing goes in here
    strtokIndx = strtok(NULL, " "); // process next string segment // This returns the next token "X##.##" then "Y##.##"...
    Serial.println(strtokIndx[0]);
    if (strtokIndx[0] == 'H' || strtokIndx[0] == 'h')
    { // if the first character is X
      Serial.println("IT has an h\n");
      strtokIndx = strtok(NULL, " ");
      Serial.println(strtokIndx[0]);
      if (strtokIndx == NULL)
      {
        // If there is nothing after H then Home all axis here
      }
      else
      {
        while (strtokIndx != NULL)
        {
          if (strtokIndx[0] == 'X' || strtokIndx[0] == 'x')
          { // if the first character is X
            Xstepper.moveToHomeInRevolutions(-1,20,20,PG6);
          }
          if (strtokIndx[0] == 'Y' || strtokIndx[0] == 'y')
          { // if the first character is Y
            // Home The Y Axis
            Ystepper.moveToHomeInRevolutions(-1,20,20,PG9);
            Zstepper.moveToHomeInRevolutions(-1,20,20,PG13);
          }
          if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a')
          { // if the first character is A -> Meaning AoA
            if (strtokIndx[3] == 'T' || strtokIndx[3] == 't')
            { // if the third character is T -> Meaning AoAT
              // Home AoA Top here
              E0stepper.moveToHomeInRevolutions(-1,20,20,PG10);
            }
            if (strtokIndx[3] == 'B' || strtokIndx[3] == 'b')
            { // if the third character is B -> Meaning AoAB
              // Home AoA Bottom here
              E1stepper.moveToHomeInRevolutions(-1,20,20,PG11);
            }
          }
        } // end while
      }   // End else
    }
    else if (isdigit(strtokIndx[0]))
    {
      Serial.print("G-code entered does not match the correct format please try again when prompted\n");
      strtokIndx = NULL; // exit the while loop
      return false;      // tell the system this command failed
    }
    else
    {
      // If it Gets here its in the Form <G X##.##>
      while (strtokIndx != NULL)
      {
        Serial.println(strtokIndx[0]);
        if (strtokIndx[0] == 'X' || strtokIndx[0] == 'x')
        { // if the first character is X
          Serial.println("IT has an X\n");
          char *substr = strtokIndx + 1; // This Truncates the first char "X"
          Temp_Pos[0] = atof(substr);    // Assign the value to the X position
        }
        if (strtokIndx[0] == 'Y' || strtokIndx[0] == 'y')
        {                                // if the first character is Y
          char *substr = strtokIndx + 1; // This Truncates the first char "X"
          Temp_Pos[1] = atof(substr);    // Assign the value to the Y position
        }
        if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a')
        { // if the first character is A -> Meaning AoA
          if (strtokIndx[3] == 'T' || strtokIndx[3] == 't')
          {                                // if the third character is T -> Meaning AoAT
            char *substr = strtokIndx + 4; // This Truncates the chacters "AoAT"
            Temp_Pos[2] = atof(substr);    // Assign the value to the AoAT position
          }
          if (strtokIndx[3] == 'B' || strtokIndx[3] == 'b')
          {                                // if the third character is B -> Meaning AoAB
            char *substr = strtokIndx + 4; // This Truncates the chacters "AoAB"
            Temp_Pos[3] = atof(substr);    // Assign the value to the AoAB position
          }
        }
        if (strtokIndx == NULL)
        {
          Serial.print("G-code entered does not match the correct format please try again when prompted\n");
          strtokIndx = NULL; // exit the while loop
          return false;      // tell the system this command failed
        }

        strtokIndx = strtok(NULL, " "); // process next string segment // This returns the next token "X##.##" then "Y##.##"...
        // if(strtokIndx[0] == NULL)
        // {
        //   Serial.println("strtokIndx[0] is NULL\n");
        // }
        // else
        // {
        //   Serial.println("Something else happened. I don't know what's going on.\n");
        // }
        // We are incrementing at the end of the loop so it stops before it could fall into the else statment when it reaches the end of the string
      }                             // End While loop
    }                    // If it makes it out of the while loop with out getting kicked out
    Xpos= Temp_Pos[0]; // set the global varibles
    Ypos = Temp_Pos[1];
    AoA[0]= Temp_Pos[2];
    AoA[1] = Temp_Pos[3];
    //Serial.println(Xpos);
    //Serial.println(Ypos);
    //Serial.println(AoA[0]); // Debug code here 
    //Serial.println(AoA[1]);
    //Serial.println("Heading to \"MOVE_FUNCTION()\".");
    MOVE_FUNCTION();
    return true; // Tell the system that the function worked
  }              // End G code parsing
                 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MCODE Section ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                 // M A X###.## Y###.## AoAT###.## AoAB###.## or M S X###.## Y###.## AoAT###.## AoAB###.##
  if (strtokIndx[0] == 'M' || strtokIndx[0] == 'm')
  { // If the First Char is M
    Serial.println(("strtokIndx[0] == M or m; strtokIndx[0] == " + std::string(strtokIndx)).c_str());
    strtokIndx = strtok(NULL, " "); // Get the second token "A or S"
    Serial.println(("Got next strtokIndx; strtokIndx[0] == " + std::string(strtokIndx)).c_str());
    if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a')
    {
      Serial.println("strtokIndx[0] == A or a");
      Setting_Num = 0; // i intoduced this varible because we will lose the A as this is a singly linked list (Forward)
    }
    else if (strtokIndx[0] == 'S' || strtokIndx[0] == 's')
    {
      Serial.println("strtokIndx[0] == S or a");
      Setting_Num = 1;
    }
    else if (strtokIndx[0] == 'D' || strtokIndx[0] == 'D')
    {
      Serial.println("strtokIndx[0] == D or d");
      strtokIndx = strtok(NULL, " "); // if it starts with a D get the next one
      Serial.println("strtokIndx = strtok() called.");
      if (strtokIndx[0] == 'M' || strtokIndx[0] == 'm')
      {
        Serial.println("strtokIndx[0] == M or m");
        Setting_Num = 2;
      }
      if (strtokIndx[0] == 'P' || strtokIndx[0] == 'p')
      {
        Serial.println("strtokIndx[0] == P or p");
        Setting_Num = 3;
      }
      if (strtokIndx[0] == 'S' || strtokIndx[0] == 's')
      {
        Serial.println("strtokIndx[0] == S or s");
        Setting_Num = 4;
      }
    }
    else
    {
      Serial.print("M-code entered does not match the correct format please try again when prompted");
      return false; // tell the system this command failed
    }
    strtokIndx = strtok(NULL, " "); // process next string segment // This returns the next token "X##.##" then "Y##.##"...
    Serial.println(("strtokIndx == " + std::string(strtokIndx)).c_str());
    if (isdigit(strtokIndx[0]))
    {
      Serial.println("Got to assigning numbers");
      Temp_Settings[0] = atof(strtokIndx); // Setting is for all Axis
      Temp_Settings[1] = atof(strtokIndx);
      Temp_Settings[2] = atof(strtokIndx);
      Temp_Settings[3] = atof(strtokIndx);
    }
    else
    {
      Serial.println("Not a digit; About to enter temp-settings assignment while loop.");
      while (strtokIndx != NULL)
      {
        Serial.println("Entered temp-settings assignment while loop.");
        if (strtokIndx[0] == 'X' || strtokIndx[0] == 'x')
        { // if the first character is X
          Serial.println("Assign X");
          char *substr = strtokIndx + 1;   // This Truncates the first char "X"
          Temp_Settings[0] = atof(substr); // Assign the value to the X position
        }
        if (strtokIndx[0] == 'Y' || strtokIndx[0] == 'y')
        { // if the first character is Y
          Serial.println("Assign Y.");
          char *substr = strtokIndx + 1;   // This Truncates the first char "Y"
          Temp_Settings[1] = atof(substr); // Assign the value to the Y position
        }
        if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a')
        { // if the first character is A -> Meaning AoA
          Serial.println("strtokIndx[0] == A or a");
          if (strtokIndx[3] == 'T' || strtokIndx[3] == 't')
          { // if the third character is T -> Meaning AoAT
            Serial.println("strtokIndx[0] == T or t");
            char *substr = strtokIndx + 4;   // This Truncates the chacters "AoAT"
            Temp_Settings[2] = atof(substr); // Assign the value to the AoAT position
          }
          if (strtokIndx[3] == 'B' || strtokIndx[3] == 'b')
          { // if the third character is B -> Meaning AoAB
            Serial.println("strtokIndx[0] == B or b");
            char *substr = strtokIndx + 4;   // This Truncates the chacters "AoAB"
            Temp_Settings[3] = atof(substr); // Assign the value to the AoAB position
          }
        }
        else
        {
          Serial.print("M-code entered does not match the correct format please try again when prompted\n");
          strtokIndx = NULL; // exit the while loop
          return false;      // tell the system this command failed
        }
        strtokIndx = strtok(NULL, " "); // process next string segment // This returns the next token "X##.##" then "Y##.##"...
      }                                 // End While loop
    }                                   // End Else Statment
    if (Setting_Num == 0)
    { // If Acceleration
      Serial.println("Set acceleration");
      Acell_Data[0] = Temp_Settings[0];
      Acell_Data[1] = Temp_Settings[1];
      Acell_Data[2] = Temp_Settings[2];
      Acell_Data[3] = Temp_Settings[3];
      SET_ACELL(Acell_Data[0], Acell_Data[1], Acell_Data[2], Acell_Data[3]);
      return true; // Tell the system that the function worked
    }
    if (Setting_Num == 1)
    { // Speed settings
      Serial.println("Set speed");
      Speed_Data[0] = Temp_Settings[0];
      Speed_Data[1] = Temp_Settings[1];
      Speed_Data[2] = Temp_Settings[2];
      Speed_Data[3] = Temp_Settings[3];
      SET_SPEED(Speed_Data[0], Speed_Data[1], Speed_Data[2], Speed_Data[3]);
      return true; // Tell the system that the function worked
    }
    if (Setting_Num == 2)
    { // Mirco stepping // 0,16,64,256
      Serial.println("Setting_Num == 2; Microstepping");
      if (Temp_Settings[0] != 0 || Temp_Settings[0] != 16 || Temp_Settings[0] != 64 || Temp_Settings[0] != 256 || Temp_Settings[1] != 0 || Temp_Settings[1] != 16 || Temp_Settings[1] != 64 || Temp_Settings[1] != 256 ||
          Temp_Settings[2] != 0 || Temp_Settings[2] != 16 || Temp_Settings[2] != 64 || Temp_Settings[2] != 256 || Temp_Settings[3] != 0 || Temp_Settings[3] != 16 || Temp_Settings[3] != 64 || Temp_Settings[3] != 256)
      {
        Serial.println("Microstepping not an acceptable input");
        return false; // The entered number was not one of the acctable inputs
      }
      else
      {
        Serial.println("Setting microstepping");
        Micro_stepping[0] = Temp_Settings[0];  //x
        Micro_stepping[1] = Temp_Settings[1];  //y
        Micro_stepping[2] = Micro_stepping[1]; //Z is tied to y
        Micro_stepping[3] = Temp_Settings[2];
        Micro_stepping[4] = Temp_Settings[3];
        driverX.microsteps(Micro_stepping[0]);
        driverY.microsteps(Micro_stepping[1]);
        driverZ.microsteps(Micro_stepping[2]);
        driverE0.microsteps(Micro_stepping[3]);
        driverE1.microsteps(Micro_stepping[4]);
        return true; // Tell the system that the function worked
      }
    }
    if (Setting_Num == 3)
    { // Stepper Mode
      Serial.println("Setting_Num == 3");
      if (Temp_Settings[0] != 0 || Temp_Settings[0] != 1)
      {
        return false; // The entered number was one of the acctable inputs
      }
      else
      {
        driverX.en_spreadCycle(Temp_Settings[0]);
        driverY.en_spreadCycle(Temp_Settings[1]);
        driverZ.en_spreadCycle(Temp_Settings[2]);
        driverE0.en_spreadCycle(Temp_Settings[3]);
        driverE1.en_spreadCycle(Temp_Settings[4]);
        return true; // Tell the system that the function worked
      }
    }
    if (Setting_Num == 4)
    { // Stall Gaurd
      if (Temp_Settings[0] != 0 || Temp_Settings[0] != 1)
      {
        return false; // The entered number was one of the acctable inputs
      }
      else
      {
        return true; // Tell the system that the function worked
      }
    }
  }
  else
  {
    Serial.println("Shouldn't Have made it here\n");
    return false;
  }
} // End Parsing Function

void showParsedData() // Debug Function
{ // show parsed data and move
  Serial.print("X Pos");
  Serial.println(Xpos);
  Serial.print("Y Pos"); // debug stuff here
  Serial.println(Ypos);
  Serial.print("\nAoA Top");
  Serial.println(AoA[0]);
  Serial.print("AoA Bottom ");
  Serial.println(AoA[1]);
  //Serial.print("AoA Bottom Speed");
  //Serial.println(Speed_Data[1]);
  //Serial.print("AoA Bottom Speed");
  //Serial.println(Acell_Data[1]);
  //MOVE_FUNCTION();
  // move function goes here
}
void serial_flush_buffer()
{
  while (Serial.read() >= 0)
    ; // do nothing
  Serial.print("Serial Flushed");
}