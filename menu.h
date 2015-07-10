#ifndef _MENU_H
#define _MENU_H   1

#include <avr/io.h>

extern uint8_t optionIndex;
extern uint8_t curMenu;

/*
typedef struct MenuTAG {
    uint8_t x;          //Position on the game surface, 0 is left
    uint8_t y;          //Position on the game surface, 0 is top
    uint8_t tarX;       //Target X coord. for enemy
    uint8_t tarY;       //Target Y coord. for enemy
    int16_t speed;      //Countdown how freqeuntly to move
    uint8_t travelDir;  //Uses directional defines below
    uint8_t color;      //Uses color defines below
    uint8_t inPlay;     //On the hunt = TRUE, in reserve = FALSE
    uint8_t dotCount;   //For player tracks level completion
                        //For enemy decides when to go inPlay
    uint8_t dotLimit;   //How many dots before this enemy is inPlay
    uint8_t speedMode;  //Index used to look up player speed
    uint8_t title[140];         //Index used to find stored values
} Menu;
*/

#define MENUCOUNT   3
#define MENUCANCEL  1

/*
uint8_t menuTitles[MENUCOUNT][16] = {
    { "Placeholder\0" },
    { "Cancel Message?\0" },
    { "Send Message?\0" }
};

//How many options does each menu have?
uint8_t menuOptionsCount[MENUCOUNT] = { 0, 2, 5 };

//Arrays to hold options list for each menu
uint8_t menuCancelOptions[2][4] = {
    { "No\0" },
    { "Yes\0" },
};
//Pointers to the menu option arrays
//This can be stored in external EEPROM once that's added
//uint8_t *
*/

void initMenu(void);
void showArrow(uint8_t boolean);
void drawDivider(uint8_t page);
void showMenu(uint8_t defaultOption, char *titleString);
void putOption(uint8_t lineNum, char *optionString);
void menuDn(void);
void menuUp(void);
void menuAction(void);
void homeScreen(void);
void compose(void);
void cancelMsg(void);
void sendMsg(void);
void confirmSend(void);
void confirmCancel(void);
void msgList(void);
void msgDsp(void);

#endif
