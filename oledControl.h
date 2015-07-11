#include <avr/io.h>
#include "i2cmaster.h"

//Screen Parameters
#define SCREENX 128 //Screen width
#define SCREENY 64  //Screen height

#define CHARWID 6   //Includes space after letter
#define CHARPERLINE SCREENX/CHARWID     //Max chars per line
#define HIGHLIGHTCHAR CHARPERLINE/2     //Find center(ish) char

#define OLED_ADDRESS					0x78

#define OLED_CMD_DISPLAY_OFF			0xAE
#define OLED_CMD_DISPLAY_ON			    0xAF
#define OLED_CMD_NORMAL_DISPLAY		    0xA6
#define OLED_CMD_INVERSE_DISPLAY		0xA7

#define OLED_COMMAND_MODE				0x80
#define OLED_DATA_MODE				    0x40

#define PAGE_ADDRESSING		        	0x02

uint8_t cursX;
uint8_t cursY;

/*********** Prototypes **********************************************/
extern void oledInit(void);
extern void oledWriteCmd(uint8_t cmd);
extern void oledWriteData(uint8_t data);
extern void oledSetCursor(uint8_t col, uint8_t page);
extern void oledClearScreen(uint8_t black);
extern void putChar(uint8_t charIdx, uint8_t inverted);
char getFont(uint8_t charIdx, uint8_t column);
extern void putString(int16_t x, int16_t y, char *msg, uint8_t inverted);
extern void advanceCursor(uint8_t size);
extern void showCharList(uint8_t startChar, uint8_t maxChar, uint8_t line);
extern uint8_t decCharIdx(uint8_t startChar, uint8_t maxChar);
extern uint8_t incCharIdx(uint8_t startChar, uint8_t maxChar);
extern void showHighlighted(uint8_t x, uint8_t y);
extern uint8_t findHighlighted(uint8_t curStart, uint8_t maxChar);
extern uint8_t embiggen(uint8_t charSlice, uint8_t bottom);
void putDblChar(uint8_t x, uint8_t y, uint8_t charIdx);
