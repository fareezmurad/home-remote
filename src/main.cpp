#include <Arduino.h>

#include <Bounce2.h>
#include <ESP32Encoder.h>
#include <U8g2lib.h>

#include "IRCodes.h"
#include "Info.h"
#include "ESPNOW.h"

// Define pin numbers
#define CLK 27
#define DT 25
#define SELECT_BUTTON 32

// Initialize the OLED display object (U8g2 library)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Initialize the rotary encoder object (ESP32Encoder library)
ESP32Encoder rotaryEncoder;

// Initialize button object for "select" button with debouncing (Bounce2 library)
Bounce2::Button selectButton = Bounce2::Button();

// Version info
const char *version = "v1.75";

// Menu item structure for title, optional submenu, action and state of display for action
struct MenuItem {
  const char *title;
  struct MenuItem *subMenu;
  void (*action)();
  bool requireUpdateDisplay;
};

MenuItem dekaFanMenu[] = {
  {"Off", nullptr, []() { dekaSpeedControl(0); }, false},
  {"Speed 1", nullptr, []() { dekaSpeedControl(1); }, false},
  {"Speed 2", nullptr, []() { dekaSpeedControl(2); }, false},
  {"Speed 3", nullptr, []() { dekaSpeedControl(3); }, false},
  {"Back", nullptr, nullptr, false},  // Back button (ONLY FOR SUB-MENU)
  {nullptr, nullptr, nullptr, false}  // Count terminator. REQUIRED FOR EVERY MENU!
};

MenuItem sharpAcMenu[] = {
  {"On", nullptr, sharpAcSetOn, false},
  {"AC Mode", nullptr, sharpAcSetModeUI, true},
  {"Temperature", nullptr, sharpAcSetTempUI, true},
  {"Fan Mode", nullptr, sharpAcSetFanUI, true},
  {"Swing", nullptr, sharpAcSetSwingUI, true},
  {"Off", nullptr, sharpAcSetOff, false},
  {"Back", nullptr, nullptr, false},  // Back button (ONLY FOR SUB-MENU)
  {nullptr, nullptr, nullptr, false}  // Count terminator. REQUIRED FOR EVERY MENU!
};

MenuItem daikinAcMenu[] = {
  {"Power Toggle", nullptr, daikinAcToggleOn, false},
  {"AC Mode", nullptr, daikinAcSetModeUI, true},
  {"Temperature", nullptr, daikinAcSetTempUI, true},
  {"Fan Mode", nullptr, daikinAcSetFanUI, true},
  {"Swing", nullptr, daikinAcSetSwingUI, true},
  {"Back", nullptr, nullptr, false},  // Back button (ONLY FOR SUB-MENU)
  {nullptr, nullptr, nullptr, false}  // Count terminator. REQUIRED FOR EVERY MENU!
};

MenuItem irSendMenu[] = {
  {"Deka Fan", dekaFanMenu, nullptr, false},
  {"Sharp A/C", sharpAcMenu, nullptr, false},
  {"Daikin A/C", daikinAcMenu, nullptr, false},
  {"Back", nullptr, nullptr, false},  // Back button (ONLY FOR SUB-MENU)
  {nullptr, nullptr, nullptr, false}  // Count terminator. REQUIRED FOR EVERY MENU!
};

MenuItem homeAutomationMenu[] = {
  {"Switch 1", nullptr, sendDataSwitch1, false},
  {"Switch 2", nullptr, sendDataSwitch2, false},
  {"Switch 3", nullptr, sendDataSwitch3, false},
  {"Switch 4", nullptr, sendDataSwitch4, false},
  {"Back", nullptr, nullptr, false},  // Back button (ONLY FOR SUB-MENU)
  {nullptr, nullptr, nullptr, false}  // Count terminator. REQUIRED FOR EVERY MENU!
};

void underDevelopment();
MenuItem mainMenu[] = {
  {"Home Automation", homeAutomationMenu, nullptr, false},
  {"IR Remote", irSendMenu, nullptr, false},
  {"QR Codes", nullptr, displayQr, true},
  {"Information", nullptr, displayInfo, true},
  {"Exit", nullptr, nullptr, false},
  {nullptr, nullptr, nullptr, false}  // Count terminator. REQUIRED FOR EVERY MENU!
};

// Function to calculate the number of menu items dynamically
int getMenuItemCount(MenuItem *menu) {
  int count = 0;
  while (menu[count].title != nullptr) {
    count++;
  }
  return count;
};

// Track current menu state, menu history, header and menu depth for nested menus
MenuItem *currentMenu = mainMenu;
const int MAX_MENU_DEPTH = 10;  // Max levels of menu nesting
MenuItem *menuStack[MAX_MENU_DEPTH];  // Stack to store menu history
const char *headerStack[MAX_MENU_DEPTH];  // Stack to store current and previous display header
int menuDepth = 0;  // Current depth in the menu stack

// Track rotary encoder states
int encoderCurrentRead = 0;
int encoderLastRead;

// Track actual index for selected/highlighted menu item
int currentItemIndex = 0;

// Track visible display menu indexes for scrolling
int displaySelectedItemIndex = 0;  // Index of the currently selected item
int displayStartItemIndex = 0;  // Track the starting index of the displayed menu items

// Global flag to track if a non-menu screen is being displayed
bool displayingScreen = false;

