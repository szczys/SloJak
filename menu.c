#include "menu.h"
#include "oledControl.h"

uint8_t menuBuffer[9][128];

void initMenu(void) {
    blankBuffer();
}

void blankBuffer(void) {
    //Zero out menu buffer
    for (uint8_t page=0; page<8; page++) {
        for (uint8_t col=0; col<128; col++) {
            menuBuffer[page][col] = 0x00;
        }
    }
}

void displayBuffer(void) {
    //Push menuBuffer to the oled display
    //Zero out menu buffer
    for (uint8_t page=0; page<8; page++) {
        oledSetCursor(0, page);
        for (uint8_t col=0; col<128; col++) {
            oledWriteData(menuBuffer[col][page]);
        }
    }
}

void menuCancel(uint8_t message, uint8_t selected) {
    blankBuffer();
    menuBuffer[4][10] = 0x55;
    displayBuffer();
}



