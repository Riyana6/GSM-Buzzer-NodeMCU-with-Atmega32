#define F_CPU 8000000UL // define cpu frquency
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include "i2cmaster.h"
#include "i2c_lcd.h"
#include "USART.h"
#include "wifi.c"

#define START '^'
#define END '~'

#define MYUBRR F_CPU / 16UL / BAUD - 1

unsigned char At[] = {"AT\n"};				   // AT to check GSM
unsigned char Meg_Mode[] = {"AT+CMGF=1\n"};	// Active message mode
unsigned char Meg_cmd1[] = {"AT+CMGS="};	   // message command
unsigned char Re_num[] = {"\"+94766172720\""}; // receivers number
char emergency[40] = {"Emergency at the site!!!"};
/////////////////////////////////wifi

#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) // convert to baudrate dec value

#define STRING_SIZE 16

/////////////gsm
void Send_msg(void)
{
	int a;
	for (a = 0; At[a] != '\0'; a++)

	{
		usart_data_transmit(At[a]);
		//_delay_ms(10);
	}
	_delay_ms(100);

	for (a = 0; Meg_Mode[a] != '\0'; a++)
	{
		usart_data_transmit(Meg_Mode[a]);
		//_delay_ms(10);
	}
	_delay_ms(100);

	for (a = 0; Meg_cmd1[a] != '\0'; a++)
	{
		usart_data_transmit(Meg_cmd1[a]);
		//_delay_ms(1);
	}
	_delay_ms(100);

	for (a = 0; Re_num[a] != '\0'; a++)
	{
		usart_data_transmit(Re_num[a]);
		//_delay_ms(10);
	}
	usart_data_transmit(13); //UDR=(13);    //  <CR> character
	_delay_ms(100);

	for (a = 0; emergency[a] != '\0'; a++)
	{
		usart_data_transmit(emergency[a]);
		//_delay_ms(10);
	}
	usart_data_transmit(26);
	_delay_ms(100);
}
//buzzer tone
void buzzer()
{
	DDRB |= (1 << PINB0);   //make b0 output pin
	while (1)
	{
		PORTB &= ~(1 << PINB0); //b0 0
		_delay_ms(1000);
		PORTB |= (1 << PINB0); //b0 1
		_delay_ms(1000);
	}
}

void UART_init(long USART_BAUDRATE)
{
	UCSRB |= (1 << RXEN) | (1 << TXEN);					 // Turn on transmission and reception by setting RX Tx bits
	UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // Use 8-bit character sizes
	UBRRL = BAUD_PRESCALE;								 // Load lower 8-bits of the baud rate value
	UBRRH = (BAUD_PRESCALE >> 8);						 // Load upper 8-bits of the baud rate value
}

void UART_TxChar(char c)
{
	while (!(UCSRA & (1 << UDRE)))
		; // Wait for empty transmit buffer
	UDR = c;
}

void UART_sendString(char *str)
{
	unsigned char s = 0;

	while (str[s] != 0) // string till null
	{
		UART_TxChar(str[s]); // send s to UART_TxChar(s) function
		s++;
	}
}
unsigned char UART_RxChar() /* Data receiving function */
{
	while (!(UCSRA & (1 << RXC)))
		;		/* Wait until new data receive */
	return UDR; /* Get and return received data */
}

char *receiveStringFromWifi()
{
	static char str[STRING_SIZE];
	char num;
	while (UART_RxChar() != 255)
		;
	for (char i = 0; i < STRING_SIZE; i++)
	{
		if (num = UART_RxChar())
			str[i] = num;
		if (num == '\0')
			break;
	}
	return str;
}

unsigned char readCharFromWifi()
{
	return UART_RxChar();
}

void sendStringToWifi(char *str)
{
	_delay_ms(50);
	UART_sendString(str); // send string
	_delay_ms(1200);
}

#define SUCCESS_CHAR 'Z'

void waitTillChar(char ch)
{
	while (readCharFromWifi() != ch)
		;
}

void sendCharToWifi(char c)
{
	UART_sendString(c);
}

////wifi end

int main(void)
{
	DDRA &= ~(1 << PINA0); //make b0 input pin
	lcd_init(LCD_BACKLIGHT_ON);
	lcd_clear();
	lcd_goto_xy(0, 0);

	UART_init(9600); // initialize UART communication

	lcd_puts_at("  ALL IS WELL!", 0, 0);

	_delay_ms(3000);

	char value;

	while (1)
	{
		while ((value = readCharFromWifi()) != START);
		lcd_clear();
		lcd_clear();
		lcd_goto_xy(0, 1);
		value = readCharFromWifi();
		while (value != END)
		{
			lcd_putc(value);
			value = readCharFromWifi();
		}
		////////////send message
		Send_msg();
		////////////////////
		buzzer();
	}
}
