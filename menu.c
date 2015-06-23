#include "menu.h"
#include "oledControl.h"

uint8_t totOptions, arrowOnLine, curTopOptionIdx;

void showArrow(uint8_t boolean) {
    //FIXME: Get proper font file and use real arrow codes
    //Arrow on the left
    oledSetCursor(0,arrowOnLine);
    if (boolean) { putChar(26,0); }
    else { putChar(28,0); } //FIXME: This should be a space " " but font file doesn't implement it yet
}

void menuUp(void) {
    //TODO: Handle lists larger than the screen has space for
    showArrow(0);
    if (--arrowOnLine < 2) { arrowOnLine = totOptions+1; }
    showArrow(1);
}

void menuDn(void) {
    //TODO: Handle lists larger than the screen has space for
    showArrow(0);
    if (++arrowOnLine > totOptions+1) { arrowOnLine = 2; }
    showArrow(1);
}

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
    showArrow(1);
}

