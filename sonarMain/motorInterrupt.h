#ifndef MOTOR_ISR
#define MOTOR_ISR

#define TIMER_PRESCALE_1024 ((1<<CS00) | (1<<CS02))
#define TIMER_PRESCALE_256 (1<<CS02)

volatile unsigned int leftSpeed = 0;
volatile unsigned int rightSpeed = 0;

int timerState = 0;

ISR(TIMER0_OVF_vect){
	// Stop and reset the Timer
	TCCR0B &= ~((1<<CS00) | (1<<CS01) | (1<<CS02));
	TCNT0 = 0;

	int prescaler = 0;

	// Switch Timer State
	if (timerState != 0){
		// Set OCR0x
		OCR0B = leftSpeed;
		OCR0A = rightSpeed;

		// Set Prescaler to 8
		prescaler = TIMER_PRESCALE_256;

		// Set Timer State
		timerState = 0;

		// Set Pins to High
		PORTD |= (1<<PD5) | (1<<PD6);
	} else {
		// Set Pins to Low
		PORTD &= ~((1<<PD5) | (1<<PD6));

		// Disconnect Timer from Pins
		TCCR0A &= ~((1<<COM0A0) | (1<< COM0A1) | (1 << COM0B0) | (1 << COM0B1)); 

		// Set Prescaler to 64
		prescaler = TIMER_PRESCALE_1024;

		// Set Timer State
		timerState = 1;
	}

	// Enable Timer
	TCCR0B |= prescaler;

	#ifdef DEBUG_MOTOR_TIMING
		// Report Motor Speeds
		char buff[64];
		sprintf(buff, "Prescaler: %d\n",
			TCCR0B);
		Serial.print(buff);
	#endif
}

ISR(TIMER0_COMPA_vect){
	PORTD &= ~(1<<PD6);
}
ISR(TIMER0_COMPB_vect){
	PORTD &= ~(1<<PD5);
}
#endif