#ifndef _MENU_H
#define _MENU_H   1

#include <avr/io.h>
#include <avr/pgmspace.h>

extern uint8_t optionIndex;
extern uint8_t curMenu;

/**************** Function pointers for menu items ****************/
extern void (*knobLeft)(void);
extern void (*knobRight)(void);
extern void (*doBack)(void);
extern void (*doSelect[6])(void);

/**************** Prototypes **************************************/
void initMenu(void);
void knobNavigatesList(void);
void knobScrollsAlphabet(void);
void showArrow(uint8_t boolean);
void drawDivider(uint8_t page);
void showMenu(uint8_t defaultOption, char *titleString);
void putOption(uint8_t lineNum, char *optionString);
void menuDn(void);
void menuUp(void);
void homeScreen(void);
void compose(void);
void selectChar(void);
void sendMsg(void);
void cancelMsg(void);
void clearMsgAndReturn(void);
void confirmSend(void);
void confirmCancel(void);
void msgList(void);
void msgDsp(void);
void slideAlphaLeft(void);
void slideAlphaRight(void);

#endif
