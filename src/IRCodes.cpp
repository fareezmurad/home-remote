#include "IRCodes.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>
#include <ir_Sharp.h>

// Pin configuration for IR LED
const uint8_t IR_LED = 17;

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

// Bitmap for temperature indicator
static const unsigned char celcius_bits[] U8X8_PROGMEM = {
  0x38, 0x00, 0x44, 0x40, 0xd4, 0xa0, 0x54, 0x40, 0xd4, 0x1c, 0x54,
  0x06, 0xd4, 0x02, 0x54, 0x02, 0x54, 0x06, 0x92, 0x1c, 0x39, 0x01,
  0x75, 0x01, 0x7d, 0x01, 0x39, 0x01, 0x82, 0x00, 0x7c, 0x00};

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
bool currentPowerState = false;  // Variable to track the power state

uint8_t sharpSetTemp = 20;  // Initial temperature

uint8_t sharpSetModeIndex = 0;  // Initial AC mode
const uint8_t sharpSetMode[3] = {kSharpAcFan, kSharpAcDry, kSharpAcCool};
const char* sharpSetModeLabel[3] = {"Auto", "Dry", "Cool"};
uint8_t sharpSetFanIndex = 0;  // Initial fan speed index
const uint8_t sharpSetFan[4] = {kSharpAcFanAuto, kSharpAcFanMin, kSharpAcFanMed, kSharpAcFanMax};
const char* sharpSetFanLabel[] = {"Auto", "Min", "Med", "Max"};

bool sharpSetSwing = true;  // Swing state (On/Off)
const char* sharpGetSwingString(bool swing) {
  return swing ? "On" : "Off";  // Return "On" if true, "Off" if false
}

// Fan always set to auto whenever mode changed
void sharpValidateFanSetting() {
  static uint8_t lastModeIndex = 0;
  if (sharpSetModeIndex != lastModeIndex) sharpSetFanIndex = 0;
  lastModeIndex = sharpSetModeIndex;
}

void sharpAcUI() {
  char tempStr[4];  // Buffer to hold temperature as a string
  if (sharpSetModeIndex == 0 || sharpSetModeIndex == 1)
    strcpy(tempStr, "--");  // For mode that can't control temp setting
  else
    sprintf(tempStr, "%d", sharpSetTemp);  // Convert temperature to string if temp control available

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);
  u8g2.setFont(u8g2_font_profont29_tr);
  u8g2.drawStr(2, 41, tempStr);
  u8g2.drawXBMP(36, 22, 16, 16, celcius_bits);
  u8g2.setFont(u8g2_font_profont11_tr);
  u8g2.drawStr(63, 14, "Mode:");
  u8g2.drawStr(97, 14, sharpSetModeLabel[sharpSetModeIndex]);
  u8g2.drawRFrame(56, 0, 72, 22, 4);
  u8g2.drawStr(63, 35, "Fan:");
  u8g2.drawStr(95, 35, sharpSetFanLabel[sharpSetFanIndex]);
  u8g2.drawRFrame(56, 21, 72, 22, 4);
  u8g2.drawStr(63, 57, "Swing:");
  u8g2.drawStr(103, 57, sharpGetSwingString(sharpSetSwing));
  u8g2.drawRFrame(56, 42, 72, 22, 4);
  u8g2.setDrawColor(1);
  u8g2.sendBuffer();

  sharpAcChkInactivity();  // Send IR signal if there is no input
}

// Function to hold sharp AC set value
void sharpAcSetting() {
  sharpAc.setTemp(sharpSetTemp);  // Set the current temperature
  sharpAc.setFan(sharpSetFan[sharpSetFanIndex]);  // Set fan to selected mode
  sharpAc.setMode(sharpSetMode[sharpSetModeIndex]);  // Set AC mode to selected mode
  sharpAc.setSwingToggle(sharpSetSwing);  // Set swing mode
}

// Function to toggle power of Sharp AC
void sharpAcPowerToggle() {
  if (currentPowerState) {
    sharpAc.off();
    currentPowerState = false;
  } else {
    sharpAcSetting();
    sharpAc.on();
    currentPowerState = true;
  }
  sharpAc.send();
}

