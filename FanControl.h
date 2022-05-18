/*
 * FanControl.h
 *
 * Created: 5/8/2020 8:32:41 PM
 *  Author: Jace Johnson
 */ 


#ifndef FANCONTROL_H_
#define FANCONTROL_H_


#include <avr/io.h>
#include <avr/interrupt.h>
#include "ADC.h"

void InitTimer0();
void updateMotors();


/*
 * Function:  InitTimer0
 *  Sets up timer0 to output a PWM signal to the OC0B port (G5) with the
 *	following settings:		8-bit phase corrected PWM
 *							non inverted
 *							256 prescaler
 *
 *  returns:	none
 */
void InitTimer0(){
	TCCR0A |= (1 << WGM00);		//8-bit phase corr PWM mode
	TCCR0A |= (1 << COM0B1);	//non inverted mode
	TCCR0B |= (1 << CS02);		//set prescaler to 256
	
   	sei();    	//Enable global interrupts by setting global interrupt enable
                //bit in SREG
   	
   	TIMSK0 |= (1 << OCIE0B);	//enable timer0 compare B
   	OCR0B = 0;					//set timer0 compare B to 0
   	return;
}

/*
 * Function:  updateFan
 *  Reads ADC value for channel 0 (POT voltage), modifies value, and stores in
 *	fanSpeed var. Reads ADC value for channel 1 (photoresistor V-divider 
 *	voltage), modifies value, and stores in servoVal var. Update OC0B (G5 
 *	controlling DC motor speed) to be equal to fanSpeed, and Update OC1B (B6
 *	controlling servo motor angle) to be equal to servoVal.
 *
 *  returns:	none
 */
void updateFan(){
	int fanSpeed = 0;
	
	fanSpeed = ADCRead(0);			//read POT value to fanSpeed and modify
	fanSpeed = fanSpeed / 4;		//value for 0-255 range
	
	OCR0B = fanSpeed;				//update PWM outputs for speed of DC motor
	
	return;
}
#endif