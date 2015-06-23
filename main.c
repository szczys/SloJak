/********************************
* SloJak                    	*
* MIT License               	*
* Copyright 2015 - Mike Szczys  *
* http://jumptuck.com 	    	*
*				                *
********************************/

#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "oledControl.h"
//#include "menu.h"

/************** Setup a rotary encoder ********************/
/* Atmega168 */
/* encoder port */
#define ENC_CTL	DDRB	//encoder port control
#define ENC_WR	PORTB	//encoder port write
#define ENC_RD	PINB	//encoder port read
#define ENC_A 6
#define ENC_B 7

volatile int8_t selected_option = 0;

/************** Setup input buttons *********************/
#define BUT_DDR     DDRC
#define BUT_PORT    PORTC
#define BUT_PIN     PINC
#define BUT_LEFT    (1<<PC0)
#define BUT_SEL     (1<<PC1)

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
    //Rotary Encoder
    ENC_CTL &= ~(1<<ENC_A | 1<<ENC_B);
    ENC_WR |= 1<<ENC_A | 1<<ENC_B;

    //LEDs
    DDRB |= (1<<PB0) | (1<<PB2);    //Set control pins as outputs
    PORTB &= ~(1<<PB0 | 1<<PB2);    //Set control pins low

    //Buttons
    BUT_DDR &= ~(BUT_LEFT | BUT_SEL);      //Set as input
    BUT_PORT |= BUT_LEFT | BUT_SEL;      //Enable pull-up
}

void init_interrupts(void) {
    PCICR |= 1<<PCIE0;      //enable PCINT0_vect  (PCINT0..7 pins)
    PCMSK0 |= 1<<PCINT6;    //interrupt on PCINT6 pin
    PCMSK0 |= 1<<PCINT7;    //interrupt on PCINT7 pin
    sei();
}

int main(void)
{
    init_IO();
    init_interrupts();
    oledInit();
    _delay_ms(200);
    //initMenu();

    oledSetCursor(cursX, cursY);
    putChar(1);
    advanceCursor(6);

    //show which letter will be selected
    showHighlighted(HIGHLIGHTCHAR*CHARWID+1,6);  //21 charperline on 128px display plus 1 pixel for centering
    showCharList(charListStart,CHARSETLEN,7);
    
    //menuCancel(0,0);

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

        static uint16_t butCounter = 0;
        if (butCounter++ > 65000) {
            //FIXME: Proper button debounce and handling
            butCounter = 0;
            uint8_t readButtons = BUT_PIN;
            if (~readButtons & BUT_LEFT) {
                incSelOpt();
                PORTB |= 1<<PB0;
            }
            if (~readButtons & BUT_SEL) {
                oledSetCursor(cursX, cursY);
                putChar(findHighlighted(charListStart,CHARSETLEN));
                advanceCursor(6);
            }
        }
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
