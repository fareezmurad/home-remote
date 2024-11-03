// IRCodes.h
#ifndef IRCODES_H
#define IRCODES_H

#include <stdint.h>

// Define a struct for the Symphony codes
struct SymphonyCode {
  uint16_t code;
  uint8_t bits;
  uint8_t repeats;
};

// Declare the array as an external variable
extern SymphonyCode fanDeka[];

#endif // IRCODES_H
