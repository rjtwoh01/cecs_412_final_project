#include <asf.h>
#include <board.h>
#include <sysclk.h>
#include <st7565r.h>
#include <conf_usart_example.h>
#include "led.h"

int main (void)
{
	board_init();
	sysclk_init();

	gpio_set_pin_high(NHD_C12832A1Z_BACKLIGHT);

	st7565r_init();
	
	//resetScreen();
	
	static usart_rs232_options USART_SERIAL_OPTIONS = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT
	}
	
	usart_init_rs232(USART_SERIAL_EXAMPLE);
}