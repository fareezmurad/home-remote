#include "IRCodes.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Sharp.h>

// IR LED pin for transmitting signals
const uint8_t IR_LED = 27;

// IR sender instances
IRsend irSend(IR_LED);
IRSharpAc sharpAc(IR_LED);

// Tracks the on/off state of the Sharp AC
bool acState = false;

// Array of SymphonyCode for controlling Deka fan speeds
SymphonyCode fanDeka[] = {
  {0xD80, 12, 3},  // Fan off
  {0xD88, 12, 3},  // Speed 1
  {0xDC6, 12, 3},  // Speed 2
  {0xD82, 12, 3}  // Speed 3
};

// Initializes the IR sender (to be called in setup)
void initIrSend() {
  irSend.begin();
}

// Sends IR command to set Deka fan speed based on index
void dekaSpeedControl(int index) {
  irSend.sendSymphony(fanDeka[index].code, fanDeka[index].bits, fanDeka[index].repeats);
}

// Toggles the Sharp AC between on and off states with preset settings
void sharpAcControl() {
  if (!acState) {
    sharpAc.setMode(kSharpAcCool);
    sharpAc.setTemp(20);  // Set temperature to 20°C
    sharpAc.setFan(kSharpAcFanAuto);  // Set fan to auto mode
    sharpAc.on();  // Turn AC on
  } else {
    sharpAc.off();  // Turn AC off
  }
  sharpAc.send();  // Transmit command
  acState = !acState;  // Toggle AC state
}
