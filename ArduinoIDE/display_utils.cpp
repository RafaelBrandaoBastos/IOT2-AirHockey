#include "display_utils.h"
#include <Arduino.h>

// Primeira dupla de pinos
#define CLK1 33
#define DIO1 25

// Segunda dupla de pinos
#define CLK2 26
#define DIO2 27

TM1637Display display1(CLK1, DIO1);
TM1637Display display2(CLK2, DIO2);

void initDisplays()
{
    display1.setBrightness(7);
    display2.setBrightness(7);
    display1.clear();
    display2.clear();
}

void showNumberOnDisplays(int number)
{
    display1.showNumberDec(number, true);
    display2.showNumberDec(number, true);
}
