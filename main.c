/*
 * Lab08 - DC and Servo Motor Control
 * Author : Jace Johnson
 * Created: 4/23/2020 2:46:51 PM
 * Rev 1  4/23/2020
 * Description:	ATMEGA2560 Takes in analog signals from a potentiometer (POT)
 *				and a photoresister. The controller then uses its on board
 *				timers to create pulse width modulated (PWM) signals to
 *				control the speed of a DC motor and the angle of an angular
 *				servo motor, where the POT controls the motor speed and the
 *				photoresistor controls the servo angle. The motor is equipped
 *				with fan blades and is interchangebly referred to as a fan or
 *				a DC motor.
 *
 * Hardware:	ATMega 2560 operating at 16 MHz
 *				PN2222A BJT
 *				1N4007 Diode
 *				DC motor with Fan Head
 *				servo motor
 *				breadboard power supply
 *				10 KOhm potentiometer (POT)
 *				Photoresistor
 *				1 KOhm resistor
 *				10 KOhm resistor
 *				jumper wires
 * Configuration shown below
 * 
 * ATMega 2560		  BJT Transistor
 *  PORT   pin			 PN2222A
 * ------------			---------  |--(+)Diode(-)--|
 * | G5		D4|--1KOhm--|B	   C|--|--(-)Motor(+)--|--5V
 * |		  |			|	    |
 * |		  |			|	   E|--GND			Diode, Motor Connection Desc:
 * |		  |			---------				1N4007 cathode connected in 
 * |		  |									parallel with the motor. Diode
 * |		  |				Servo				cathode connected to motor (-)
 * |		  |			-------------			and 5V supply. Diode Anode
 * | B6	   D12|---------|Orange  Red|--5V		connected to motor (+) and
 * |		  |			|			|			transistor collector.
 * |		  |			|	   Brown|--GND		purpose of the diode is to
 * |		  |			-------------			disperse current spikes when
 * |		  |									the DC motor changes speed
 * |		  |			 10 KOhm POT			suddenly.
 * |		  |			-------------
 * | F0		A0|---------|Wiper  High|--5V
 * |		  |			|			|
 * |		  |			|	     Low|--GND
 * |		  |			-------------		   Photoresistor Divider Desc:
 * |		  |								Photoresistor connected to 5V on
 * |		  |  |--Photoresistor--5V		one lead and 10 KOhm resistor on
 * | F1		A1|--|							the other lead. 10 KOhm connected
 * |		  |  |-----10KOhm-----GND		to GND to form a voltage divider.
 * |		  |								Middle of divider connected to A1
 * |		  |	  bread board supply		on ATMEGA 2560.
 * |		  |		  ---------
 * |	   GND|-------|GND  5V|--5V
 * |		  |  GND--|GND	  |
 * ------------	  	  ---------
 *				
 */

#define F_CPU 16000000

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

void InitTimer0();
void InitTimer1();
void InitADC();
void updateMotors();
int ADCRead(int ch);


/*
 * Function:  main
 *  Calls functions to initialize timers, the ADC, and output ports and loops
 *	forever calling function updateMotors to handle inputs and outputs.
 *
 *  returns:	1   program ends unexpectedly
 */
int main(void)
{
	DDRG |= 0b00100000;	//set pin G5 (OC0B) and pin B6 (OC1B) as PWM outputs
	DDRB |= 0b01000000;
	
	InitTimer0();		//initialize timers 0 and 1 and ADC
	InitTimer1();
	InitADC();
	
	//forever loop
    while(1)
    {
		updateMotors();	//check sensor inputs and update PWM outputs
    }
    return 1;
}

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
 * Function:  InitTimer1
 *  Sets up timer1 to output a PWM signal to the OC1B port (B6) with the
 *	following settings:		16-bit Fast PWM with ICR1 as the top of the count
 *							non inverted
 *							64 prescaler
 *
 *  returns:	none
 */
void InitTimer1(){
	//NON Inverted, Fast PWM, ICR1 top, 64 prescaler
	TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        
	TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10);
	
	ICR1=5000;	//PWM freq = 50Hz for servo
	
   	sei();    	//Enable global interrupts by setting global interrupt enable
                //bit in SREG
   	
   	TIMSK1 |= (1 << OCIE1B);	//enable timer1 compare B
   	OCR1B = 0;					//set timer1 compare B to 0
   	return;
}

/*
 * Function:  InitADC
 *  Sets up and enable ADC with the following settings:		
 *							reference voltage is Vcc for ADC
 *							128 prescaler
 *
 *  returns:	none
 */
void InitADC(){
	ADMUX = (1<<REFS0);	//AREF = AVcc
	
	//ADC Enable and prescaler of 128
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	
	return;
}

/*
 * Function:  updateMotors
 *  Reads ADC value for channel 0 (POT voltage), modifies value, and stores in
 *	fanSpeed var. Reads ADC value for channel 1 (photoresistor V-divider 
 *	voltage), modifies value, and stores in servoVal var. Update OC0B (G5 
 *	controlling DC motor speed) to be equal to fanSpeed, and Update OC1B (B6
 *	controlling servo motor angle) to be equal to servoVal.
 *
 *  returns:	none
 */
void updateMotors(){
	int fanSpeed = 0;
	int servoVal = 0;
	
	fanSpeed = ADCRead(0);			//read POT value to fanSpeed and modify
	fanSpeed = fanSpeed / 4;		//value for 0-255 range
	
	servoVal = ADCRead(1);				//read photoresistor value to servoVal
	servoVal = (servoVal / 2) + 100;	//and modify value for 1-2ms pulse
	
	OCR0B = fanSpeed;				//update PWM outputs for speed of DC motor
	OCR1B = servoVal;				//and angle of servo
	
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