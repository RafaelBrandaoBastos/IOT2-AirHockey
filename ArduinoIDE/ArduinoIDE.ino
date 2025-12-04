#include "display_utils.h"

int contador = 0;

void setup() {
  initDisplay();
}

void loop() {
  display.showNumberDec(contador, true); // always show 4 digits (com zeros à esquerda)
  delay(500);                             // tempo da contagem

  contador++;
  if (contador > 9999) {
    contador = 0; // reinicia
  }
}