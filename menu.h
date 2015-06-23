#ifndef _MENU_H
#define _MENU_H   1

#include <avr/io.h>

extern uint8_t optionIndex;

extern void menuCancel(uint8_t message, uint8_t selected);

#endif
