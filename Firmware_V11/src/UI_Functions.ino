// Cutsom UI Functions Go Here
void MAIN_MENU()
{
  // MAIN MENU UI Function
  //  Head back to Main meanu
  current_selection = u8g2.userInterfaceSelectionList( // Bings up the Main Menu
      "Air Foil Control",
      current_selection,
      Main_menu);
}
// Serial UI Function

// potentiall replace the void pass in with a pass in of the postition variables? ~~~ Find way to make this refresh with every serial input
void SERIAL_UI(void)
{
  // ~~~~~~~~~~~~~~~~~~~~~~SERIAL_UI Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  serial_flush_buffer();                                                                                                                                                                                      // Flush out anything that was ented before it was suposed to be.
  Serial.println("This demo expects data frommated like:\n <G X###.## Y###.## AoAT###.## AoAB###.##>\n OR\n <M A X###.## Y###.## AoAT###.## AoAB###.##>\n OR\n <M S X###.## Y###.## AoAT###.## AoAB###.##>"); // Prep Serial Menu
  Serial.println("FAILURE TO FOLLOW THE FORMATTING CAN CAUSE UNEXPECTED SYSTEM MOVEMENT\n IVE DONE MY BEST TO PREVENT THIS\n");
  Serial.println();
  while (Com_selection == 1)
  {
    // Serial Stuff here
    recvWithStartEndMarkers();
    if (newData == true)
    {
      strcpy(tempChars, receivedChars);
      // this temporary copy is necessary to protect the original data
      //   because strtok() used in parseData() replaces the commas with \0
      parseData();
      showParsedData(); // Print out the AoA T, AoA B, X, Y and move function // In order to support the Python GUI the data must be printed with every updata to position Thus this was moved to the move function.
      newData = false;
    }
    // End serial Stuff
    // Serial UI String Printout
    String TempString0 = String(CurrentPositions[0]); // CONVERT CURRENT A0A POSITION INTO A STRING (Local Variables
    TempString0 += " X Pos";                          // Just adding to the String here
    String TempString1 = String(CurrentPositions[1]); // CONVERT CURRENT A0A POSITION INTO A STRING (Local Variables
    TempString1 += " Y Pos";
    String TempString2 = String(CurrentPositions[2] * -1); // CONVERT CURRENT A0A POSITION INTO A STRING (Local Variables
    TempString2 += " AoA Top";
    String TempString3 = String(CurrentPositions[3] * -1); // CONVERT CURRENT A0A POSITION INTO A STRING (Local Variables
    TempString3 += " AoA Bottom:";
    u8g2.clearBuffer();
    Draw_dialog(u8g2, 0, 0, 128, 64, "Serial Mode\n", TempString0, TempString1, TempString2, TempString3, "Return", false);
    u8g2.sendBuffer();
    if (digitalRead(BUTTON) == LOW)
    { // if the encoder is pressed go back to LCD MODE (I used digital read here to simulate a long press)
      u8g2.clearBuffer();
      Draw_dialog(u8g2, 0, 0, 128, 64, "Serial Mode\n", TempString0, TempString1, TempString2, TempString3, "Return", true);
      delay(400);
      u8g2.sendBuffer();
      Com_selection = 2;
      Sub_selection = 1; // default sub_selction back to 1 so you dont end up in the serial menu every time you click settings.
      // return 1;
      MAIN_MENU();
    }
  } // End while loop
}
// untested code bit aim here is to simplfy the direction menus down to one input menu
// u8g2.userInterfaceInputValue("AOA top:", "-", &AoA_t_value[0], 0, 20, 1, " 0-20 Negitive");
void Draw_userinput(const char *title, const char *pre, float *DisplayValue, float lo, float hi, const char *post, float increments[])
{
  // float DisplayValue = *Value;
  int button_event = 0;
  String TempString = String(*DisplayValue); // convert to a string
  u8g2.clearBuffer();
  for (int i = 0; i <= 4; i++)
  {
    do
    {
      // draw the stuff here
      // u8g2.clear();
      Draw_dialog(u8g2, 0, 0, 128, 64, title, pre, TempString, String(increments[i]), post, "Enter", true);
      u8g2.sendBuffer();
      button_event = check_button_event();
      if (button_event == U8X8_MSG_GPIO_MENU_NEXT)
      { // if the encoder is truned positive
        if (*DisplayValue + increments[i] < hi)
        {
          *DisplayValue = *DisplayValue + increments[i];
        }
        button_event = 0;
        Serial.println("NEXT");
      }
      if (button_event == U8X8_MSG_GPIO_MENU_PREV)
      { // if the encoder is truned negitive
        if (*DisplayValue - increments[i] > lo) // if the next step is still in range
        {
          *DisplayValue = *DisplayValue - increments[i];
        }
        button_event = 0;
        Serial.println("PREV");
      }
      TempString = String(*DisplayValue); // convert to a string
      delay(20);                          // debounce stop this functgion fom running so fast the vlaues go everywhere
      u8g2.clearBuffer();
    } while (button_event != U8X8_MSG_GPIO_MENU_SELECT);
    button_event = 0; // reset button_event back to zero
    //*Value=DisplayValue;
  }
}

int check_button_event()
{
  button_event = 0;
  do
  {
    button_event = u8g2.getMenuEvent();
    if (button_event != 0)
    {
      Serial.println(button_event);
    }
  } while (button_event == 0);
  return button_event;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Setup for a Button ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following is a custom menu i wrote for the serial UI LCD menu
// See here for examples https://p3dt.net/u8g2sim/
void Draw_button(U8G2 u8g2, uint8_t x, uint8_t y, uint8_t width, String str, bool clicked)
{
  if (clicked)
  {
    u8g2.setDrawColor(1);
    u8g2.drawRBox(x, y + 1, width, u8g2.getMaxCharHeight() + 4, 2);
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(x + (width / 2) - ((String(str).length() * (u8g2.getMaxCharWidth())) / 2), y + u8g2.getMaxCharHeight() + 3, str.c_str());
  }
  else
  {
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(x, y, width, u8g2.getMaxCharHeight() + 6, 4);
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(x + (width / 2) - ((String(str).length() * (u8g2.getMaxCharWidth())) / 2), y + u8g2.getMaxCharHeight() + 2, str.c_str());
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Setup for a dialog box with a button at the bottom ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Draw_dialog(U8G2 u8g2, uint8_t x, uint8_t y, uint8_t width, uint8_t height, String title, String pre, String value, String increment, String post, String btn, bool clicked)
{
  u8g2.drawRFrame(x, y, width, height, 2);

  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(x + (width / 2) - ((String(title).length() * (u8g2.getMaxCharWidth())) / 2), y + u8g2.getMaxCharHeight(), title.c_str());
  u8g2.drawHLine(x, y + u8g2.getMaxCharHeight() + 1, width);

  u8g2.drawStr(x + 2, y + u8g2.getMaxCharHeight() * 2 + 1, pre.c_str());
  u8g2.drawStr(x + 2, y + u8g2.getMaxCharHeight() * 3 + 1, value.c_str());
  u8g2.drawStr(x + 2, y + u8g2.getMaxCharHeight() * 4 + 1, increment.c_str());
  u8g2.drawStr(x + 2 + u8g2.getStrWidth(increment.c_str()), y + u8g2.getMaxCharHeight() * 4 + 1, post.c_str());

  Draw_button(u8g2, x + width / 4, y + height - u8g2.getMaxCharHeight() * 2, width / 2, btn, clicked);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End Dialog  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
