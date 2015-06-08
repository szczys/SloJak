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

void oledSetCursor(uint8_t page, uint8_t col) {
    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    //Page
    i2c_write(0xB0 + page);
    //Column
    i2c_write(0x0F & col);
    i2c_write(0x10 + (col>>4));
    i2c_stop();
}

void oledClearScreen(uint8_t black) {
    uint8_t value = 0xFF;
    if (black) { value = 0x00; }
    for (uint8_t page=0; page<8; page++) {
        i2c_start_wait(OLED_ADDRESS);
        i2c_write(OLED_COMMAND_MODE);
        i2c_write(0xB0 + page);
        i2c_write(0x00);
        i2c_write(0x10);
        i2c_stop();
        
        i2c_start_wait(OLED_ADDRESS);
        i2c_write(OLED_DATA_MODE);
        for (uint8_t col=0; col<128; col++) {
            i2c_write(value);
        }
        i2c_stop();
    }
}

int main(void)
{
    init_IO();
    _delay_ms(200);
    i2c_init();

    _delay_ms(10);
    i2c_start_wait(OLED_ADDRESS+I2C_WRITE);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(OLED_CMD_DISPLAY_OFF);
    i2c_stop();
    _delay_ms(10);

    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(OLED_CMD_DISPLAY_ON);
    i2c_stop();
    _delay_ms(10);
    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(OLED_CMD_NORMAL_DISPLAY);
    i2c_stop();
    /*
    _delay_ms(10);
    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(OLED_CMD_NORMAL_DISPLAY);
    i2c_stop();
    */
    _delay_ms(10);
    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(0x20);
    i2c_write(PAGE_ADDRESSING);
    i2c_stop();
    _delay_ms(10);
    
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

    oledClearScreen(1);

    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(0xB2);
    i2c_stop();

    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_DATA_MODE);
    for (uint8_t i=0; i<32; i++) {
    i2c_write(0x00);
    i2c_write(0xFF);
    }
    i2c_stop();

    //oledSetCursor(4,20);

    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(0xB4);
    i2c_write(0x00);
    i2c_write(0x16);
    i2c_stop();

    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_DATA_MODE);
    for (uint8_t i=0; i<12; i++) {
    i2c_write(0x00);
    i2c_write(0xFF);
    }
    i2c_stop();
    
    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(0xB5);
    i2c_write(0x01);
    i2c_write(0x10);
    i2c_stop();

    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_DATA_MODE);
    i2c_write(0xFF);
    i2c_stop();

    i2c_start_wait(OLED_ADDRESS);
    i2c_write(OLED_COMMAND_MODE);
    i2c_write(0x26);
    i2c_write(0x2F);
    i2c_stop();
  while(1)
  {
    //wait for a little bit before repeating everything
    _delay_ms(200);
    PINB = (1<<PB0);    //Toggle the ouput
  }
}
