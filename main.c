/********************************
* Driving shift registers using	*
* an AVR chip's hardwarre SPI	*
*				*
* http://jumptuck.com 		*
*				*
********************************/

#define F_CPU 1000000

#include <avr/io.h>
#include <util/delay.h>

#include "i2cmaster.h"
#include "font.h"

#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define DATA (1<<PB3)		//MOSI (SI)
#define LATCH (1<<PB2)		//SS   (RCK)
#define CLOCK (1<<PB5)		//SCK  (SCK)

#define BUT_SEL     (1<<PB6)
#define BUT_LEFT    (1<<PB1)
#define BUT_RIGHT   (1<<PB7)

//Screen Parameters
#define SCREENX 128 //Screen width
#define SCREENY 64  //Screen height

#define CHARWID 6   //Includes space after letter
#define CHARPERLINE SCREENX/CHARWID

#define OLED_ADDRESS					0x78

#define OLED_CMD_DISPLAY_OFF			0xAE
#define OLED_CMD_DISPLAY_ON			    0xAF
#define OLED_CMD_NORMAL_DISPLAY		    0xA6
#define OLED_CMD_INVERSE_DISPLAY		0xA7

#define OLED_COMMAND_MODE				0x80
#define OLED_DATA_MODE				    0x40

#define PAGE_ADDRESSING		        	0x02

uint8_t message[140] = "HELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLD\0";

uint8_t cursX = 4;
uint8_t cursY = 3;

uint8_t charListStart = 3;
#define CHARSETLEN  26  //How many characters does our fontfile have?

//Send send address, send command or data mode, send data, send stop

void init_IO(void){
  //Setup
    DDRB |= (1<<PB0);	    //Set control pins as outputs
    PORTB &= ~(1<<PB0);     //Set control pins low

    DDRB &= ~(BUT_LEFT | BUT_RIGHT | BUT_SEL);      //Set as input
    PORTB |= BUT_LEFT | BUT_RIGHT | BUT_SEL;      //Enable pull-up
}

void oledWriteCmd(uint8_t cmd) {
    i2c_start_wait(OLED_ADDRESS+I2C_WRITE);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(cmd);
    i2c_stop();
}

void oledWriteData(uint8_t data) {
    i2c_start_wait(OLED_ADDRESS+I2C_WRITE);
    i2c_write(OLED_DATA_MODE);
    i2c_write(data);
    i2c_stop();
}

void oledSetCursor(uint8_t col, uint8_t page) {
    col += 2;   //Display off by 2 columns for some reason
    //Page
    oledWriteCmd(0xB0 + page);
    //Column
    oledWriteCmd(0x0F & col);
    oledWriteCmd(0x10 + (col>>4));
}

void oledClearScreen(uint8_t black) {
    uint8_t value = 0xFF;
    if (black) { value = 0x00; }
    for (uint8_t page=0; page<8; page++) {
        oledWriteCmd(0xB0 + page);
        oledWriteCmd(0x00);
        oledWriteCmd(0x10);
        i2c_stop();

        i2c_start_wait(OLED_ADDRESS);
        i2c_write(OLED_DATA_MODE);
        for (uint8_t col=0; col<128; col++) {
            oledWriteData(value);
        }
        i2c_stop();
    }
}

void putChar(uint8_t charIdx) {
    for (uint8_t col = 0; col < 5; col++) {
        oledWriteData(font5x7[(charIdx*5)+col]);
    }
    oledWriteData(0x00);    //Space after each letter
}

void putString(int16_t x, int16_t y, uint8_t *msg) {
    const uint8_t charWidth = 6;
    const uint8_t charHeight = 1;

    //Find str length
    uint8_t i;
    for (i=0; i<141; i++) {
        if (msg[i] == 0) {
            if (i == 0) { return; } //zero len string
            else { break; }
        }
    }
    uint16_t colPosition;
    uint16_t rowPosition;
    for (uint8_t j=0; j<i; j++) {
        printf("%d\n", msg[j]);
        colPosition = x+(charWidth*j);
        rowPosition = y;

        //Linewrap
        if (colPosition+charWidth >= SCREENX) {
            uint8_t charPerRow = (SCREENX/charWidth);
            uint8_t charPos = j-((SCREENX-x)/charWidth);

            colPosition = (charPos%charPerRow)*charWidth;
            rowPosition = y+charHeight + (charHeight*(charPos/charPerRow));
        }
        oledSetCursor(colPosition, rowPosition);
        putChar(msg[j]-65);
    }
}

