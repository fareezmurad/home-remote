#include "ir_general.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>

// Instances for sending IR signals
IRsend irSend(IR_LED);

// Initialize the IR Sender
void initIrGeneral() {
  irSend.begin();
}

/*================================RC6 PROTOCOL==============================*/
void sendRC6(RC6Command& command, uint16_t nbits, uint16_t repeat = 0) {
  uint64_t toggleBitCode = command.code;  // Default to the original code
  uint8_t shift = (nbits == 36) ? 15 : (nbits == 20) ? 16 : 0;

  if (shift > 0) {
    // Toggle the appropriate bit for the RC6 protocol:
    // - 36-bit RC6: Toggle the 16th LSB (bit 15, zero-indexed)
    // - 20-bit RC6: Toggle the 17th LSB (bit 16, zero-indexed)
    toggleBitCode ^= (command.toggle << shift);
    command.toggle = !command.toggle;  // Flip the toggle state
  }

  irSend.sendRC6(toggleBitCode, nbits, repeat);
}

// Astro TV RC6 36 bits
// Need to be in the source file because of using struct
// to keep track the toggle state using toggle flag
RC6Command tvAstroPowerToggle = {0xC8056A70C};
RC6Command tvAstroButtonBack = {0xC805627A9};
RC6Command tvAstroChannelUp = {0xC8056A720};
RC6Command tvAstroChannelDown = {0xC8056A721};
RC6Command tvAstroButtonOne = {0xC80562701};
RC6Command tvAstroButtonTwo = {0xC80562702};
RC6Command tvAstroButtonThree = {0xC80562703};
RC6Command tvAstroButtonFour = {0xC80562704};
RC6Command tvAstroButtonFive = {0xC80562705};
RC6Command tvAstroButtonSix = {0xC80562706};
RC6Command tvAstroButtonSeven = {0xC80562707};
RC6Command tvAstroButtonEight = {0xC80562708};
RC6Command tvAstroButtonNine = {0xC80562709};
RC6Command tvAstroButtonZero = {0xC80562700};

void sendAstroTv(RC6Command& command) { sendRC6(command, 36, 1); }

/*==============================NEC PROTOCOL===========================*/
void sendNEC(uint64_t command, uint16_t nbits, uint16_t repeat = 0) {
  irSend.sendNEC(command, nbits, repeat);
}

void sendLGTV(uint64_t command) { sendNEC(command, 32); }

/*===========================SYMPHONY PROTOCOL=========================*/
void sendSymphony(uint32_t command, uint16_t nbits, uint16_t repeat = 0) {
  irSend.sendSymphony(command, nbits, repeat);
}

void sendDekaFan(uint32_t command) { sendSymphony(command, 12, 1); }

void sendFFTFan(uint32_t command) { sendSymphony(command, 12, 1); }