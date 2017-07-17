#include <board.h>
#include <sysclk.h>
#include <st7565r.h>
#include <conf_usart_example.h>
#include "led.h"

int getCharacter(int input);
void displayCharacter(uint8_t character);
void resetScreen();

const int Characters[9] = {
	0x30,
	0x31,
	0x32,
	0x33,
	0x34,
	0x35,
	0x36,
	0x37,
	0x38,
	0x39
};

//! the page address to write to
uint8_t page_address;
//! the column address, or the X pixel.
uint8_t column_address;
//! store the LCD controller start draw line
uint8_t start_line_address = 0;

int main(void)
{
	board_init();
	sysclk_init();

	gpio_set_pin_high(NHD_C12832A1Z_BACKLIGHT); //turns backlight on

	// initialize the interface (SPI), ST7565R LCD controller and LCD
	st7565r_init();

	// set addresses at beginning of display
	resetScreen();

	// USART options.
	static usart_rs232_options_t USART_SERIAL_OPTIONS = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT
	};
	
	// Initialize usart driver in RS232 mode
	usart_init_rs232(USART_SERIAL_EXAMPLE, &USART_SERIAL_OPTIONS);

	uint8_t tx_buf[] = "\n\rEnter a message: ";
	int tx_length = 128;
	int i;
	for (i = 0; i < tx_length; i++) {
		usart_putchar(USART_SERIAL_EXAMPLE, tx_buf[i]);
	}

	uint8_t input;
	while (true) {
		input = usart_getchar(USART_SERIAL_EXAMPLE);
		int userInput = getCharacter(input);
		displayCharacter(userInput);
	}
}

int getCharacter(int input) {
	char character = (char)input;
	
	int x;
	switch(character){
		case '30':
			x = 1;
			break;
		case '31':
			x = 1;
			break;
		case '32':
		x = 2;
		printf("x = %d", &x);
		break;
		case '33':
		x = 3;
		break;
		case '34':
		x = 4;
		break;
		case '35':
		x = 5;
		break;
		case '36':
		x = 6;
		break;
		case '37':
		x = 7;
		break;
		case '38':
		x = 8;
		break;
		case '39':
		x = 9;
		break;
	}
	return x;
}

void displayCharacter(uint8_t character)
{
	// set addresses at beginning of display
	gpio_set_pin_low(NHD_C12832A1Z_BACKLIGHT); //turns backlight off

	if (character == 37) {
		start_line_address += 7;
		st7565r_set_display_start_line_address(start_line_address++);
		st7565r_set_column_address(0);
		st7565r_set_page_address(++page_address);
	}

	int i;
	if (character == 100) {
		resetScreen();
	}
	else {
		st7565r_write_data(Characters[character]);
	}
	delay_ms(100);
	gpio_set_pin_high(NHD_C12832A1Z_BACKLIGHT); //turns backlight on
}

void resetScreen()
{
	int counter = 0;
	// clear display
	for (page_address = 0; page_address <= 4; page_address++) {
		st7565r_set_page_address(page_address);
		for (column_address = 0; column_address < 128; column_address++) {
			st7565r_set_column_address(column_address);
			/* fill every other pixel in the display. This will produce
			horizontal lines on the display. */
			st7565r_write_data(0x00);
			if (counter >= 512)
				break;
		}
	}

	st7565r_set_page_address(0);
	st7565r_set_column_address(0);
}