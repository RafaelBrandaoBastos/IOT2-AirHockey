#include "display_utils.h"
#include <Arduino.h>

#define CLK 26
#define DIO 27

TM1637Display display(CLK, DIO);

const uint8_t celsius[] = {
    SEG_A | SEG_B | SEG_F | SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F};

const uint8_t done[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    SEG_C | SEG_E | SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G};

void initDisplay()
{
    display.clear();
    display.setBrightness(7);
}

void showCounter()
{
    for (int i = 0; i < 10; i++)
    {
        display.showNumberDec(i);
        delay(500);
        display.clear();
    }
}

void showNegativeNumbers()
{
    display.showNumberDec(-91);
    delay(2000);
    display.clear();

    display.showNumberDec(-109);
    delay(2000);
    display.clear();
}

void showLeadingZeros()
{
    display.showNumberDec(21, false);
    delay(2000);
    display.clear();

    display.showNumberDec(21, true);
    delay(2000);
    display.clear();
}

void showPartialDigits()
{
    display.showNumberDec(28, false, 2, 1);
    delay(2000);
    display.clear();

    display.showNumberDec(-9, false, 3, 0);
    delay(2000);
    display.clear();
}

void showTime()
{
    display.showNumberDecEx(1530, 0b11100000, false, 4, 0);
    delay(2000);
    display.clear();
}

void showTemperature()
{
    int temperature = 23;
    display.showNumberDec(temperature, false, 2, 0);
    display.setSegments(celsius, 2, 2);
    delay(2000);
    display.clear();
}

void showDone()
{
    display.setSegments(done);
    delay(2000);
    display.clear();
}
