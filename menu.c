#include "menu.h"
#include "oledControl.h"
#include "string.h"

uint8_t writeMsgIdx = 0;
#define MAXMSGLEN   127
char writeMsg[MAXMSGLEN] = "\0";

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

/**************** Menu Strings stored in PROGMEM ******************/
const char strTitleHome[] PROGMEM = "Crappy Messager\0";
const char strTitleSend[] PROGMEM = "Send Message?\0";
const char strTitleCancel[] PROGMEM = "Cancel Message?\0";
const char strOptYes[] PROGMEM = "Yes\0";
const char strOptNo[] PROGMEM = "No\0";
const char strOptCompose[] PROGMEM = "Write Message\0";
const char strOptRead[] PROGMEM = "Read Messages\0";
const char strOptSend[] PROGMEM = "Send Messages\0";
const char strOptBackComposer[] PROGMEM = "Edit Message\0";
const char strOptDiscard[] PROGMEM = "Discard Message\0";

//Set initial behavior as compose message
uint8_t curMenu = COMPOSE;
uint8_t optionIndex = 0;
void (*knobLeft)(void) = &slideAlphaLeft;
void (*knobRight)(void) = &slideAlphaRight;
void (*doBack)(void) = &cancelMsg;
void (*doSelect[6])(void) = {
    &selectChar,
    &homeScreen,
    &homeScreen,
    &homeScreen,
    &homeScreen,
    &homeScreen
    };


char tempStr[20];

uint8_t totOptions, arrowOnLine, curTopOptionIdx;

/**************** Compose Window Vars ************************/
uint8_t charListStart = 0;
#define CHARSETLEN  96  //How many characters does our fontfile have?

void initMenu(void)
{
    //TODO: Drawing initial screen should be handled here
}

void knobNavigatesList(void)
{
    knobLeft = &menuUp;
    knobRight = &menuDn;
}

void knobScrollsAlphabet(void)
{
    knobLeft = &slideAlphaLeft;
    knobRight = &slideAlphaRight;
}



void showArrow(uint8_t boolean) {
    //Arrow on the left
    oledSetCursor(0,arrowOnLine);
    if (boolean) { putChar(128,0); }
    else { putChar(32,0); } //FIXME: This should be a space " " but font file doesn't implement it yet
}

void menuUp(void) {
    //TODO: Handle lists larger than the screen has space for
    showArrow(0);   //Erase Arrow
    if (--arrowOnLine < 2) { arrowOnLine = totOptions+1; }
    optionIndex = arrowOnLine-2;
    showArrow(1);   //Draw Arrow
}

