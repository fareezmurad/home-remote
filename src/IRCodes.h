#ifndef IRCODES_H
#define IRCODES_H

#include <U8g2lib.h>
#include <stdint.h>

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
extern int encoderCurrentRead;
extern int encoderLastRead;

// Structure for storing IR command details
struct SymphonyCode {
  uint16_t code;  // IR command code
  uint8_t bits = 0;  // Bit length of the code
  uint8_t repeats = 0;  // Number of repetitions

  SymphonyCode(uint16_t c, uint8_t b = 0, uint8_t r = 0)
    : code(c), bits(b), repeats(r) {}
};

// void IRsend::sendRC6(const uint64_t data, const uint16_t nbits, const uint16_t repeat)
struct RC6Code {
  uint64_t code;
  uint16_t bits;
  uint16_t repeat;
};

// Initializes the IR sender
void initIrSend();

// Sends IR command to control Deka fan speed
void dekaSpeedControl(int index);

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
