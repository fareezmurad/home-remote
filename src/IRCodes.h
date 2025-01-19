#ifndef IRCODES_H
#define IRCODES_H

#include <U8g2lib.h>
#include <stdint.h>

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
extern int encoderCurrentRead;
extern int encoderLastRead;

// Initializes the IR sender
void initIrAirConditioner();

// Command to control Sharp air-conditioner
void sharpAcPowerToggle();
void sharpAcSetTempUI();
void sharpAcSetFanUI();
void sharpAcSetModeUI();
void sharpAcSetSwingUI();
void sharpAcChkInactivity();

// Command to control Daikin air-conditioner
void daikinAcPowerToggle();
void daikinAcSetTempUI();
void daikinAcSetModeUI();
void daikinAcSetFanUI();
void daikinAcSetSwingUI();
void daikinAcChkInactivity();

#endif  // IRCODES
