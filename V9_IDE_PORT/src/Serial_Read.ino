// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Serial Read Functions~~~~~~~~~~~~~~~~~~~
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

void parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the first part - the string
  Position_Data[0] = atof(strtokIndx); // copy it to messageFromPC
  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  Position_Data[1] = atof(strtokIndx);     // convert this part to an integer
  strtokIndx = strtok(NULL, ",");
  Position_Data[2] = atof(strtokIndx);     // convert this part to a float
  strtokIndx = strtok(NULL, ",");
  Position_Data[3] = atof(strtokIndx);    // convert this part to a float
  // strtokIndx = strtok(NULL, ",");
  // Position_Data[4] = atof(strtokIndx); // modified for new motherboard

  strtokIndx = strtok(NULL, ",");
  Speed_Data[0] = atoi(strtokIndx);    // convert this part to a float  // data for E stepper
  strtokIndx = strtok(NULL, ",");
  Speed_Data[1] = atoi(strtokIndx);    // convert this part to a float  // data for Z stepper
  strtokIndx = strtok(NULL, ",");
  Speed_Data[2] = atoi(strtokIndx);    // convert this part to a float  // data for X stepper
  strtokIndx = strtok(NULL, ",");
  Speed_Data[3] = atoi(strtokIndx);    // convert this part to a float  // data for Y stepper
  // strtokIndx = strtok(NULL, ",");
  // Speed_Data[4] = atoi(strtokIndx); // modified for new motherboard

  strtokIndx = strtok(NULL, ",");
  Acell_Data[0] = atof(strtokIndx);    // convert this part to a float
  strtokIndx = strtok(NULL, ",");
  Acell_Data[1] = atof(strtokIndx);    // convert this part to a float
  strtokIndx = strtok(NULL, ",");
  Acell_Data[2] = atof(strtokIndx);    // convert this part to a float
  strtokIndx = strtok(NULL, ",");
  Acell_Data[3] = atof(strtokIndx);    // convert this part to a float
  // strtokIndx = strtok(NULL, ",");
  // Acell_Data[4] = atof(strtokIndx); // modified for new motherboard
}

void showParsedData() { // show parsed data and move
  SET_ACELL(Acell_Data[0], Acell_Data[1], Acell_Data[2], Acell_Data[3]); // set the new Accelerations
  SET_SPEED(Speed_Data[0], Speed_Data[1], Speed_Data[2], Speed_Data[3]); // set the new speeds
  MOVE_FUNCTION();
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
