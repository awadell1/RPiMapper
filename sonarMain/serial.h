#ifndef SERIAL_H							// This is to prevent this file from being included more than once in the project
#define SERIAL_H

#include <stdio.h>

static int put_char(char c, FILE *stream);
static FILE uart_str = FDEV_SETUP_STREAM(put_char, NULL, _FDEV_SETUP_WRITE);

// Transmit one character
static int put_char(char c, FILE *stream) {
	loop_until_bit_is_set(UCSR0A, UDRE0);	// wait for data register (UDR0) to clear
	UDR0 = c;								//send the character
	return 0;
}

// Initialization of the UART. See data sheet to understand these settings.
void init_uart(void) { 						// 8 bit, 1 stop bit, no parity
	UCSR0B=_BV(TXEN0);						// Enable transmit
	UBRR0L = 51;							// scale from 16 MHz main clock to 19200 baud rate.
	UBRR0H = 0;
	stdout = &uart_str; 					//set the output stream
}

#endif // SERIAL_H