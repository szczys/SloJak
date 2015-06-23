/********************************
* Driving shift registers using	*
* an AVR chip's hardwarre SPI	*
*				*
* http://jumptuck.com 		*
*				*
********************************/

#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "oledControl.h"

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


uint8_t message[140] = "HELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLD\0";

uint8_t charListStart = 3;
#define CHARSETLEN  26  //How many characters does our fontfile have?

/**************** Prototypes *************************************/
void incSelOpt(void);
void decSelOpt(void);
void slideAlphaLeft(void);
void slideAlphaRight(void);
/**************** End Prototypes *********************************/

void init_IO(void){
  //Setup
    DDRB |= (1<<PB0) | (1<<PB2);	    //Set control pins as outputs
    PORTB &= ~(1<<PB0 | 1<<PB2);     //Set control pins low

    DDRB &= ~(BUT_LEFT | BUT_RIGHT | BUT_SEL);      //Set as input
    PORTB |= BUT_LEFT | BUT_RIGHT | BUT_SEL;      //Enable pull-up
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

    oledInit();

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
