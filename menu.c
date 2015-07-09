#include "menu.h"
#include "oledControl.h"
#include "string.h"

/************************Menu Defines ****************************/
#define COMPOSE         0
#define HOMESCREEN      1
#define CANCELMSG       2
#define SENDMSG         3
#define CONFIRMSEND     4
#define MSGLIST         5
#define MSGDISPLAY      6

uint8_t const menuChoice[7][2] = {
    { COMPOSE, COMPOSE },  //COMPOSE
    { MSGLIST, COMPOSE },       //HOMESCREEN
    { HOMESCREEN, COMPOSE },    //CANCELMSG
    { HOMESCREEN, HOMESCREEN }, //SENDMSG
    { HOMESCREEN, COMPOSE },    //CONFIRMSEND
    { MSGDISPLAY, MSGDISPLAY }, //MSGLIST FIXME: up to 7 options here
    { MSGDISPLAY, MSGDISPLAY }
    };


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

/* TODO Menu Flow
    Each option is linked to a mode
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
    //putString(12,2, "NO\0",0);
    uint8_t tempStr[20];
    strcpy(tempStr, "NO\0");
    putOption(2, tempStr);

    //Yes
    //putString(12,3, "YES\0", 0);
    strcpy(tempStr, "YES\0");
    putOption(3, tempStr);

    totOptions = 2;
    arrowOnLine = 2+defaultOption;
    curTopOptionIdx = 0;    //Which option from the option array is currently in optionLine0
    showArrow(1);
}

void putOption(uint8_t lineNum, uint8_t *optionString)
{
    putString(12,lineNum, optionString, 0);
}

void homeScreen(void)
{
    //Set what back button does
    
}

void compose(void);
void cancelMsg(void);
void sendMsg(void);
void confirmSend(void);
void confirmCancel(void);
void msgList(void);
void msgDsp(void);