void menuDn(void) {
    //TODO: Handle lists larger than the screen has space for
    showArrow(0);
    if (++arrowOnLine > totOptions+1) { arrowOnLine = 2; }
    optionIndex = arrowOnLine-2;
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

void showMenu(uint8_t defaultOption, char *titleString) {
    oledClearScreen(1);

    //Display the menu title
    putString(0,0, titleString,0);
    //Divider between title and options (options added elsewhere)
    drawDivider(1);

    //Draw the arrow for selecting options (options added elsewhere)
    arrowOnLine = 2+defaultOption;
    showArrow(1);
}

void putOption(uint8_t lineNum, char *optionString)
{
    putString(12,lineNum, optionString, 0);
}

void homeScreen(void)
{
    //TODO: Set back button behavior
    knobNavigatesList();    //Setup Knob Behavior
    //Fill the selection function pointer arrays
    optionIndex = 0;
    doSelect[0] = &compose;     //List available messages to read
    doSelect[1] = &msgList;  //Compose message

    strcpy_P(tempStr, strTitleHome);
    showMenu(0, tempStr);

    //No
    strcpy_P(tempStr, strOptCompose);
    putOption(2, tempStr);

    //Yes
    strcpy_P(tempStr, strOptRead);
    putOption(3, tempStr);

    totOptions = 2;
}

void compose(void)
{
    oledClearScreen(1);

    cursX = 0;
    cursY = 0;

    if (writeMsgIdx > 0) {
        putString(cursX, cursY, writeMsg, 0);
        for (uint8_t i=0; i<writeMsgIdx; i++) { advanceCursor(6); }
    }


    doBack = &cancelMsg;    //Set back button behavior
    knobScrollsAlphabet();  //Setup Knob Behavior
    //TODO: Fill the selection function pointer arrays
    optionIndex = 0;
    doSelect[0] = &selectChar;

    //show which letter will be selected
    showHighlighted(HIGHLIGHTCHAR*CHARWID+1,6);  //21 charperline on 128px display plus 1 pixel for centering
    showCharList(charListStart,CHARSETLEN,7);
}

void selectChar(void)
{
    if (writeMsgIdx >= MAXMSGLEN-2) { return; } //Messages have a length limit (plus zero terminator)

    oledSetCursor(cursX, cursY);
    uint8_t selected = findHighlighted(charListStart,CHARSETLEN)+32;

    if (selected == 127) { sendMsg(); }     //Send Icon
    else if (selected == 126) {             //Backspace Icon
        if (writeMsgIdx > 0) {
            if (cursX<6) {
                cursX = 120;
                cursY -= 1;
            }
            else { cursX -= 6; }
            oledSetCursor(cursX, cursY);
            putChar(32, 0);                 //Erase char on screen
            --writeMsgIdx;                  //Decrement index
            writeMsg[writeMsgIdx] = 0;      //Add zero terminator
        }
    }
    else {                                  //Place selected char
        writeMsg[writeMsgIdx] = selected;
        writeMsg[writeMsgIdx+1] = 0;
        putChar(writeMsg[writeMsgIdx], 0);
        ++writeMsgIdx;
        advanceCursor(6);
    }
}

void sendMsg(void)
{
    doBack = &compose;      //Set back button behavior
    knobNavigatesList();    //Setup Knob Behavior
    //Fill the selection function pointer arrays
    optionIndex = 0;
    doSelect[0] = &sendMsg;             //Send
    doSelect[1] = &compose;             //Back
    doSelect[2] = &clearMsgAndReturn;   //Cancel
    totOptions = 3;

    //strcpy(tempStr, "CANCEL MESSAGE?\0");
    strcpy_P(tempStr, strTitleSend);
    showMenu(0, tempStr);

    strcpy_P(tempStr, strOptSend);
    putOption(2, tempStr);
    strcpy_P(tempStr, strOptBackComposer);
    putOption(3, tempStr);
    strcpy_P(tempStr, strOptDiscard);
    putOption(4, tempStr);
}

void cancelMsg(void)
{
    //TODO: Set back button behavior
    knobNavigatesList();    //Setup Knob Behavior
    //TODO: Fill the selection function pointer arrays
    optionIndex = 0;
    doSelect[0] = &compose;    //TODO: if we go back to compose window, the partial composed message should appear
    doSelect[1] = &clearMsgAndReturn;  //Exit to the home screen

    //strcpy(tempStr, "CANCEL MESSAGE?\0");
    strcpy_P(tempStr, strTitleCancel);
    showMenu(0, tempStr);

    //No
    strcpy_P(tempStr, strOptNo);
    putOption(2, tempStr);

    //Yes
    strcpy_P(tempStr, strOptYes);
    putOption(3, tempStr);

    totOptions = 2;
}

void clearMsgAndReturn(void)
{
    writeMsg[0] = 0;
    writeMsgIdx = 0;
    homeScreen();
}

void confirmSend(void);
void confirmCancel(void);
void msgList(void)
{
    //FIXME: Do Something
}
void msgDsp(void);

/************************ Compose Screen Stuff *************************/
void slideAlphaLeft(void) {
    charListStart = decCharIdx(charListStart,CHARSETLEN);
    showCharList(charListStart,CHARSETLEN,7);
}

void slideAlphaRight(void) {
    charListStart = incCharIdx(charListStart,CHARSETLEN);
    showCharList(charListStart,CHARSETLEN,7);
}

