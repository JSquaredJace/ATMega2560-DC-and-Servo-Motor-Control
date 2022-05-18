#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
//#include <avr/interrupt.h>	//include if interrupts are used

/*
 * Function:  initADC
 *  Sets up and enable ADC with the following settings:		
 *							reference voltage is Vcc for ADC
 *							128 prescaler
 *
 *  returns:	none
 */
void initADC(){
	ADMUX = (1<<REFS0);	//AREF = AVcc
	
	//ADC Enable and prescaler of 128
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	
	return;
}

/*
 * Function:  ADCRead
 *  Sets ADC multiplexer to read from input channel number, starts single 
 *	conversion for current channel's analog voltage to digital value, waits 
 *	for conversion complete, and returns 10-bit digital value output by ADC as
 *	16-bit int.
 *
 *	ch	int	input channel number to set multiplexer to. determines which 
 *			analog pin will be converted and returned
 *
 *  returns:	int	Digital value of analog reading from input channel
 */
int ADCRead(int ch){
	int temp = 0;
	
	ch &= 0b00000111;				//clear higher bits of ch
	ADMUX = (ADMUX & 0xF8) | ch;	//set ADC to input channel
	
	
	ADCSRA |= (1<<ADSC);			//start single conversion
	
	
	while(ADCSRA & (1<<ADSC));	//wait for ADC conversion complete (ADSC = 0)
	
	temp = ADCL;				//concatenate ADC low and high bytes in temp
	temp |= (ADCH << 8);		//read ADC high second to clear ADCIF flag
	return (temp);				//return ADC value for input channel
}

#endif