#include "IRCodes.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>
#include <ir_Sharp.h>

// Pin configuration for IR LED
const uint8_t IR_LED = 27;

// Instances for sending IR signals
IRsend irSend(IR_LED);
IRSharpAc sharpAc(IR_LED);
IRDaikin64 daikinAc(IR_LED);

// Function to initialize the IR sender
void initIrSend() {
  irSend.begin();
  sharpAc.begin();
  daikinAc.begin();
}

/* --------------- General variable and function for AC sends usage --------------- */
bool irSignalSent = false;  // Tracks if IR signal has been sent
unsigned long lastInputTime = 0;  // Records time of last encoder input
const unsigned long inactivityDuration = 2000;  // Duration (ms) before sending IR automatically

// Resets the inactivity timer and IR sent flag on new input
void resetTimer() {
  lastInputTime = millis();
  irSignalSent = false;
}

// Adjusts encoder-controlled value within specified limits
void inputEncoder(uint8_t& value, int min, int max) {
  if (encoderCurrentRead > encoderLastRead && value < max) {
    value++;  // Increment value if within max limit
    resetTimer();
  }
  if (encoderLastRead > encoderCurrentRead && value > min) {
    value--;  // Decrement value if within min limit
    resetTimer();
  }
}

// Adjust encoder-controlled value for toggling only
void toggleEncoder(bool& state) {
  if (encoderCurrentRead > encoderLastRead) {
    state = !state;  // Toggle state
    resetTimer();
  }
  if (encoderLastRead > encoderCurrentRead) {
    state = !state;  // Toggle state
    resetTimer();
  }
}

/*---------------------------DEKA FAN---------------------------*/
// Array of SymphonyCode for controlling Deka fan speeds
SymphonyCode fanDeka[] = {
  {0xD80, 12, 3},  // Fan off
  {0xD88, 12, 3},  // Speed 1
  {0xDC6, 12, 3},  // Speed 2
  {0xD82, 12, 3}  // Speed 3
};

// Function to send IR command for Deka fan speed based on selected index
void dekaSpeedControl(int index) {
  irSend.sendSymphony(fanDeka[index].code, fanDeka[index].bits, fanDeka[index].repeats);
}

/*-------------------------SHARP AIR-CONDITIONER-------------------------*/
uint8_t sharpSetTemp = 20;  // Initial temperature

uint8_t sharpSetFanIndex = 0;  // Initial fan speed index
const uint8_t sharpSetFan[5] = {kSharpAcFanAuto, kSharpAcFanMin, kSharpAcFanMed, kSharpAcFanHigh, kSharpAcFanMax};
const char* sharpFanLabel[] = {"Auto", "Min", "Med", "High", "Max"};

bool sharpSetSwing = true;  // Swing state (On/Off)
const char* sharpGetSwingString(bool swing) {
  return swing ? "On" : "Off";  // Return "On" if true, "Off" if false
}

// Common UI rendering function for displaying temperature, fan speed, and swing status
void sharpAcUI() {
  char tempStr[4];  // Buffer to hold temperature as a string
  sprintf(tempStr, "%d", sharpSetTemp);  // Convert temperature to string

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);
  u8g2.setFont(u8g2_font_profont29_tr);
  u8g2.drawStr(38, 41, tempStr);
  u8g2.setFont(u8g2_font_profont22_tr);
  u8g2.drawStr(78, 41, "C");
  u8g2.setFont(u8g2_font_profont11_tr);
  u8g2.drawStr(38, 7, "Fan:");
  u8g2.drawStr(66, 7, sharpFanLabel[sharpSetFanIndex]);
  u8g2.drawStr(38, 61, "Swing:");
  u8g2.drawStr(78, 61, sharpGetSwingString(sharpSetSwing));
  u8g2.sendBuffer();

  sharpAcChkInactivity();  // Send IR signal if there is no input
}

// Functions to control Sharp AC power state
void sharpAcSetOn() {
  sharpAc.setTemp(sharpSetTemp);  // Set the current temperature
  sharpAc.setFan(sharpSetFan[sharpSetFanIndex]);  // Set fan to selected mode
  sharpAc.setMode(kSharpAcCool);  // Set AC mode to cooling
  sharpAc.setSwingToggle(sharpSetSwing);  // Set swing mode
  sharpAc.on();  // Turn the AC on
  sharpAc.send();  // Transmit the IR command
}

void sharpAcSetOff() {
  sharpAc.off();  // Turn the AC off
  sharpAc.send();  // Transmit the IR command
}

// Function to send IR signal Automatically
void sharpAcChkInactivity() {
  if (!irSignalSent && millis() - lastInputTime >= inactivityDuration) {
    sharpAc.setTemp(sharpSetTemp);
    sharpAc.setFan(sharpSetFan[sharpSetFanIndex]);
    sharpAc.setMode(kSharpAcCool);
    sharpAc.setSwingToggle(sharpSetSwing);
    sharpAc.send();
    irSignalSent = true;
  }
}

// Increase or decrease temperature within valid range (16-30°C)
void sharpAcSetTemp() { inputEncoder(sharpSetTemp, 16, 30); }
void sharpAcSetTempUI() {
  sharpAcSetTemp();  // Update temperature setting
  sharpAcUI();
}

