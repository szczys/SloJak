/********************************
* Driving shift registers using	*
* an AVR chip's hardwarre SPI	*
*				*
* http://jumptuck.com 		*
*				*
********************************/

#define F_CPU 1000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "i2cmaster.h"
#include "font.h"

/************** Setup a rotary encoder********************/
/* Atmega168 */
/* encoder port */
#define ENC_CTL	DDRB	//encoder port control
#define ENC_WR	PORTB	//encoder port write	
#define ENC_RD	PINB	//encoder port read
#define ENC_A 6
#define ENC_B 7

volatile int8_t selected_option = 0;
/*********************************************************/

//Display
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

uint8_t message[140] = "HELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLD\0";

uint8_t cursX = 1;
uint8_t cursY = 0;

uint8_t charListStart = 3;
#define CHARSETLEN  26  //How many characters does our fontfile have?

//Send send address, send command or data mode, send data, send stop

void init_IO(void){
  //Setup
    DDRB |= (1<<PB0) | (1<<PB2);	    //Set control pins as outputs
    PORTB &= ~(1<<PB0 | 1<<PB2);     //Set control pins low

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

/*
curStart = which char is first on the row
maxChar = total number of chars in set
returns: which char idx is highlighted onscreen
*/
uint8_t findHighlighted(uint8_t curStart, uint8_t maxChar) {
    //NOTE: beware overflows
    uint8_t newIdx = curStart+HIGHLIGHTCHAR;
    if (newIdx >= maxChar) { newIdx -= maxChar;}
    return newIdx;
}

uint8_t embiggen(uint8_t charSlice, uint8_t bottom) {
    if (bottom) { charSlice = charSlice>>4; }
    uint8_t returnByte = 0x00;
    for (uint8_t i=0; i<8; i++) {
    /*
        old new
        0   0
        0   1
        1   2
        1   3
        2   4
        2   5
        3   6
        3   7
    */
        if (charSlice & (1<<(i/2))) { returnByte |= (1<<i); }
    }
    return returnByte;
}

void putDblChar(uint8_t x, uint8_t y, uint8_t charIdx) {
    for (uint8_t row=0; row<2; row++) {
        for (uint8_t col = 0; col < 5; col++) {
            uint8_t embiggened = embiggen(font5x7[(charIdx*5)+col],row);
            oledSetCursor(x+(2*col),y+row);
            oledWriteData(embiggened);
            oledWriteData(embiggened);
        }
        oledWriteData(0x00);    //Space after each letter
        oledWriteData(0x00);    //Space after each letter
    }
}

int main(void)
{

    //setup pin change interrupt
    PCICR |= 1<<PCIE0;      //enable PCINT0_vect  (PCINT0..7 pins)
    PCMSK0 |= 1<<PCINT6;    //interrupt on PCINT6 pin
    

    init_IO();
    
    //setup pin change interrupt
    PCICR |= 1<<PCIE0;      //enable PCINT0_vect  (PCINT0..7 pins)
    PCMSK0 |= 1<<PCINT6;    //interrupt on PCINT6 pin
    PCMSK0 |= 1<<PCINT7;    //interrupt on PCINT7 pin
    sei();
    
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

/*
    for (uint8_t i=0; i<8; i++) {
        oledSetCursor(0, i);
        oledWriteData(0xFF);
        oledSetCursor(127, i);
        oledWriteData(0xFF);
    }
*/
    oledSetCursor(cursX, cursY);
    putChar(1);
    advanceCursor(6);

    putDblChar(10,2,5);
    putDblChar(22,2,20);
    putDblChar(34,2,13);
    //putString(120,2, (uint8_t *)&message);

    //show which letter will be selected
    showHighlighted(HIGHLIGHTCHAR*CHARWID+1,6);  //21 charperline on 128px display plus 1 pixel for centering
    showCharList(charListStart,CHARSETLEN,7);

    while(1)
    {
        if (selected_option) {
            if (selected_option > 0) {
                incSelOpt();
                slideAlphaLeft();
            }
            else {
                decSelOpt();
                slideAlphaRight();
            }
            selected_option = 0;
        }

    //wait for a little bit before repeating everything
    /*
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
        putChar(findHighlighted(charListStart,CHARSETLEN));
        advanceCursor(6);
        //PORTB |= (1<<PB0);
    }
    */
    //PINB = (1<<PB0);    //Toggle the ouut
  }
}

void incSelOpt(void) {
    PORTB &= ~(1<<PB0);
    PORTB |= 1<<PB2;
}

void decSelOpt(void) {
    PORTB &= ~(1<<PB2);
    PORTB |= 1<<PB0;
}

void slideAlphaLeft(void) {
    charListStart = decCharIdx(charListStart,CHARSETLEN);
    showCharList(charListStart,CHARSETLEN,7);
}

void slideAlphaRight(void) {
    charListStart = incCharIdx(charListStart,CHARSETLEN);
    showCharList(charListStart,CHARSETLEN,7);
}

ISR(PCINT0_vect) {
  static uint8_t old_AB = 3;  //lookup table index
  static int8_t encval = 0;   //encoder value  
  static const int8_t enc_states [] PROGMEM = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};  //encoder lookup table
  /**/
  old_AB <<=2;  //remember previous state
  old_AB |= ((ENC_RD>>6) & 0x03 ); //Shift magic to get PB6 and PB7 to LSB
  encval += pgm_read_byte(&(enc_states[( old_AB & 0x0f )]));
  /* post "Navigation forward/reverse" event */
  if( encval < -3 ) {  //four steps forward
    selected_option = -1;
    encval = 0;
  }
  else if( encval > 3  ) {  //four steps backwards
    selected_option = 1;
    encval = 0;
  }
}
