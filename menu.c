#include "menu.h"
#include "oledControl.h"

uint8_t totOptions, arrowOnLine, curTopOptionIdx;

void showArrow(void) {
    //FIXME: Get proper font file and use real arrow codes
    //Arrow on the left
    oledSetCursor(0,arrowOnLine);
    putChar(26,0);
}

/*
void menuCancel(uint8_t message, uint8_t selected) {
    oledClearScreen(1);
    //Cancel Message?
    putString(getCenter(15),2,"CANCEL MESSAGE?\0",0);
    //No
    putString(getCenter(2),4, "NO\0",0);
    //Yes
    putString(getCenter(3),5, "YES\0", 0);

    selX1 = getCenter(7);
    selX2 = 127-selX1;
    selY = 4;
    showArrows();
}
*/

/* Better universal menu design:

Menu Title
--divider--
optionLine0
optionLine1
optionLine2
optionLine3
optionLine4
optionLine5

indexes: whichMenu, totOptions, arrowOnLine, curTopOptionIdx

*/

void drawDivider(uint8_t page) {
    oledSetCursor(0,page);
    for (uint8_t i=0; i<128; i++) { oledWriteData(0x3C); }
}

void showMenu(uint8_t defaultOption) {
    oledClearScreen(1);
    
    //TODO: All this stuff should be set programmatically.
    //TODO: Typdef a data type to hold: title, number of options, option text
    //Cancel Message?
    putString(0,0,"CANCEL MESSAGE?\0",0);
    drawDivider(1);
    //No
    putString(12,2, "NO\0",0);
    //Yes
    putString(12,3, "YES\0", 0);

    totOptions = 2;
    arrowOnLine = 2+defaultOption;
    curTopOptionIdx = 0;    //Which option from the option array is currently in optionLine0
    showArrow();
}

