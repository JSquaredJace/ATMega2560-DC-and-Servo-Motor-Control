#ifndef SERVO_H_
#define SERVO_H_

/*
 * Function:  initServo
 *  Sets up timer1 to output a PWM signal to the OC1B port (B6) with the
 *	following settings:		16-bit Fast PWM with ICR1 as the top of the count
 *							non inverted
 *							64 prescaler
 *
 *  returns:	none
 */
void initServo(){
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
 * Function:  servo
 *  Sets the servo to an angle based on the input value.
 *	ang	 int 	0 - 100 value for angle of servo
 *
 *  returns:	none
 */
void servo(int ang){
	servoVal = (625 * ang / 100) + 100;	//modify value for 1-2ms pulse (150 - 650)
	OCR1B = servoVal;					//update angle of servo
	return;
}

#endif