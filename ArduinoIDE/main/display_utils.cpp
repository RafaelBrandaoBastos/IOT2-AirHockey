#include "display_utils.h"

// Objetos globais
TM1637Display display1(CLK1, DIO1);
TM1637Display display2(CLK2, DIO2);

void initDisplays() {
    display1.setBrightness(7);
    display2.setBrightness(7);
    display1.clear();
    display2.clear();
}

void showNumberOnDisplays(int number) {
    // 0x40 ativa as duas bolinhas no meio
    display1.showNumberDecEx(number, 0x40, true);
    display2.showNumberDecEx(number, 0x40, true);
}

void showTextOnDisplays(const char* text) {
    uint8_t data[4];
    for (int i = 0; i < 4; i++) {
        char c = text[i];
        if (c >= '0' && c <= '9') {
            data[i] = display1.encodeDigit(c - '0');
        } else if (c == '-') {
            data[i] = 0b01000000; // padrão '-' no TM1637
        } else {
            data[i] = 0x00; // espaço em branco
        }
    }
    
    // NÃO ativa as bolinhas quando mostrar texto
    display1.setSegments(data);
    display2.setSegments(data);
}
