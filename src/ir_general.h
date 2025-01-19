#ifndef IR_GENERAL_H
#define IR_GENERAL_H

#include <stdint.h>

extern const uint8_t IR_LED;

// The RC6 protocol benefits from using a struct to manage command codes and their toggle states.
// The toggle flag tracks whether the command's toggle bit has been toggled, ensuring proper protocol handling.
// But declaration of command need to be in the source file to avoid error.
// All the command then use extern the share instances across files.
struct RC6Command {
  uint64_t code;
  bool toggle;  // Tracks the toggle state for the command

  // Constructor to initialize the command code with an optional default toggle state (default: false)
  RC6Command(uint64_t c, bool t = false) : code(c), toggle(t) {}
};

// Astro TV RC6 36 bits
extern RC6Command tvAstroPowerToggle;
extern RC6Command tvAstroButtonBack;
extern RC6Command tvAstroChannelUp;
extern RC6Command tvAstroChannelDown;
extern RC6Command tvAstroButtonOne;
extern RC6Command tvAstroButtonTwo;
extern RC6Command tvAstroButtonThree;
extern RC6Command tvAstroButtonFour;
extern RC6Command tvAstroButtonFive;
extern RC6Command tvAstroButtonSix;
extern RC6Command tvAstroButtonSeven;
extern RC6Command tvAstroButtonEight;
extern RC6Command tvAstroButtonNine;
extern RC6Command tvAstroButtonZero;

// Deka Fan Symphony 12 bits
const uint32_t fanDekaPowerOff = 0xD80;
const uint32_t fanDekaSpeedOne = 0xD88;
const uint32_t fanDekaSpeedTwo = 0xDC6;
const uint32_t fanDekaSpeed0Three = 0xD82;

// IR/4S-FFT Fan Remote Symphony 12 bits
const uint64_t fanFFTPowerOff = 0xC05;
const uint64_t fanFFTSpeedOne = 0xC04;
const uint64_t fanFFTSpeedTwo = 0xC43;
const uint64_t fanFFTSpeedThree = 0xC10;
const uint64_t fanFFTSpeedFour = 0xC01;

// LG TV NEC 32 bits
const uint64_t tvLGPowerToggle = 0x20DF10EF;
const uint64_t tvLGVolumeUp = 0x20DF40BF;
const uint64_t tvLGVolumeDown = 0x20DFC03F;
const uint64_t tvLGVolumeMute = 0x20DF906F;

void initIrGeneral();  // Put this in the void setup in the main.cpp
void sendLGTV(uint64_t command);  // irSend LG TV
void sendDekaFan(uint32_t command);  // irSend Deka fan
void sendFFTFan(uint32_t command);  // irSend FFT fan
void sendAstroTv(RC6Command &command);  // irSend astro (Satellite TV) decoder

#endif