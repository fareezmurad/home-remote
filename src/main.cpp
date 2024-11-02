#include <Arduino.h>
#include <U8g2lib.h>
#include <Bounce2.h>
#include <ESP32Encoder.h>

// Define button pin numbers
#define CLK 36
#define DT 39
#define SELECT_BUTTON 34
// #define BACK_BUTTON_PIN 19

// Initialize the OLED display using the U8g2 library
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Initialize the rotary encoder using the ESP32Enccoder
ESP32Encoder rotaryEncoder;

// Initialize button objects for handling button presses
Bounce2::Button selectButton = Bounce2::Button();
// Bounce2::Button backButton = Bounce2::Button();

// Version information for display
const char* version = "v1.1";

struct MenuItem {
  const char* title;
  struct MenuItem* subMenu;
  void (*action)();
};

MenuItem dekaFanSpeed[] = {
  {"Off", nullptr, nullptr},
  {"Speed 1", nullptr, nullptr},
  {"Speed 2", nullptr, nullptr},
  {"Speed 3", nullptr, nullptr},
  {nullptr, nullptr, nullptr} // Count terminator. REQUIRED FOR EVERY MENU!
};

MenuItem irSendMenu[] = {
  {"Deka Fan", nullptr, nullptr},
  {"Sharp A/C", nullptr, nullptr},
  {"Daikin A/C", nullptr, nullptr},
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

MenuItem* currentMenu = mainMenu;

// Variables for tracking the encoder current values
int encoderCurrentRead = 0;
int encoderLastRead;

// Variable to track the menu
int selectedMenu = 0;

// Variables for tracking the currently selected item and starting index for scrolling
int selectedItemIndex = 0; // Index of the currently selected item
int startItemIndex = 0; // Track the starting index of the displayed menu items

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
    int yPos = (i * 12) + 26;  // Calculate the y position for each menu item
    u8g2.setFont(u8g2_font_spleen6x12_mr); // Set font for menu items
    u8g2.drawStr(1, yPos, currentMenu[startItemIndex + i].title); // Draw the menu item
  }
}

// Function for select button behaviour
void selectMenu() {
  if (selectButton.pressed()) {
    if (currentMenu[selectedMenu].action != nullptr) currentMenu[selectedMenu].action(); // Check if there is action or not
    else if (currentMenu[selectedMenu].subMenu != nullptr) { // Check if there is a menu
      currentMenu = currentMenu[selectedMenu].subMenu;
      currentMenu[selectedMenu].subMenu;

      startItemIndex = 0;
      selectedItemIndex = 0;
      selectedMenu = 0;
    }
  }
}

// Function to highlight the selected menu item
void highlightSelectedItem() {
  int totalMenuItems = getMenuItemCount(currentMenu);

  const int visibleItemsCount = min(totalMenuItems - startItemIndex, 3); // Limit to the number of items being displayed
  int yPos = (min(selectedItemIndex, 3) * 12) + 15; // Calculate y position for the highlighted item

  u8g2.setDrawColor(2); // Set draw color for the highlight
  u8g2.drawBox(0, yPos, 128, 12); // Draw a box to highlight the selected item

  // Update the selected item index based on rotary encoder
  if (encoderCurrentRead > encoderLastRead) {
    selectedMenu++;
    if (selectedMenu > totalMenuItems - 1) selectedMenu = totalMenuItems - 1;

    if (selectedItemIndex < visibleItemsCount - 1) {
      selectedItemIndex++; // Move down in the currently visible items
    } else if (startItemIndex + 3 < totalMenuItems) {
      // If at the end of visible items and more items exist
      startItemIndex++;  // Scroll down the list
    }
  }
  if (encoderLastRead > encoderCurrentRead) {
    selectedMenu--;
    if (selectedMenu < 0) selectedMenu = 0;

    if (selectedItemIndex > 0) {
      selectedItemIndex--; // Move up in the currently visible items
    } else if (startItemIndex > 0) {
      // If at the top of visible items and more items above exist
      startItemIndex--; // Scroll up the list
    }
  }
  selectMenu();
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
  u8g2.drawHLine(0, 53, 128); // Draw a horizontal line at the footer
  u8g2.setFont(u8g2_font_minuteconsole_mr); // Set font for footer
  u8g2.drawStr(128 - (strlen(version) * 5), 63, version); // Draw the version information at the bottom right
  
  u8g2.sendBuffer(); // Send the buffer to the display
}

void setup() {
  Serial.begin(115200); // Initialize serial communication
  u8g2.begin(); // Initialize the OLED display

  // Configure the rotary encoder
  rotaryEncoder.attachHalfQuad(DT, CLK);
  rotaryEncoder.setCount(0);

  // Configure the select button
  selectButton.attach(SELECT_BUTTON, INPUT);
  selectButton.interval(5); // Set debounce interval
  selectButton.setPressedState(LOW); // Set pressed state for active-low logic

  /*
  // Configure the back button
  backButton.attach(BACK_BUTTON_PIN, INPUT_PULLUP);
  backButton.interval(5); // Set debounce interval
  backButton.setPressedState(LOW); // Set pressed state for active-low logic
  */
}

void loop() {
  // Obtain encoder read value
  encoderCurrentRead = rotaryEncoder.getCount();
  
  // Update the button states
  selectButton.update();
  // backButton.update();

  // Draw the menu on the OLED display
  drawMenu();

  // Update the rotary encoder values for tracking
  encoderLastRead = encoderCurrentRead;
  Serial.println(getMenuItemCount(currentMenu));
}
