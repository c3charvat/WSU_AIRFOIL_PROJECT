#include <Arduino.h>

// ~~~~~~~~~Serial Read Functions~~~~~~~~~~~~~~~~~~~
void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

// Flow Chart
// Read the entire string
// We are pasing in Data like this:
// <G X###.## Y###.## AoAT###.## AoAB###.##>
// OR
// <M A X###.## Y###.## AoAT###.## AoAB###.##> or <M S X###.## Y###.## AoAT###.## AoAB###.##>
// Temp Charters (charter array) looks like:
// G X###.## Y###.## AoAT###.## AoAB###.##
// OR
// M A X###.## Y###.## AoAT###.## AoAB###.## or M S X###.## Y###.## AoAT###.## AoAB###.##
bool parseData() {      // split the data into its parts
  char * strtokIndx; // this is used by strtok() as an index
  //char * LeadChar; // Getting the lead Charter of the command
  float Temp_Pos[5];
  int Setting_Num; // Acelleration = 0 Speed = 1
  int Temp_Settings[5]; // only passed into the global varible if this function completes sucessfully

  strtokIndx = strtok(tempChars, " ");// get the first part - the string // This returns the first token "G"
  if (strtokIndx[0] == 'G' || strtokIndx[0] == 'g') { // Begin G code parsing                                                           // G code Section
    // Movement parsing goes in here
    strtokIndx = strtok(NULL, " "); // process next string segment // This returns the next token "X##.##" then "Y##.##"...
    if (strtokIndx[0] == 'H' || strtokIndx[0] == 'h') { // if the first character is X
      strtokIndx = strtok(NULL, " ");
      if (strtokIndx == NULL) {
        // If there is nothing after H then Home all axis here
      }
      else {
        while (strtokIndx != NULL) {
          if (strtokIndx[0] == 'X' || strtokIndx[0] == 'x') { // if the first character is X
            // Home X axis here
          }
          if (strtokIndx[0] == 'Y' || strtokIndx[0] == 'y') { // if the first character is Y
            // Home The Y Axis
          }
          if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a') { // if the first character is A -> Meaning AoA
            if (strtokIndx[3] == 'T' || strtokIndx[0] == 't') { // if the third character is T -> Meaning AoAT
              // Home AoA Top here
            }
            if (strtokIndx[3] == 'B' || strtokIndx[0] == 'b') { // if the third character is B -> Meaning AoAB
              // Home AoA Bottom here
            }
          }
        } // end while
      } // End else
    }
      else {
        while (strtokIndx != NULL) {
          if (strtokIndx[0] == 'X' || strtokIndx[0] == 'x') { // if the first character is X
            char* substr = strtokIndx + 1; // This Truncates the first char "X"
            Temp_Pos[0] = atof(substr); // Assign the value to the X position
          }
          if (strtokIndx[0] == 'Y' || strtokIndx[0] == 'y') { // if the first character is Y
            char* substr = strtokIndx + 1; // This Truncates the first char "X"
            Temp_Pos[1] = atof(substr); // Assign the value to the Y position
          }
          if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a') { // if the first character is A -> Meaning AoA
            if (strtokIndx[3] == 'T' || strtokIndx[0] == 't') { // if the third character is T -> Meaning AoAT
              char* substr = strtokIndx + 4; // This Truncates the chacters "AoAT"
              Temp_Pos[2] = atof(substr); // Assign the value to the AoAT position
            }
            if (strtokIndx[3] == 'B' || strtokIndx[0] == 'b') { // if the third character is B -> Meaning AoAB
              char* substr = strtokIndx + 4;// This Truncates the chacters "AoAB"
              Temp_Pos[3] = atof(substr); // Assign the value to the AoAB position
            }
          }
          else {
            Serial.print("G-code entered does not match the correct format please try again when prompted");
            strtokIndx = NULL; // exit the while loop
            return false; // tell the system this command failed
          }
          strtokIndx = strtok(NULL, " "); // process next string segment // This returns the next token "X##.##" then "Y##.##"...
          // We are incrmenting at the end of the loop so it stops before it could fall into the else statment when it reaches the end of the string
        }// End While loop
      }// If it makes it out of the while loop with out getting kicked out
      Position_Data[0] = Temp_Pos[0]; // set the global varibles
      Position_Data[1] = Temp_Pos[1];
      Position_Data[2] = Temp_Pos[2];
      Position_Data[3] = Temp_Pos[3];
      MOVE_FUNCTION();
      return true; // Tell the system that the function worked
    } // End G code parsing
    if (strtokIndx[0] == 'M' || strtokIndx[0] == 'm') { // If the First Char is M                                                                   // M code Starts here
      strtokIndx = strtok(NULL, " "); // Get the second token "A or S"
      if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a') {
        Setting_Num = 0; // i intoduced this varible because we will lose the A as this is a singly linked list (Forward)
      }
      if (strtokIndx[0] == 'S' || strtokIndx[0] == 'S') {
        Setting_Num = 1;
      }
      if (strtokIndx[0] == 'D' || strtokIndx[0] == 'D') {
        strtokIndx = strtok(NULL, " "); // if it starts with a D get the next one
        if (strtokIndx[0] == 'M' || strtokIndx[0] == 'm') {
          Setting_Num = 2;
        }
        if (strtokIndx[0] == 'P' || strtokIndx[0] == 'p') {
          Setting_Num = 3;
        }
        if (strtokIndx[0] == 'S' || strtokIndx[0] == 's') {
          Setting_Num = 4;
        }
      }
      else {
        Serial.print("M-code entered does not match the correct format please try again when prompted");
        return false; // tell the system this command failed
      }
      strtokIndx = strtok(NULL, " "); // process next string segment // This returns the next token "X##.##" then "Y##.##"...
      if (isdigit(strtokIndx[0])) {
      Temp_Settings[0] = atof(strtokIndx); // Setting is for all Axis
        Temp_Settings[1] = atof(strtokIndx);
        Temp_Settings[2] = atof(strtokIndx);
        Temp_Settings[3] = atof(strtokIndx);
      }
      else {
        while (strtokIndx != NULL) {
          if (strtokIndx[0] == 'X' || strtokIndx[0] == 'x') { // if the first character is X
            char* substr = strtokIndx + 1; // This Truncates the first char "X"
            Temp_Settings[0] = atof(substr); // Assign the value to the X position
          }
          if (strtokIndx[0] == 'Y' || strtokIndx[0] == 'y') { // if the first character is Y
            char* substr = strtokIndx + 1;// This Truncates the first char "Y"
            Temp_Settings[1] = atof(substr); // Assign the value to the Y position
          }
          if (strtokIndx[0] == 'A' || strtokIndx[0] == 'a') { // if the first character is A -> Meaning AoA
            if (strtokIndx[3] == 'T' || strtokIndx[0] == 't') { // if the third character is T -> Meaning AoAT
              char* substr = strtokIndx + 4; // This Truncates the chacters "AoAT"
              Temp_Settings[2] = atof(substr); // Assign the value to the AoAT position
            }
            if (strtokIndx[3] == 'B' || strtokIndx[0] == 'b') { // if the third character is B -> Meaning AoAB
              char* substr = strtokIndx + 4; // This Truncates the chacters "AoAB"
              Temp_Settings[3] = atof(substr); // Assign the value to the AoAB position
            }
          }
          else {
            Serial.print("M-code entered does not match the correct format please try again when prompted");
            strtokIndx = NULL; // exit the while loop
            return false; // tell the system this command failed
          }
          strtokIndx = strtok(NULL, " "); // process next string segment // This returns the next token "X##.##" then "Y##.##"...
        }// End While loop
      }// End Else Statment
      if (Setting_Num == 0) { // If Acceleration
      Acell_Data[0] = Temp_Settings[0];
        Acell_Data[1] = Temp_Settings[1];
        Acell_Data[2] = Temp_Settings[2];
        Acell_Data[3] = Temp_Settings[3];
        SET_ACELL(Acell_Data[0], Acell_Data[1], Acell_Data[2], Acell_Data[3]);
        return true; // Tell the system that the function worked
      }
      if (Setting_Num == 1) { // Speed settings
      Speed_Data[0] = Temp_Settings[0];
        Speed_Data[1] = Temp_Settings[1];
        Speed_Data[2] = Temp_Settings[2];
        Speed_Data[3] = Temp_Settings[3];
        SET_SPEED(Acell_Data[0], Acell_Data[1], Acell_Data[2], Acell_Data[3]);
        return true; // Tell the system that the function worked
      }
      if (Setting_Num == 2) { // Mirco stepping // 0,16,64,256
      if ( Temp_Settings[0] != 0 || Temp_Settings[0] != 16 || Temp_Settings[0] != 64 || Temp_Settings[0] != 256 || Temp_Settings[1] != 0 || Temp_Settings[1] != 16 || Temp_Settings[1] != 64 || Temp_Settings[1] != 256 ||
             Temp_Settings[2] != 0 || Temp_Settings[2] != 16 || Temp_Settings[2] != 64 || Temp_Settings[2] != 256 || Temp_Settings[3] != 0 || Temp_Settings[3] != 16 || Temp_Settings[3] != 64 || Temp_Settings[3] != 256) {
          return false; // The entered number was one of the acctable inputs
        }
        else {
          Micro_stepping[0] = Temp_Settings[0]; //x
          Micro_stepping[1] = Temp_Settings[1];//y
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
      if (Setting_Num == 3) { // Stepper Mode
      if ( Temp_Settings[0] != 0 || Temp_Settings[0] != 1) {
          return false; // The entered number was one of the acctable inputs
        }
        else {
          driverX.en_spreadCycle(Temp_Settings[0]);
          driverY.en_spreadCycle(Temp_Settings[1]);
          driverZ.en_spreadCycle(Temp_Settings[2]);
          driverE0.en_spreadCycle(Temp_Settings[3]);
          driverE1.en_spreadCycle(Temp_Settings[4]);
          return true; // Tell the system that the function worked
        }
      }
      if (Setting_Num == 4) { // Stall Gaurd
      if ( Temp_Settings[0] != 0 || Temp_Settings[0] != 1) {
          return false; // The entered number was one of the acctable inputs
        }
        else {
          return true; // Tell the system that the function worked
        }
      }
    }
} // End Parsing Function

void showParsedData() { // show parsed data and move
  Serial.print("\nAoA Top");
  Serial.println(Position_Data[0]);
  Serial.print("AoA Bottom ");
  Serial.println(Position_Data[1]);
  Serial.print("X Pos");
  Serial.println(Position_Data[2]);
  Serial.print("Y Pos"); // debug stuff here
  Serial.println(Position_Data[3]);
  //Serial.print("AoA Bottom Speed");
  //Serial.println(Speed_Data[1]);
  //Serial.print("AoA Bottom Speed");
  //Serial.println(Acell_Data[1]);
  //MOVE_FUNCTION();
  // move function goes here
}
void serial_flush_buffer() {
  while (Serial.read() >= 0); // do nothing
  Serial.print("Serial Flushed");
}