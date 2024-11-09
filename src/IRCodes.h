#ifndef IRCODES_H
#define IRCODES_H

#include <stdint.h>

// Structure for storing IR command details
struct SymphonyCode {
  uint16_t code;  // IR command code
  uint8_t bits;  // Bit length of the code
  uint8_t repeats;  // Number of repetitions
};

// External array of SymphonyCode for controlling Deka fan
extern SymphonyCode fanDeka[];

// Initializes the IR sender
void initIrSend();

// Sends IR command to control Deka fan speed
void dekaSpeedControl(int index);

// Toggles the Sharp AC on/off with specified settings
void sharpAcControl();

#endif  // IRCODES_H
