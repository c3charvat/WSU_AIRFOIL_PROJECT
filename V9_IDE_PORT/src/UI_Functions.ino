// Cutsom UI Functions Go Here
void MAIN_MENU(){
//MAIN MENU UI Function 
  // Head back to Main meanu
  current_selection = u8g2.userInterfaceSelectionList(   // Bings up the Main Menu
                        "Air Foil Control",
                        current_selection,
                        Main_menu);
}
//Serial UI Function 

// potentiall replace the void pass in with a pass in of the postition variables? ~~~ Find way to make this refresh with every serial input
int SERIAL_UI (void){
  // ~~~~~~~~~~~~~~~~~~~~~~SERIAL_UI Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Serial.println("This demo expects 12 pieces of data - in floating point value"); // Prep Serial Menu
  Serial.println("Enter data in this style <12, 12, 24.7.....>  ");
  Serial.println();
  while ( Com_selection == 1) {
    // Serial Stuff here
    recvWithStartEndMarkers();
    if (newData == true) {
      strcpy(tempChars, receivedChars);
      // this temporary copy is necessary to protect the original data
      //   because strtok() used in parseData() replaces the commas with \0
      parseData();
      showParsedData(); // Print out the AoA T, AoA B, X, Y and move function
      newData = false;
    }
    // End serial Stuff
    // Serial UI String Printout 
   String TempString0 = String(CurrentPositions[0]); // CONVERT CURRENT A0A POSITION INTO A STRING (Local Variables
   TempString0 += " X Pos"; // Just adding to the String here 
      String TempString1 = String(CurrentPositions[1]); // CONVERT CURRENT A0A POSITION INTO A STRING (Local Variables
   TempString1 += " Y Pos";
      String TempString2 = String(CurrentPositions[2]); // CONVERT CURRENT A0A POSITION INTO A STRING (Local Variables
   TempString2 += " AoA Top";
      String TempString3 = String(CurrentPositions[3]); // CONVERT CURRENT A0A POSITION INTO A STRING (Local Variables
   TempString3 += " AoA Bottom:";
    u8g2.clearBuffer();
    Draw_dialog(u8g2,0,0,128,64,"Serial Mode\n" ,TempString0,TempString1,TempString2,TempString3, "Return", false);
    u8g2.sendBuffer();
    if (digitalRead(BUTTON) == LOW ) { // if the encoder is pressed go back to LCD MODE (I used digital read here to simulate a long press)
      u8g2.clearBuffer();
    Draw_dialog(u8g2,0,0,128,64,"Serial Mode\n" ,TempString0,TempString1,TempString2,TempString3, "Return", true);
      delay(400);
      u8g2.sendBuffer();
      Com_selection = 2;
      return 1;
      MAIN_MENU();
    }
    //delay(50);// Stops screen from flashing horibly this is kinda a bandaid not sure whats goin on here
  } // End while loop
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Setup for a Button ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Draw_button(U8G2 u8g2, uint8_t x, uint8_t y, uint8_t width, String str, bool clicked){
    if (clicked) {
        u8g2.setDrawColor(1);
        u8g2.drawRBox(x, y+1, width,  u8g2.getMaxCharHeight() + 4, 2);
        u8g2.setDrawColor(0);
        u8g2.setFont(u8g2_font_5x8_tf);
        u8g2.drawStr(x + (width / 2) - ((String(str).length() * (u8g2.getMaxCharWidth())) / 2), y + u8g2.getMaxCharHeight()+3, str.c_str());
    } else {
        u8g2.setDrawColor(1);
        u8g2.drawRFrame(x, y, width,  u8g2.getMaxCharHeight() + 6, 4);
        u8g2.setFont(u8g2_font_5x8_tf);
        u8g2.drawStr(x + (width / 2) - ((String(str).length() * (u8g2.getMaxCharWidth())) / 2), y + u8g2.getMaxCharHeight()+2, str.c_str());      
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Setup for a dialog box with a button at the bottom ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Draw_dialog(U8G2 u8g2, uint8_t x, uint8_t y, uint8_t width, uint8_t height, String title, String msg1, String msg2, String msg3, String msg4, String btn, bool clicked) {
  u8g2.drawRFrame(x, y, width, height, 2);

  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(x + (width / 2) - ((String(title).length() * (u8g2.getMaxCharWidth())) / 2) , y + u8g2.getMaxCharHeight(), title.c_str());
  u8g2.drawHLine(x, y + u8g2.getMaxCharHeight() + 1, width);

  u8g2.drawStr(x + 2 , y + u8g2.getMaxCharHeight() * 2 + 1, msg1.c_str());
  u8g2.drawStr(x + 2 , y + u8g2.getMaxCharHeight() * 3 + 1, msg2.c_str());
  u8g2.drawStr(x + 2 , y + u8g2.getMaxCharHeight() * 4 + 1, msg3.c_str());
  u8g2.drawStr(x + 2 , y + u8g2.getMaxCharHeight() * 5 + 1, msg4.c_str());

  Draw_button(u8g2, x + width / 4, y + height - u8g2.getMaxCharHeight() * 2, width / 2, btn, clicked);

}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End Dialog  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