// Function to send IR signal Automatically
void sharpAcChkInactivity() {
  if (!irSignalSent && millis() - lastInputTime >= inactivityDuration) {
    sharpAcSetting();
    sharpAc.setPower(false);
    sharpAc.send();
    irSignalSent = true;
  }
}

// Increase or decrease temperature within valid range (16-30°C)
void sharpAcSetTemp() {
  if (sharpSetModeIndex == 2) inputEncoder(sharpSetTemp, 16, 30);
}
void sharpAcSetTempUI() {
  sharpAcSetTemp();  // Update temperature setting
  sharpAcUI();
}

// Change fan speed based on current AC mode
void sharpAcSetFan() {
  if (sharpSetModeIndex == 0 || sharpSetModeIndex == 1) {
    inputEncoder(sharpSetFanIndex, 0, 0);
  } else {
    inputEncoder(sharpSetFanIndex, 0, 3);
  }
}
void sharpAcSetFanUI() {
  sharpAcSetFan();  // Update fan setting
  sharpAcUI();
}

// Change mode within valid range (index 0-2)
void sharpAcSetMode() {
  inputEncoder(sharpSetModeIndex, 0, 2);
  sharpValidateFanSetting();
}
void sharpAcSetModeUI() {
  sharpAcSetMode();  // Update mode setting
  sharpAcUI();
}

// Toggle swing state (on/off) based on encoder input
void sharpAcSetSwing() { toggleEncoder(sharpSetSwing); }
void sharpAcSetSwingUI() {
  sharpAcSetSwing();  // Update swing setting
  sharpAcUI();
}

/*-------------------------DAIKIN AIR-CONDITIONER-------------------------*/
uint8_t daikinSetTemp = 24;  // Set initial temperature

uint8_t daikinSetModeIndex = 1;  // Set initial AC mode
const uint8_t daikinSetMode[3] = {kDaikin64Dry, kDaikin64Cool, kDaikin64Fan};
const char* daikinSetModeLabel[3] = {"Dry", "Cool", "Fan"};
uint8_t daikinSetFanIndex = 1;  // Set initial Fan mode
const uint8_t daikinSetFan[6] = {kDaikin64FanQuiet, kDaikin64FanAuto, kDaikin64FanLow, kDaikin64FanMed, kDaikin64FanHigh, kDaikin64FanTurbo};
const char* daikinSetFanLabel[6] = {"Quiet", "Auto", "Min", "Med", "Max", "Turbo"};

bool daikinSetSwing = false;  // Set initial swing mode
const char* daikinGetSwingString(bool swing) {
  return swing ? "On" : "Off";  // Return "On" if true, "Off" if false
}

void daikinValidateFanSetting() {
  static uint8_t lastModeIndex = 255;
  if (daikinSetModeIndex == 2 && lastModeIndex != 2 && (daikinSetFanIndex < 2 || daikinSetFanIndex > 4)) daikinSetFanIndex = 2;
  lastModeIndex = daikinSetModeIndex;
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
  u8g2.drawStr(95, 35, daikinSetFanLabel[daikinSetFanIndex]);
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
void daikinAcPowerToggle() {
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
void daikinAcSetMode() {
  inputEncoder(daikinSetModeIndex, 0, 2);
  daikinValidateFanSetting();  // To check if fan set to the correct setting based on current AC mode
}
void daikinAcSetModeUI() {
  daikinAcSetMode();
  daikinAcUI();
}

// Select Daikin AC fan mode
void daikinAcSetFan() {
  // Update fan index with encoder input, and limit available range in Fan mode (2-4)
  if (daikinSetModeIndex == 2)
    inputEncoder(daikinSetFanIndex, 2, 4);  // Constrain the fan index to Min (2) to Max (4) in Fan mode
  else
    inputEncoder(daikinSetFanIndex, 0, 5);  // In other modes, the full range of fan speeds (0-5) is allowed
}
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