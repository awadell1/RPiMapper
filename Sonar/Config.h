/*
 * Config.h
 *
 * Created: 11/25/2015 12:59:01 AM
 *  Author: Alex
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

//////////////////////////////////////////////////////////////////////////
////					STANDARD OPERATING LIBRARIES				//////
//////////////////////////////////////////////////////////////////////////

//Define CPU Speed
#define F_CPU 16000000UL

//Include relevant Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//////////////////////////////////////////////////////////////////////////

#include "Motors.h"
#include "Sensors.h"
#include "States.h"


#endif /* CONFIG_H_ */