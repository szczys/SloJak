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

#include "menu.h"
#include "oledControl.h"


/************** Setup a rotary encoder ********************/
/* Atmega168 */
/* encoder port */
#define ENC_CTL	DDRB	//encoder port control
#define ENC_WR	PORTB	//encoder port write
#define ENC_RD	PINB	//encoder port read
#define ENC_A 6
#define ENC_B 7

volatile int8_t knobChange = 0;

/************** Setup input buttons *********************/
#define BUT_DDR     DDRC
#define BUT_PORT    PORTC
#define BUT_PIN     PINC
#define BUT_LEFT    (1<<PC0)
#define BUT_SEL     (1<<PC1)

uint8_t goLeft = 0;
uint8_t goSel = 0;

uint8_t message[140] = "HELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLDHELLOWORLD\0";

uint8_t previousMode = 0;
uint8_t windowMode = 0;

/**************** Prototypes *************************************/
void incSelOpt(void);
void decSelOpt(void);
void changeMode(uint8_t newMode);
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

void changeMode(uint8_t newMode) {
    previousMode = windowMode;
    windowMode = newMode;
}

int main(void)
{
    init_IO();
    init_interrupts();
    oledInit();
    _delay_ms(200);

    oledSetCursor(cursX, cursY);
    putChar(66,1);
    advanceCursor(6);

    compose();

    initMenu();

    while(1)
    {
        static uint16_t butCounter = 0;
        if (butCounter++ > 65000) {
            //FIXME: Proper button debounce and handling
            butCounter = 0;
            uint8_t readButtons = BUT_PIN;
            if (~readButtons & BUT_LEFT) {
                ++goLeft;
            }
            if (~readButtons & BUT_SEL) {
                ++goSel;
            }
        }
        switch (windowMode) {
            case 0:
                if (knobChange) {
                    if (knobChange > 0) {
                        incSelOpt();
                        slideAlphaLeft();
                    }
                    else {
                        decSelOpt();
                        slideAlphaRight();
                    }
                    knobChange = 0;
                }

                if (goLeft) {
                    changeMode(MENUCANCEL);
                    goLeft = 0;
                    goSel = 0;
                    //cancelMsg();
                    doBack();
                }
                else if (goSel) {
                    selectChar();
                    goLeft = 0;
                    goSel = 0;
                }
                break;

            default:
                if (knobChange) {
                    if (knobChange > 0) {
                        //menuUp();
                        knobLeft();
                    }
                    else {
                        //menuDn();
                        knobRight();
                    }
                    knobChange = 0;
                }

                if (goSel) {
                    //Lookup and execute action
                    doSelect[optionIndex]();
                    goSel = 0;
                }
                else if (goLeft) {
                    doBack();
                    goLeft = 0;
                };
                break;
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
    knobChange = -1;
    encval = 0;
  }
  else if( encval > 3  ) {  //four steps backwards
    knobChange = 1;
    encval = 0;
  }
}
