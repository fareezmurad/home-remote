#ifndef UTILS_H
#define UTILS_H

#include <U8g2lib.h>

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

extern const char *version;

void exitToSleep();
void displayInfo();
void displayQr();
int fpsCounter();
void underDevelopment();

#endif