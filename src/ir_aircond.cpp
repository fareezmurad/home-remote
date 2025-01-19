#include "ir_aircond.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>
#include <ir_Sharp.h>

// Pin configuration for IR LED
const uint8_t IR_LED = 17;

// Instances for sending IR signals
IRSharpAc sharpAc(IR_LED);
IRDaikin64 daikinAc(IR_LED);

// Function to initialize the IR sender
void initIrAirConditioner() {
  sharpAc.begin();
  daikinAc.begin();
}

/* ------------------- General variables and functions for AC control ------------------ */
bool irSignalSent = true;  // Tracks wether IR signal has been sent
unsigned long lastInputTime = 0;  // Records timestamp of last encoder input
const unsigned long inactivityDuration = 2000;  // Duration (ms) before sending IR automatically

// Resets the inactivity timer and IR sent flag on new input
void resetTimer() {
  lastInputTime = millis();
  irSignalSent = false;
}

// Adjusts encoder-controlled value within specified range
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

// Toggles a boolean state using the encoder input
void toggleEncoder(bool& state) {
  if (encoderCurrentRead > encoderLastRead || encoderLastRead > encoderCurrentRead) {
    state = !state;  // Toggle the state
    resetTimer();
  }
}

// Bitmap for Celsius temperature indicator (used in UI)
static const unsigned char celcius_bits[] U8X8_PROGMEM = {
  0x38, 0x00, 0x44, 0x40, 0xd4, 0xa0, 0x54, 0x40, 0xd4, 0x1c, 0x54,
  0x06, 0xd4, 0x02, 0x54, 0x02, 0x54, 0x06, 0x92, 0x1c, 0x39, 0x01,
  0x75, 0x01, 0x7d, 0x01, 0x39, 0x01, 0x82, 0x00, 0x7c, 0x00};

/*-------------------------SHARP AIR-CONDITIONER-------------------------*/
bool currentPowerState = false;  // Tracks the power state of the Sharp AC

uint8_t sharpSetTemp = 20;  // Default temperature setting

uint8_t sharpSetModeIndex = 0;  // Initial AC mode index
const uint8_t sharpSetMode[3] = {kSharpAcFan, kSharpAcDry, kSharpAcCool};
const char* sharpSetModeLabel[3] = {"Auto", "Dry", "Cool"};
uint8_t sharpSetFanIndex = 0;  // Initial fan speed index
const uint8_t sharpSetFan[4] = {kSharpAcFanAuto, kSharpAcFanMin, kSharpAcFanMed, kSharpAcFanMax};
const char* sharpSetFanLabel[4] = {"Auto", "Min", "Med", "Max"};

bool sharpSetSwing = true;  // Initial swing state
const char* sharpGetSwingString(bool swing) {
  return swing ? "On" : "Off";  // Returns "On" or "Off" as a string based on the swing state
}

// Ensures fan speed is reset to "Auto" when the AC mode changes
void sharpValidateFanSetting() {
  static uint8_t lastModeIndex = 0;
  if (sharpSetModeIndex != lastModeIndex) sharpSetFanIndex = 0;
  lastModeIndex = sharpSetModeIndex;
}

// Renders the settings on the OLED display
void sharpAcUI() {
  char tempStr[4];  // Buffer to hold temperature as a string
  sprintf(tempStr, "%d", sharpSetTemp);  // Convert temperature to string

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);
  u8g2.setFont(u8g2_font_profont29_tr);
  // sharpSetModeIndex: 0 = Auto, 1 = Dry
  u8g2.drawStr(2, 41, sharpSetModeIndex == 0 || sharpSetModeIndex == 1 ? "--" : tempStr);
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

  sharpAcChkInactivity();  // Automatically sends IR signal after inactivity
}

// Updates AC settings
void sharpAcSetting() {
  sharpAc.setTemp(sharpSetTemp);
  sharpAc.setFan(sharpSetFan[sharpSetFanIndex]);
  sharpAc.setMode(sharpSetMode[sharpSetModeIndex]);
  sharpAc.setSwingToggle(sharpSetSwing);
}

// Toggles power of the AC
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

// Automatically sends IR signal if there's no input for a set duration
void sharpAcChkInactivity() {
  if (!irSignalSent && millis() - lastInputTime >= inactivityDuration) {
    sharpAcSetting();
    sharpAc.send();
    irSignalSent = true;
  }
}

// Set temperature within valid range (16-30°C). Only for cool mode
void sharpAcSetTemp() {
  if (sharpSetModeIndex == 2) inputEncoder(sharpSetTemp, 16, 30);  //  2 = cool
}
void sharpAcSetTempUI() {
  sharpAcSetTemp();
  sharpAcUI();
}