// Function to draw the header
void drawHeader(const char *header) {
  // Display "MAIN MENU" if at top level
  if (header == 0) header = "MAIN MENU";

  // Create a temporary buffer to hold the capitalized string
  char headerUpper[strlen(header) + 1];
  strcpy(headerUpper, header);  // Copy the input string to the buffer

  // Convert each character to uppercase
  for (int i = 0; headerUpper[i] != '\0'; i++) {
    headerUpper[i] = toupper(headerUpper[i]);
  }

  u8g2.setFont(u8g2_font_spleen8x16_mr);  // Set font for header
  u8g2.drawStr((128 - (strlen(headerUpper) * 8)) / 2, 10, headerUpper);  // Draw the capitalized header text
  u8g2.drawHLine(0, 12, 128);  // Draw a horizontal line below the header
}

// Function to draw the list up to 3 menu items
void drawMenuList() {
  for (int i = 0; i < 3; i++) {
    int yPos = (i * 12) + 25;  // Calculate the y position for each menu item
    u8g2.setFont(u8g2_font_spleen6x12_mr);  // Set font for menu items
    u8g2.drawStr(1, yPos, currentMenu[displayStartItemIndex + i].title);  // Draw the menu item
  }
}

// Handle "select" button press for menu navigation
void selectHighlightedMenu() {
  if (selectButton.pressed()) {
    if (currentMenu[currentItemIndex].action != nullptr) {  // Execute action if defined
      // Check if the action requires display update
      if (currentMenu[currentItemIndex].requireUpdateDisplay) displayingScreen = true;  // function require display to be updated
      else currentMenu[currentItemIndex].action();  // Only execute code without requiring to update the display
    } else if (currentMenu[currentItemIndex].subMenu != nullptr && menuDepth < MAX_MENU_DEPTH) {  // Enter sub-menu if defined
      headerStack[menuDepth] = currentMenu[currentItemIndex].title;  // Push current menu title onto the stack to update the header
      // Push current menu onto the stack before entering submenu
      menuStack[menuDepth++] = currentMenu;
      currentMenu = currentMenu[currentItemIndex].subMenu;
      // Reset index for display and selection
      displayStartItemIndex = 0;
      displaySelectedItemIndex = 0;
      currentItemIndex = 0;
    } else if (currentItemIndex == (getMenuItemCount(currentMenu) - 1) && menuDepth > 0) {  // Go back if select 'back' option in sub-menu
      // "Back" option: Pop the previous menu from the stack
      currentMenu = menuStack[--menuDepth];
      // Reset index for display and selection
      displayStartItemIndex = 0;
      displaySelectedItemIndex = 0;
      currentItemIndex = 0;
    }
  }
}

// Draw and highlight the currently selected menu item
void highlightSelectedItem() {
  int totalMenuItems = getMenuItemCount(currentMenu);  // Get total current menu count

  const int visibleItemsCount = min(totalMenuItems - displayStartItemIndex, 3);  // Limit to the number of items being displayed
  int yPos = (min(displaySelectedItemIndex, 3) * 12) + 15;  // Calculate y position for the highlighted item

  u8g2.setDrawColor(2);  // Set draw color for the highlight
  u8g2.drawBox(0, yPos, 128, 13);  // Draw a box to highlight the selected item

  // Handle encoder rotation for menu navigation
  if (encoderCurrentRead > encoderLastRead) {
    currentItemIndex++;

    if (currentItemIndex > totalMenuItems - 1) currentItemIndex = totalMenuItems - 1;  // Prevent overflow

    if (displaySelectedItemIndex < visibleItemsCount - 1) displaySelectedItemIndex++;  // Move down in the currently visible items
    else if (displayStartItemIndex + 3 < totalMenuItems) displayStartItemIndex++;  // Scroll down the list
  }

  if (encoderLastRead > encoderCurrentRead) {
    currentItemIndex--;

    if (currentItemIndex < 0) currentItemIndex = 0;  // Prevent overflow

    if (displaySelectedItemIndex > 0) displaySelectedItemIndex--;  // Move up in the currently visible items
    else if (displayStartItemIndex > 0) displayStartItemIndex--;  // Scroll up the list
  }
  selectHighlightedMenu();
}

// Function to draw the entire menu screen
void drawMenu() {
  u8g2.clearBuffer();  // Clear the display buffer
  u8g2.setFontMode(1);  // Set font mode
  u8g2.setBitmapMode(1);  // Set bitmap mode

  if (displayingScreen) {  // Check if function require to update the display
    currentMenu[currentItemIndex].action();
    if (selectButton.pressed()) displayingScreen = false;
  } else {
    drawHeader(headerStack[menuDepth - 1]);
    drawMenuList();
    highlightSelectedItem();

    // Footer with version info
    u8g2.drawHLine(0, 54, 128);  // Draw a horizontal line at the footer
    u8g2.setFont(u8g2_font_minuteconsole_mr);  // Set font for footer
    u8g2.drawStr(128 - (strlen(version) * 5), 63, version);  // Draw the version information at the bottom right

    u8g2.sendBuffer();  // Send the buffer to the display
  }
}

// Dummy function for testing
void underDevelopment() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x13_tr);
  u8g2.drawStr(13, 37, "Under Development");
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  u8g2.begin();  // Initialize the OLED display
  initIrSend();  // Initialize the IR LED
  dataUpdateOnStartup();  // Update Home Automation Data

  // Configure the rotary encoder
  rotaryEncoder.attachHalfQuad(DT, CLK);
  rotaryEncoder.setCount(0);

  // Configure the select button
  selectButton.attach(SELECT_BUTTON, INPUT);
  selectButton.interval(5);  // Set debounce interval
  selectButton.setPressedState(LOW);  // Set pressed state for active-low logic
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
