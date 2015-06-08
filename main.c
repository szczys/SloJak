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

#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define DATA (1<<PB3)		//MOSI (SI)
#define LATCH (1<<PB2)		//SS   (RCK)
#define CLOCK (1<<PB5)		//SCK  (SCK)

#define OLED_ADDRESS					0x78

#define OLED_CMD_DISPLAY_OFF			0xAE
#define OLED_CMD_DISPLAY_ON			    0xAF
#define OLED_CMD_NORMAL_DISPLAY		    0xA6
#define OLED_CMD_INVERSE_DISPLAY		0xA7

#define OLED_COMMAND_MODE				0x80
#define OLED_DATA_MODE				    0x40

#define PAGE_ADDRESSING		        	0x02

//Send send address, send command or data mode, send data, send stop

void init_IO(void){
  //Setup IO
  DDRB |= (1<<PB0);	//Set control pins as outputs
  PORTB &= ~(1<<PB0);		//Set control pins low
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

void oledSetCursor(uint8_t page, uint8_t col) {
    //Page
    oledWriteCmd(0xB0 + page);
    //Column
    oledWriteCmd((0x0F & col)+2);
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

        for (uint8_t col=0; col<128; col++) {
            oledWriteData(value);
        }
    }
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
        oledSetCursor(i,0);
        oledWriteData(0xFF);
        oledSetCursor(i,127);
        oledWriteData(0xFF);
    }

  while(1)
  {
    //wait for a little bit before repeating everything
    _delay_ms(200);
    PINB = (1<<PB0);    //Toggle the ouput
  }
}
