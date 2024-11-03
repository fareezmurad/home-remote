#include <Arduino.h>
#include <U8g2lib.h>
#include <Bounce2.h>
#include <ESP32Encoder.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "IRCodes.h"

// Define pin numbers
#define CLK 36
#define DT 39
#define SELECT_BUTTON 34
#define IR_LED 27

// Initialize the OLED display using the U8g2 library
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Initialize the rotary encoder using the ESP32Enccoder
ESP32Encoder rotaryEncoder;

// Initialize button objects for handling button presses
Bounce2::Button selectButton = Bounce2::Button();

// Initialize the IRsend objects to send IR signals
IRsend irsend(IR_LED); 

// Version information for display
const char* version = "v1.2";

struct MenuItem {
  const char* title;
  struct MenuItem* subMenu;
  void (*action)();
};

void dekaSpeedControl(int index); // calling the function to the top
MenuItem dekaFanMenu[] = {
  {"Off", nullptr, []() { dekaSpeedControl(0); }},
  {"Speed 1", nullptr, []() { dekaSpeedControl(1); }},
  {"Speed 2", nullptr, []() { dekaSpeedControl(2); }},
  {"Speed 3", nullptr, []() { dekaSpeedControl(3); }},
  {"Back", nullptr, nullptr}, // Back button (ONLY FOR SUB-MENU)
  {nullptr, nullptr, nullptr} // Count terminator. REQUIRED FOR EVERY MENU!
};

MenuItem irSendMenu[] = {
  {"Deka Fan", dekaFanMenu, nullptr},
  {"Sharp A/C", nullptr, nullptr},
  {"Daikin A/C", nullptr, nullptr},
  {"Back", nullptr, nullptr}, // Back button (ONLY FOR SUB-MENU)
  {nullptr, nullptr, nullptr} // Count terminator. REQUIRED FOR EVERY MENU!
};

// Define the menu items
MenuItem mainMenu[] = {
  {"Home Automation", nullptr, nullptr},
  {"IR Sends", irSendMenu, nullptr},
  {"QR Codes", nullptr, nullptr},
  {"Information", nullptr, nullptr},
  {"Exit", nullptr, nullptr},
  {nullptr, nullptr, nullptr} // Count terminator. REQUIRED FOR EVERY MENU!
};

// Calculate the total number of menu items
int getMenuItemCount(MenuItem* menu) {
  int count = 0;
  while (menu[count].title != nullptr) {
    count++;
  }
  return count;
};

// To track current and previous menu for display and button
MenuItem* currentMenu = mainMenu;
const int MAX_MENU_DEPTH = 10; // Max levels of menu nesting
MenuItem* menuStack[MAX_MENU_DEPTH]; // Stack to store menu history
int menuDepth = 0; // Current depth in the menu stack

// Variables for tracking the encoder current values
int encoderCurrentRead = 0;
int encoderLastRead;

// Variable to track the which menu index currently be highlight
int currentItemIndex = 0;

// Variables for tracking the currently selected item and starting index for /display/scrolling
int displaySelectedItemIndex = 0; // Index of the currently selected item
int displayStartItemIndex = 0; // Track the starting index of the displayed menu items

// Function to draw the header
void drawHeader(const char* header) {
  u8g2.setFont(u8g2_font_spleen8x16_mr); // Set font for header
  u8g2.drawStr(28, 10, header); // Draw the header text
  u8g2.drawHLine(0, 12, 128); // Draw a horizontal line below the header
}

// Function to draw the list of menu items
void drawMenuList() {
  // Draw up to 3 items based on the starting index
  for (int i = 0; i < 3; i++) {
    int yPos = (i * 12) + 25;  // Calculate the y position for each menu item
    u8g2.setFont(u8g2_font_spleen6x12_mr); // Set font for menu items
    u8g2.drawStr(1, yPos, currentMenu[displayStartItemIndex + i].title); // Draw the menu item
  }
}

