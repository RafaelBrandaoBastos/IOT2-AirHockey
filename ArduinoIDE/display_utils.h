#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <TM1637Display.h>
extern TM1637Display display;

// Inicialização do display será feita no .cpp
void initDisplay();

// Funções de exibição
void showCounter();
void showNegativeNumbers();
void showLeadingZeros();
void showPartialDigits();
void showTime();
void showTemperature();
void showDone();

#endif
