#ifndef MOTOR_ISR
#define MOTOR_ISR

#define TIMER_PRESCALE_64 ((1<<CS00) | (1<<CS01))
#define TIMER_PRESCALE_8 (1<<CS01)

ISR(TIMER0_OVF_vect){
	// Switch Timer State
	if ( TCCR0B & TIMER_PRESCALE_64){
		// Setup Timer to generate the control pulse
		timerState = 0;

		// Set Prescaler to 8
		TCCR0B &= ~((1<<CS00) | (1<<CS01) | (1<<CS02));	// Clear Prescaler
		TCCR0B |= TIMER_PRESCALE_8;					// Set to 8

		// Set Pins to High
		DDRD |= (1<<PD5) | (1<<PD6);

		// Set Timer to 0
		TCNT0 = 0;

		// Clear Pin on Compare Match
		TCCR0A |= (1<< COM0A1) | (1 << COM0B1); 
	} else {
		// Setup Timer to hold off
		timerState = 1;

		// Disconnect Timer from Pins
		TCCR0A &= ~((1<<COM0A0) | (1<< COM0A1) | (1 << COM0B0) | (1 << COM0B1)); 

		// Set Prescaler to 64
		TCCR0B &= ~((1<<CS00) | (1<<CS01) | (1<<CS02));	// Clear Prescaler
		TCCR0B |= TIMER_PRESCALE_64;					// Set to 64

		// Set Timer to 0
		TCNT0 = 0;
	}
}
#endif