#include <board.h>
#include <sysclk.h>
#include <st7565r.h>
#include <conf_usart_example.h>
#include <pwm.h>
#include "led.h"
#include <conf_example.h>

int getCharacter(int input);
void displayCharacter(uint8_t character);
static void adc_handler(ADC_t *adc, uint8_t ch_mask, adc_result_t result);
void resetScreen();
const int Characters[37][6] = {
	0x7E, 0x09, 0x09, 0x09, 0x7E, 0x00,  //A
	0x7F, 0x49, 0x49, 0x36, 0x00, 0x00,  //B
	0x3E, 0x41, 0x41, 0x41, 0x00, 0x00,  //C
	0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00,  //D
	0x7F, 0x49, 0x49, 0x49, 0x00, 0x00,  //E
	0x7F, 0x09, 0x09, 0x09, 0x00, 0x00,  //F
	0x3F, 0x41, 0x49, 0x79, 0x00, 0x00,  //G
	0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00,  //H
	0x00, 0x41, 0x7F, 0x41, 0x00, 0x00,  //I
	0x60, 0x41, 0x7F, 0x01, 0x00, 0x00,  //J
	0x7F, 0x08, 0x04, 0x0A, 0x71, 0x00,  //K
	0x7F, 0x40, 0x40, 0x40, 0x00, 0x00,  //L
	0x7F, 0x02, 0x04, 0x02, 0x7F, 0x00,  //M
	0x7F, 0x02, 0x04, 0x08, 0x7F, 0x00,  //N
	0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,  //O
	0x7F, 0x09, 0x09, 0x09, 0x06, 0x00,  //P
	0x3E, 0x41, 0x61, 0x7E, 0x80, 0x00,  //Q
	0x7F, 0x09, 0x19, 0x29, 0x46, 0x00,  //R
	0x46, 0x49, 0x49, 0x49, 0x31, 0x00,  //S
	0x01, 0x01, 0x7F, 0x01, 0x01, 0x00,  //T
	0x3F, 0x40, 0x40, 0x3F, 0x00, 0x00,  //U
	0x07, 0x18, 0x60, 0x18, 0x07, 0x00,  //V
	0x7F, 0x80, 0x70, 0x80, 0x7F, 0x00,  //W
	0x63, 0x14, 0x08, 0x14, 0x63, 0x00,  //X
	0x03, 0x04, 0x78, 0x04, 0x03, 0x00,  //Y
	0x31, 0x29, 0x25, 0x23, 0x21, 0x00,  //Z
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //SPACE
	0x42, 0x7F, 0x40, 0x00, 0x00,0x00,  //1
	0x62, 0x51, 0x4D, 0x46, 0x00,0x00,  //2
	0x41, 0x45, 0x4B, 0x31, 0x00,0x00,  //3
	0x3C, 0x22, 0x7F, 0x20, 0x00,0x00,  //4
	0x47, 0x45, 0x45, 0x39, 0x00,0x00,  //5
	0x7E, 0x49, 0x49, 0x31, 0x00,0x00,  //6
	0x01, 0x79, 0x05, 0x03, 0x00,0x00,  //7
	0x3E, 0x49, 0x49, 0x3E, 0x00,0x00,  //8
	0x06, 0x09, 0x09, 0x7F, 0x00,0x00,  //9
	0x7F, 0x41, 0x41, 0x7F, 0x00,0x00,  //0
	0x00, 0x00, 0x40, 0x00, 0x00, 0x00, //. (USED FOR NEW LINE)
};

//! the page address to write to
uint8_t page_address;
//! the column address, or the X pixel.
uint8_t column_address;
//! store the LCD controller start draw line
uint8_t start_line_address = 0;

struct pwm_config mypwm[4];
signed int switchValue;