// Change fan speed within valid range (index 0-4)
void sharpAcSetFan() { inputEncoder(sharpSetFanIndex, 0, 4); }
void sharpAcSetFanUI() {
  sharpAcSetFan();  // Update fan setting
  sharpAcUI();
}

// Toggle swing state (on/off) based on encoder input
void sharpAcSetSwing() { toggleEncoder(sharpSetSwing); }
void sharpAcSetSwingUI() {
  sharpAcSetSwing();  // Update swing setting
  sharpAcUI();
}

/*-------------------------DAIKIN AIR-CONDITIONER-------------------------*/
static const unsigned char celcius_bits[] U8X8_PROGMEM = {
  0x38, 0x00, 0x44, 0x40, 0xd4, 0xa0, 0x54, 0x40, 0xd4, 0x1c, 0x54,
  0x06, 0xd4, 0x02, 0x54, 0x02, 0x54, 0x06, 0x92, 0x1c, 0x39, 0x01,
  0x75, 0x01, 0x7d, 0x01, 0x39, 0x01, 0x82, 0x00, 0x7c, 0x00};

uint8_t daikinSetTemp = 24;  // Set initial temperature
uint8_t daikinSetModeIndex = 1;  // Set initial AC mode
const uint8_t daikinSetMode[3] = {kDaikin64Dry, kDaikin64Cool, kDaikin64Fan};
const char* daikinSetModeLabel[3] = {"Dry", "Cool", "Fan"};
uint8_t daikinSetFanIndex = 1;  // Set initial Fan mode
const uint8_t daikinSetFan[6] = {kDaikin64FanQuiet, kDaikin64FanAuto, kDaikin64FanLow, kDaikin64FanMed, kDaikin64FanHigh, kDaikin64FanTurbo};
const char* daikinSetFanlabel[6] = {"Quiet", "Auto", "Min", "Med", "Max", "Turbo"};
bool daikinSetSwing = false;  // Set initial swing mode
const char* daikinGetSwingString(bool swing) {
  return swing ? "On" : "Off";  // Return "On" if true, "Off" if false
}

void daikinAcUI() {
  char tempStr[4];  // Buffer to hold temperature as a string
  sprintf(tempStr, "%d", daikinSetTemp);  // Convert temperature to string

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);
  u8g2.setFont(u8g2_font_profont29_tr);
  u8g2.drawStr(2, 41, tempStr);
  u8g2.drawXBMP(36, 22, 16, 16, celcius_bits);
  u8g2.setFont(u8g2_font_profont11_tr);
  u8g2.drawStr(63, 14, "Mode:");
  u8g2.drawStr(97, 14, daikinSetModeLabel[daikinSetModeIndex]);
  u8g2.drawRFrame(56, 0, 72, 22, 4);
  u8g2.drawStr(63, 35, "Fan:");
  u8g2.drawStr(95, 35, daikinSetFanlabel[daikinSetFanIndex]);
  u8g2.drawRFrame(56, 21, 72, 22, 4);
  u8g2.drawStr(63, 57, "Swing:");
  u8g2.drawStr(103, 57, daikinGetSwingString(daikinSetSwing));
  u8g2.drawRFrame(56, 42, 72, 22, 4);
  u8g2.setDrawColor(1);
  u8g2.sendBuffer();

  daikinAcChkInactivity();  // Send IR signal if there is no input
}

// Function to hold Daikin AC set value
void daikinAcSetting() {
  daikinAc.setTemp(daikinSetTemp);
  daikinAc.setMode(daikinSetMode[daikinSetModeIndex]);
  daikinAc.setFan(daikinSetFan[daikinSetFanIndex]);
  daikinAc.setSwingVertical(daikinSetSwing);
}

// Function to toggle power Daikin AC
void daikinAcToggleOn() {
  daikinAcSetting();
  daikinAc.setPowerToggle(true);
  daikinAc.send();
}

// Function to send IR signal automatically
void daikinAcChkInactivity() {
  if (!irSignalSent && millis() - lastInputTime >= inactivityDuration) {
    daikinAcSetting();
    daikinAc.setPowerToggle(false);
    daikinAc.send();
    irSignalSent = true;
  }
}

// Select Daikin AC temperature (16c-30c)
void daikinAcSetTemp() { inputEncoder(daikinSetTemp, 16, 30); }
void daikinAcSetTempUI() {
  daikinAcSetTemp();
  daikinAcUI();
}

// Select Daikin AC mode
void daikinAcSetMode() { inputEncoder(daikinSetModeIndex, 0, 2); }
void daikinAcSetModeUI() {
  daikinAcSetMode();
  daikinAcUI();
}

// Select Daikin AC fan mode
void daikinAcSetFan() { inputEncoder(daikinSetFanIndex, 0, 5); }
void daikinAcSetFanUI() {
  daikinAcSetFan();
  daikinAcUI();
}

// Toggle Daikin AC swing mode
void daikinAcSetSwing() { toggleEncoder(daikinSetSwing); }
void daikinAcSetSwingUI() {
  daikinAcSetSwing();
  daikinAcUI();
}