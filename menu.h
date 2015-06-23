#ifndef _MENU_H
#define _MENU_H   1

#include <avr/io.h>

extern uint8_t optionIndex;

void showArrow(uint8_t boolean);
void drawDivider(uint8_t page);
void showMenu(uint8_t defaultOption);

#endif