// Set fan speed based on current AC mode. Only for cool mode
void sharpAcSetFan() {
  if (sharpSetModeIndex == 2) inputEncoder(sharpSetFanIndex, 0, 3);  // 2 = cool
}
void sharpAcSetFanUI() {
  sharpAcSetFan();
  sharpAcUI();
}

// Set mode within valid range (index 0-2)
void sharpAcSetMode() {
  inputEncoder(sharpSetModeIndex, 0, 2);
  sharpValidateFanSetting();
}
void sharpAcSetModeUI() {
  sharpAcSetMode();
  sharpAcUI();
}

// Toggle swing state (on/off)
void sharpAcSetSwing() { toggleEncoder(sharpSetSwing); }
void sharpAcSetSwingUI() {
  sharpAcSetSwing();
  sharpAcUI();
}

/*-------------------------DAIKIN AIR-CONDITIONER-------------------------*/
uint8_t daikinSetTemp = 20;  // Default temperature setting

uint8_t daikinSetModeIndex = 2;  // Initial AC mode index
const uint8_t daikinSetMode[3] = {kDaikin64Fan, kDaikin64Dry, kDaikin64Cool};
const char* daikinSetModeLabel[3] = {"Fan", "Dry", "Cool"};
uint8_t daikinSetFanIndex = 1;  // Initial fan speed index
const uint8_t daikinSetFan[6] = {kDaikin64FanQuiet, kDaikin64FanAuto, kDaikin64FanLow, kDaikin64FanMed, kDaikin64FanHigh, kDaikin64FanTurbo};
const char* daikinSetFanLabel[6] = {"Quiet", "Auto", "Min", "Med", "Max", "Turbo"};

bool daikinSetSwing = true;  // Initial swing state
const char* daikinGetSwingString(bool swing) {
  return swing ? "On" : "Off";  // Returns "On" or "Off" as a string based on the swing state
}

// Ensures fan speed is set to correct setting when in fan mode
void daikinValidateFanSetting() {
  static uint8_t lastModeIndex = 255;
  if (daikinSetModeIndex == 0 && lastModeIndex != 0 && (daikinSetFanIndex < 2 || daikinSetFanIndex > 4)) daikinSetFanIndex = 2;
  lastModeIndex = daikinSetModeIndex;
}

// Renders the settings on the OLED display
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

  daikinAcChkInactivity();  // Automatically sends IR signal after inactivity
}

// Update AC setting
void daikinAcSetting() {
  daikinAc.setTemp(daikinSetTemp);
  daikinAc.setFan(daikinSetFan[daikinSetFanIndex]);
  daikinAc.setMode(daikinSetMode[daikinSetModeIndex]);
  daikinAc.setSwingVertical(daikinSetSwing);
}

// Toggles power of the AC
void daikinAcPowerToggle() {
  daikinAcSetting();
  daikinAc.setPowerToggle(true);
  daikinAc.send();
}

// Automatically sends IR signal if there's no input for a set duration
void daikinAcChkInactivity() {
  if (!irSignalSent && millis() - lastInputTime >= inactivityDuration) {
    daikinAcSetting();
    daikinAc.setPowerToggle(false);
    daikinAc.send();
    irSignalSent = true;
  }
}

// Set temperature within valid range (16-30°C)
void daikinAcSetTemp() { inputEncoder(daikinSetTemp, 16, 30); }
void daikinAcSetTempUI() {
  daikinAcSetTemp();
  daikinAcUI();
}

// Set fan speed based on current AC mode
void daikinAcSetFan() {
  if (daikinSetModeIndex == 0)
    inputEncoder(daikinSetFanIndex, 2, 4);  // Constrain the fan index to Min (2) to Max (4) in Fan mode
  else
    inputEncoder(daikinSetFanIndex, 0, 5);  // In other modes, the full range of fan speeds (0-5) is allowed
}
void daikinAcSetFanUI() {
  daikinAcSetFan();
  daikinAcUI();
}

// Set mode within valid range (index 0-2)
void daikinAcSetMode() {
  inputEncoder(daikinSetModeIndex, 0, 2);
  daikinValidateFanSetting();
}
void daikinAcSetModeUI() {
  daikinAcSetMode();
  daikinAcUI();
}

// Toggle swing state (on/off)
void daikinAcSetSwing() { toggleEncoder(daikinSetSwing); }
void daikinAcSetSwingUI() {
  daikinAcSetSwing();
  daikinAcUI();
}