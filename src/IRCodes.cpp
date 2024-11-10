#include "IRCodes.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Sharp.h>

// Pin configuration for IR LED
const uint8_t IR_LED = 27;

// Instances for sending IR signals
IRsend irSend(IR_LED);
IRSharpAc sharpAc(IR_LED);

// Temperature, fan settings, and swing state
int setTemp = 20;  // Initial temperature

int setFanIndex = 0;  // Initial fan speed index
// Fan speed modes (used for Sharp AC)
const uint8_t setFan[5] = {kSharpAcFanAuto, kSharpAcFanMin, kSharpAcFanMed, kSharpAcFanHigh, kSharpAcFanMax};
// Display labels for fan modes
const char* fanLabels[] = {"Auto", "Min", "Med", "High", "Max"};

bool setSwing = true;  // Swing state (On/Off)
// Function to get the string representation of swing state
const char* getSwingString(bool swing) {
  return swing ? "On" : "Off";  // Return "On" if true, "Off" if false
}

// Array of SymphonyCode for controlling Deka fan speeds
SymphonyCode fanDeka[] = {
  {0xD80, 12, 3},  // Fan off
  {0xD88, 12, 3},  // Speed 1
  {0xDC6, 12, 3},  // Speed 2
  {0xD82, 12, 3}  // Speed 3
};

// Function to initialize the IR sender
void initIrSend() {
  irSend.begin();
  sharpAc.begin();
}

// Function to send IR command for Deka fan speed based on selected index
void dekaSpeedControl(int index) {
  irSend.sendSymphony(fanDeka[index].code, fanDeka[index].bits, fanDeka[index].repeats);
}

// Functions to control Sharp AC power state
void sharpAcSetOn() {
  sharpAc.setTemp(setTemp);  // Set the current temperature
  sharpAc.setFan(setFan[setFanIndex]);  // Set fan to selected mode
  sharpAc.setMode(kSharpAcCool);  // Set AC mode to cooling
  sharpAc.setSwingToggle(setSwing);  // Set swing mode
  sharpAc.on();  // Turn the AC on
  sharpAc.send();  // Transmit the IR command
}

void sharpAcSetOff() {
  sharpAc.off();  // Turn the AC off
  sharpAc.send();  // Transmit the IR command
}

// Common UI rendering function for displaying temperature, fan speed, and swing status
void sharpAcUI() {
  char tempStr[4];  // Buffer to hold temperature as a string
  sprintf(tempStr, "%d", setTemp);  // Convert temperature to string

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);
  u8g2.setFont(u8g2_font_profont29_tr);
  u8g2.drawStr(38, 41, tempStr);
  u8g2.setFont(u8g2_font_profont22_tr);
  u8g2.drawStr(78, 41, "C");
  u8g2.setFont(u8g2_font_profont11_tr);
  u8g2.drawStr(38, 7, "Fan:");
  u8g2.drawStr(66, 7, fanLabels[setFanIndex]);
  u8g2.drawStr(38, 61, "Swing:");
  u8g2.drawStr(78, 61, getSwingString(setSwing));
  u8g2.sendBuffer();
}

// Functions to adjust AC settings based on encoder input
void sharpAcSetTemp() {
  // Increase or decrease temperature within valid range (16-30°C)
  if (encoderCurrentRead > encoderLastRead) {
    setTemp++;
    if (setTemp > 30) setTemp = 30;  // Prevent overflow
  }
  if (encoderLastRead > encoderCurrentRead) {
    setTemp--;
    if (setTemp < 16) setTemp = 16;  // Prevent underflow
  }
}

// Functions to update UI based on the selected AC setting
void sharpAcSetTempUI() {
  sharpAcSetTemp();  // Update temperature setting
  sharpAcUI();
}

void sharpAcSetFan() {
  // Change fan speed within valid range (index 0-4)
  if (encoderCurrentRead > encoderLastRead) {
    setFanIndex++;
    if (setFanIndex > 4) setFanIndex = 4;
  }
  if (encoderLastRead > encoderCurrentRead) {
    setFanIndex--;
    if (setFanIndex < 0) setFanIndex = 0;
  }
}

void sharpAcSetFanUI() {
  sharpAcSetFan();  // Update fan setting
  sharpAcUI();
}

void sharpAcSetSwing() {
  // Toggle swing state (on/off) based on encoder input
  if (encoderCurrentRead > encoderLastRead) setSwing = !setSwing;
  if (encoderLastRead > encoderCurrentRead) setSwing = !setSwing;
}

void sharpAcSetSwingUI() {
  sharpAcSetSwing();  // Update swing setting
  sharpAcUI();
}
