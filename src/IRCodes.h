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
  uint8_t nbits;  // Bit length of the code
  uint8_t repeat = 2;  // Number of repetitions

  SymphonyCode(uint16_t c, uint8_t b, uint8_t r = 2)
      : code(c), nbits(b), repeat(r) {}
};

// void IRsend::sendRC6(const uint64_t data, const uint16_t nbits, const uint16_t repeat)
struct RC6Code {
  uint64_t code;
  uint16_t nbits;
  uint16_t repeat = 0;
  bool toggle = false;  // Toggle bit state

  RC6Code(uint64_t c, uint16_t b, uint16_t r = 0, bool t = false)
      : code(c), nbits(b), repeat(r), toggle(t) {}
};

// LG TV NEC 32 bits IR Command
const uint64_t tvLGPowerToggle = 0x20DF10EF;
const uint64_t tvLGVolumeUp = 0x20DF40BF;
const uint64_t tvLGVolumeDown = 0x20DFC03F;
const uint64_t tvLGVolumeMute = 0x20DF906F;

void sendLGTV(uint64_t command);

// Initializes the IR sender
void initIrSend();

// Sends IR command to control Deka fan speed
void dekaSpeedControl(uint8_t index);

// Control astro (Satellite TV) decoder
void astroRemote(uint8_t index);

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