int main(void)
{
	struct adc_config         adc_conf;
	struct adc_channel_config adcch_conf;
	
	board_init();
	sysclk_init();
	sleepmgr_init();
	irq_initialize_vectors();
	cpu_irq_enable();

	gpio_set_pin_high(NHD_C12832A1Z_BACKLIGHT); //turns backlight on

	// initialize the interface (SPI), ST7565R LCD controller and LCD
	st7565r_init();
	
	adc_read_configuration(&ADCB, &adc_conf);
	adcch_read_configuration(&ADCB, ADC_CH0, &adcch_conf);
	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_ON, ADC_RES_8,
	ADC_REF_VCC);
	adc_set_clock_rate(&adc_conf, 200000UL);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_enable_internal_input(&adc_conf, ADC_INT_TEMPSENSE);
	
	adc_write_configuration(&ADCB, &adc_conf);
	adc_set_callback(&ADCB, &adc_handler);
	
	/* Configure ADC channel 0:
	* - single-ended measurement from temperature sensor
	* - interrupt flag set on completed conversion
	* - interrupts disabled
	*/
	adcch_set_input(&adcch_conf, ADCCH_POS_PIN1, ADCCH_NEG_NONE,
	1);
	adcch_set_interrupt_mode(&adcch_conf, ADCCH_MODE_COMPLETE);
	adcch_enable_interrupt(&adcch_conf);
	
	adcch_write_configuration(&ADCB, ADC_CH0, &adcch_conf);
	
	// Enable the ADC and start the first conversion.
	adc_enable(&ADCB);
	adc_start_conversion(&ADCB, ADC_CH0);
	
	pwm_init(&mypwm[0], PWM_TCE0, PWM_CH_A, 50);
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

		if(input > 0 && input <10){
			displayCharacter(input);
			pwm_start(&mypwm[0], 2);
			int counter = 0;
			//pwm_start(&mypwm[0], 0);
			while(switchValue < 50 && counter < 50) {
				counter++;
				delay_ms(100);
			}
			//While read switch is open - do nothing
			while(switchValue > 50) {
				delay_ms(100);
			}
			
			delay_ms(500);

			pwm_start(&mypwm[0], 7.5);
			
			delay_s(2);
			
			pwm_start(&mypwm[0], 0);
		}
	}
}

int getCharacter(int input) {
	if(input == 13)
	return 100;
	
	if(input > 96 && input < 128){
		input -= 32;
	}
	
	if(input < 48 || (input > 57 && input < 65) || input > 90){
		return 26;
	}
	
	char character = (char)input;
	
	int x;
	switch(character){
		//int x;
		case 'A':
		x = 0;
		break;
		case 'B':
		x = 1;
		break;
		case 'C':
		x = 2;
		break;
		case 'D':
		x = 3;
		break;
		case 'E':
		x = 4;
		break;
		case 'F':
		x = 5;
		break;
		case 'G':
		x = 6;
		break;
		case 'H':
		x = 7;
		break;
		case 'I':
		x = 8;
		break;
		case 'J':
		x = 9;
		break;
		case 'K':
		x = 10;
		break;
		case 'L':
		x = 11;
		break;
		case 'M':
		x = 12;
		break;
		case 'N':
		x = 13;
		break;
		case 'O':
		x = 14;
		break;
		case 'P':
		x = 15;
		break;
		case 'Q':
		x = 16;
		break;
		case 'R':
		x = 17;
		break;
		case 'S':
		x = 18;
		break;
		case 'T':
		x = 19;
		break;
		case 'U':
		x = 20;
		break;
		case 'V':
		x = 21;
		break;
		case 'W':
		x = 22;
		break;
		case 'X':
		x = 23;
		break;
		case 'Y':
		x = 24;
		break;
		case 'Z':
		x = 25;
		break;
		case ' ':
		x = 26;
		break;
		case '1':
		x = 27;
		break;
		case '2':
		x = 28;
		break;
		case '3':
		x = 29;
		break;
		case '4':
		x = 30;
		break;
		case '5':
		x = 31;
		break;
		case '6':
		x = 32;
		break;
		case '7':
		x = 33;
		break;
		case '8':
		x = 34;
		break;
		case '9':
		x = 35;
		break;
		case '0':
		x = 36;
		break;
		case '.':
		x = 37;
		break;
		default:
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
		for (i = 0; i < 6; i++)
		{
			st7565r_write_data(Characters[character+26][i]);
		}
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

static void adc_handler(ADC_t *adc, uint8_t ch_mask, adc_result_t result)
{
	switchValue = result;

	// Start next conversion.
	adc_start_conversion(adc, ch_mask);
}