void advanceCursor(uint8_t size) {
    cursX += size;
    if (cursX > SCREENX-size) {
        cursX = 0;
        if (++cursY >= SCREENY) { cursY = 0; }
    }
}

void showCharList(uint8_t startChar, uint8_t maxChar, uint8_t line) {
    for (uint8_t i=0; i<CHARPERLINE; i++) {
        if (startChar>=maxChar) { startChar = 0; }
        oledSetCursor((i*CHARWID)+1,line);  //adding 1 centers on a 128px screen with 6px CHARWID
        putChar(startChar);
        startChar++;
    }
}

/*
Decrements index tracking which letter starts alphabet
startChar is first letter printed on line
maxChar is total number of chars in set
*/
uint8_t decCharIdx(uint8_t startChar, uint8_t maxChar)
{
    if (startChar == 0) { return maxChar-1; }
    return --startChar;
}

/*
Increments index tracking which letter starts alphabet
startChar is first letter printed on line
maxChar is total number of chars in set
*/
uint8_t incCharIdx(uint8_t startChar, uint8_t maxChar)
{
    if (startChar >= maxChar) { return 0; }
    return ++startChar;
}

void showHighlighted(uint8_t x, uint8_t y) {
    oledSetCursor(x,y);
    for (uint8_t i=0; i<5; i++) {
        oledWriteData(0b01000000);
    }
    oledWriteData(0x00);
}

int main(void)
{
    init_IO();
    _delay_ms(200);
    i2c_init();

    oledWriteCmd(OLED_CMD_DISPLAY_OFF);
    oledWriteCmd(0xD5); //clkdiv
    oledWriteCmd(0x80);
    oledWriteCmd(0xA8); //multiplex
    oledWriteCmd(0x3F);
    oledWriteCmd(0xD3); //displayoffet
    oledWriteCmd(0x00);
    oledWriteCmd(0x40); //displayStartLine
    oledWriteCmd(0x8D);
    oledWriteCmd(0x14);
    oledWriteCmd(0x20);
    oledWriteCmd(PAGE_ADDRESSING);
    oledWriteCmd(0xA1); //segremap 0x01
    oledWriteCmd(0xC8); //comscandec
    oledWriteCmd(0xDA);
    oledWriteCmd(0x12);
    oledWriteCmd(0x81);
    oledWriteCmd(0xCF);
    oledWriteCmd(0xD9);
    oledWriteCmd(0xF1);
    oledWriteCmd(0xDB);
    oledWriteCmd(0x40);

    oledWriteCmd(0x26);

    oledWriteCmd(0x00);
    oledWriteCmd(0x05);
    oledWriteCmd(0x00);
    oledWriteCmd(0x07);

    oledWriteCmd(0x2F);
    //oledWriteCmd(0x2E);

    //oledWriteCmd(0x26);
    oledWriteCmd(OLED_CMD_DISPLAY_ON);
    oledWriteCmd(OLED_CMD_NORMAL_DISPLAY);

/*
    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(0x26);
    i2c_write(0x00);
    i2c_write(0x00);
    i2c_write(0x00);
    i2c_write(0x00);
    i2c_write(0x00);
    i2c_write(0xFF);
    i2c_stop();
*/
    oledClearScreen(1);

    for (uint8_t i=0; i<8; i++) {
        oledSetCursor(0, i);
        oledWriteData(0xFF);
        oledSetCursor(127, i);
        oledWriteData(0xFF);
    }

    oledSetCursor(3, 0);
    putChar(1);

    putString(120,2, (uint8_t *)&message);

    //show which letter will be selected
    showHighlighted((6*10)+1,6);  //21 charperline on 128px display plus 1 pixel for centering
    showCharList(charListStart,CHARSETLEN,7);

  while(1)
  {
    //wait for a little bit before repeating everything
    uint8_t readButtons = PINB;
    _delay_ms(40);
    if (~readButtons & BUT_LEFT) {
        charListStart = decCharIdx(charListStart,CHARSETLEN);
        showCharList(charListStart,CHARSETLEN,7);
    }
    if (~readButtons & BUT_RIGHT) {
        charListStart = incCharIdx(charListStart,CHARSETLEN);
        showCharList(charListStart,CHARSETLEN,7);
    }
    if (~readButtons & BUT_SEL) {
        oledSetCursor(cursX, cursY);
        putChar(18);
        advanceCursor(6);
        //PORTB |= (1<<PB0);
    }

    //PINB = (1<<PB0);    //Toggle the ouut
  }
}
