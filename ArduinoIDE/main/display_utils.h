#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <TM1637Display.h>

// Primeira dupla de pinos
#define CLK1 33
#define DIO1 25

// Segunda dupla de pinos
#define CLK2 26
#define DIO2 27

extern TM1637Display display1;
extern TM1637Display display2;

void initDisplays();
void showNumberOnDisplays(int number);
void showTextOnDisplays(const char* text);

#endif