// Function for select button behaviour
void selectHighlightedMenu() {
  if (selectButton.pressed()) {
    if (currentMenu[currentItemIndex].action != nullptr) currentMenu[currentItemIndex].action(); // Check if there is action or not
    else if (currentMenu[currentItemIndex].subMenu != nullptr && menuDepth < MAX_MENU_DEPTH) { // Check if there is a menu
      // Push current menu onto the stack before entering submenu
      menuStack[menuDepth++] = currentMenu;
      currentMenu = currentMenu[currentItemIndex].subMenu;

      displayStartItemIndex = 0;
      displaySelectedItemIndex = 0;
      currentItemIndex = 0;
    }

    else if (currentItemIndex == (getMenuItemCount(currentMenu) - 1) && menuDepth > 0) {
      // "Back" option: Pop the previous menu from the stack
      currentMenu = menuStack[--menuDepth];

      displayStartItemIndex = 0;
      displaySelectedItemIndex = 0;
      currentItemIndex = 0;
    }
  }
}

// Function to highlight the selected menu item
void highlightSelectedItem() {
  int totalMenuItems = getMenuItemCount(currentMenu); // Get total current menu count

  const int visibleItemsCount = min(totalMenuItems - displayStartItemIndex, 3); // Limit to the number of items being displayed
  int yPos = (min(displaySelectedItemIndex, 3) * 12) + 15; // Calculate y position for the highlighted item

  u8g2.setDrawColor(2); // Set draw color for the highlight
  u8g2.drawBox(0, yPos, 128, 13); // Draw a box to highlight the selected item

  // Update the selected item index based on rotary encoder
  if (encoderCurrentRead > encoderLastRead) {
    currentItemIndex++;

    if (currentItemIndex > totalMenuItems - 1) currentItemIndex = totalMenuItems - 1; // To avoid overflow index count

    if (displaySelectedItemIndex < visibleItemsCount - 1) displaySelectedItemIndex++; // Move down in the currently visible items
    else if (displayStartItemIndex + 3 < totalMenuItems) displayStartItemIndex++;  // Scroll down the list
  }

  if (encoderLastRead > encoderCurrentRead) {
    currentItemIndex--;

    if (currentItemIndex < 0) currentItemIndex = 0; // Set minimum limit to avoid overflow

    if (displaySelectedItemIndex > 0) displaySelectedItemIndex--; // Move up in the currently visible items
    else if (displayStartItemIndex > 0) displayStartItemIndex--; // Scroll up the list
  }
  selectHighlightedMenu();
}

// Function to draw the entire menu screen
void drawMenu() {
  u8g2.clearBuffer(); // Clear the display buffer
  u8g2.setFontMode(1); // Set font mode
  u8g2.setBitmapMode(1); // Set bitmap mode

  drawHeader("MAIN MENU"); // Call the function to draw the header
  drawMenuList(); // Call the function to draw the list of menu items
  highlightSelectedItem(); // Call the function to highlight the selected item

  // Footer
  u8g2.drawHLine(0, 54, 128); // Draw a horizontal line at the footer
  u8g2.setFont(u8g2_font_minuteconsole_mr); // Set font for footer
  u8g2.drawStr(128 - (strlen(version) * 5), 63, version); // Draw the version information at the bottom right
  
  u8g2.sendBuffer(); // Send the buffer to the display
}

// Function to send IR to Deka ceiling fan
void dekaSpeedControl(int index) {
  irsend.sendSymphony(fanDeka[index].code, fanDeka[index].bits, fanDeka[index].repeats); // code save in IRCodes.cpp
}

void setup() {
  Serial.begin(115200); // Initialize serial communication
  u8g2.begin(); // Initialize the OLED display
  irsend.begin(); // Initialize the IR LED

  // Configure the rotary encoder
  rotaryEncoder.attachHalfQuad(DT, CLK);
  rotaryEncoder.setCount(0);

  // Configure the select button
  selectButton.attach(SELECT_BUTTON, INPUT);
  selectButton.interval(5); // Set debounce interval
  selectButton.setPressedState(LOW); // Set pressed state for active-low logic
}

void loop() {
  // Obtain encoder read value
  encoderCurrentRead = rotaryEncoder.getCount();
  
  // Update the button states
  selectButton.update();

  // Draw the menu on the OLED display
  drawMenu();

  // Update the rotary encoder values for tracking
  encoderLastRead = encoderCurrentRead;
}
