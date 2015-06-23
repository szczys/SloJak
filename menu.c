#include "menu.h"
#include "oledControl.h"

void menuCancel(uint8_t message, uint8_t selected) {
    oledClearScreen(1);
    oledSetCursor(10,4);
    oledWriteData(0x55);
}



