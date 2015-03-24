/* Smart RC Car */

#include <avr/io.h>
#include <avr/interrupt.h>

#define NUM_TASKS 4

typedef struct task {
	int state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;
task tasks[NUM_TASKS];

// periods
unsigned short period_JoyStick = 90;
unsigned short period_Motor = 3;
unsigned short period_GCD = 3;

unsigned char output_Motor = 0;
unsigned short input_ADC = 0;
unsigned char clockwise = 0;
unsigned char motor_Go = 0;
unsigned char distance = 0;
unsigned char up = 0;
unsigned char down = 0;
unsigned char left = 0;
unsigned char right = 0;

//Function definitions on bottom
unsigned char setBit(unsigned char, unsigned char, unsigned char);
unsigned char getBit(unsigned char, unsigned char);
void TimerOn();
void TimerSet(int);
void A2D_init();
int Tick_Motor(int);
int Tick_JoyStick(int);
int Tick_Distance_Send(int);
int Tick_Motor_Set(int);

void TimerISR() 
{	
	//get distance from arduino
	distance = PIND;
	
	unsigned char i = 0;
	for ( i = 0; i < NUM_TASKS; ++i ) 
	{
		
		if ( tasks[i].elapsedTime >= tasks[i].period ) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += period_GCD;
	}
}

ISR( TIMER1_COMPA_vect )
{
	TimerISR();
}

int main()
{
	DDRA = 0xF0; PORTA = 0x0F; // 0-3: joystick, 4-7: motorUpperRight
	DDRB = 0xFF; PORTB = 0x00; // 0-3: motorBottomRight, 4-7: data to 2nd av
	DDRC = 0xFF; PORTC = 0x00; // 0-3: motorBottomLeft, 4-7: motorUpperLeft 
	DDRD = 0x00; PORTD = 0xFF; // distance from Arduino
	A2D_init();

	//Joystick
	unsigned char i = 0;
	tasks[i].state = -1;
	tasks[i].period = period_JoyStick;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_JoyStick;
	i++;
	
	//Distance
	tasks[i].state = -1;
	tasks[i].period = period_GCD;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Distance_Send;
	i++;
	
	//Motor
	tasks[i].state = -1;
	tasks[i].period = period_Motor;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Motor;
	i++;

	//Motor set
	tasks[i].state = -1;
	tasks[i].period = period_GCD;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Motor_Set;
	i++;
	
	TimerSet( period_GCD );
	TimerOn();
	while(1)
	{
	}
	return 0;
}

int Tick_Motor_Set(int state)
{
	if( (up || down) && !left && !right)
	{
		//set port values to motor output
		//upper right
		for(unsigned i = 0; i < 4; ++i)
		{
			PORTA = setBit(PORTA, i+4, getBit(output_Motor, i));
		}
		//upper left
		for(unsigned i = 0; i < 4; ++i)
		{
			PORTC = setBit(PORTC, i+4, getBit(output_Motor, i));
		}
		//bottom left
		for(unsigned i = 0; i < 4; ++i)
		{
			PORTC = setBit(PORTC, i, getBit(output_Motor, i));
		}
		//bottom right
		for(unsigned i = 0; i < 4; ++i)
		{
			PORTB = setBit(PORTB, i, getBit(output_Motor, i));
		}
	}
	else if(left && !right && !down && !up)
	{
		//upper right
		for(unsigned i = 0; i < 4; ++i)
		{
			PORTA = setBit(PORTA, i+4, getBit(output_Motor, i));
		}
		//bottom right
		for(unsigned i = 0; i < 4; ++i)
		{
			PORTB = setBit(PORTB, i, getBit(output_Motor, i));
		}
	}
	else if(right && !left && !up && !down)
	{
		//upper left
		for(unsigned i = 0; i < 4; ++i)
		{
			PORTC = setBit(PORTC, i+4, getBit(output_Motor, i));
		}
		//bottom left
		for(unsigned i = 0; i < 4; ++i)
		{
			PORTC = setBit(PORTC, i, getBit(output_Motor, i));
		}
	}
	
	return state;
}

int Tick_Distance_Send(int state)
{
	//Transfer distance to 2nd AVR
	if(distance <= 8)
	{
		//stop
		PORTB = setBit(PORTB, 4, 1);
		PORTB = setBit(PORTB, 5, 0);
		PORTB = setBit(PORTB, 6, 0);
	}
	else if(distance > 8 && distance < 20)
	{
		//slow
		PORTB = setBit(PORTB, 4, 0);
		PORTB = setBit(PORTB, 5, 1);
		PORTB = setBit(PORTB, 6, 0);
	}
	else if( distance >= 20)
	{
		//go
		PORTB = setBit(PORTB, 4, 0);
		PORTB = setBit(PORTB, 5, 0);
		PORTB = setBit(PORTB, 6, 1);
	}
	
	return state;
}

enum States_Motor { STOP, PHASE_0, PHASE_1, PHASE_2, PHASE_3, PHASE_4, PHASE_5, PHASE_6, PHASE_7 };
int Tick_Motor(int state)
{
	//Transitions
	switch(state)
	{
		case -1:
			state = STOP;
			break;
			
		case STOP:
			//clockwise
			if( motor_Go && clockwise )
			{
				state = PHASE_7;
			}				
			//counter clockwise
			else if( motor_Go && !clockwise )
			{
				state = PHASE_1;
			}
			//stop
			else
			{
				//stay
			}
			
			break;
			
		case PHASE_0:
			if( clockwise )
				state = PHASE_7;
			else
				state = PHASE_1;
			
			//stop
			if( !motor_Go )
			{
				state = STOP;
			}
			
			break;
			
		case PHASE_1:
			if( clockwise )
				state = PHASE_0;
			else
				state = PHASE_2;
			
			//stop
			if( !motor_Go )
			{
				state = STOP;
			}
		
			break;
			
		case PHASE_2:
			if( clockwise )
				state = PHASE_1;
			else
				state = PHASE_3;
			
			//stop
			if( !motor_Go )
			{
				state = STOP;
			}
			break;
			
		case PHASE_3:
			if( clockwise )
				state = PHASE_2;
			else
				state = PHASE_4;
			
			//stop
			if( !motor_Go )
			{
				state = STOP;
			}
		
			break;
			
		case PHASE_4:
			if( clockwise )
				state = PHASE_3;
			else
				state = PHASE_5;
			
			//stop
			if( !motor_Go )
			{
				state = STOP;
			}
		
			break;
			
		case PHASE_5:
			if( clockwise )
				state = PHASE_4;
			else
				state = PHASE_6;
			
			//stop
			if( !motor_Go )
			{
				state = STOP;
			}
		
			break;
			
		case PHASE_6:
			if( clockwise )
				state = PHASE_5;
			else
				state = PHASE_7;
			
			//stop
			if( !motor_Go )
			{
				state = STOP;
			}
		
			break;
			
		case PHASE_7:
			if( clockwise )
				state = PHASE_6;
			else
				state = PHASE_0;
			
			//stop
			if( !motor_Go )
			{
				state = STOP;
			}
		
			break;
			
		default:
			state = STOP;
			break;
	}
	
	//Actions
	switch(state)
	{
		case STOP:
			break;
			
		//0001
		case PHASE_0:
			output_Motor = 1;
			//num_Phases--;
			break;
			
		//0011
		case PHASE_1:
			output_Motor = 3;
			//num_Phases--;
			break;
			
		//0010
		case PHASE_2:
			output_Motor = 2;
			//num_Phases--;
			break;
			
		//0110
		case PHASE_3:
			output_Motor = 6;
			//num_Phases--;
			break;
			
		//0100
		case PHASE_4:
			output_Motor = 4;
			//num_Phases--;
			break;
			
		//1100
		case PHASE_5:
			output_Motor = 12;
			//num_Phases--;
			break;
			
		//1000
		case PHASE_6:
			output_Motor = 8;
			//num_Phases--;
			break;
			
		//1001
		case PHASE_7:
			output_Motor = 9;
			//num_Phases--;
			break;
			
		default:
			break;
	}
	
	return state;
}

int Tick_JoyStick(int state)
{
	Set_A2D_Pin(0);
	input_ADC = ADC;
	
	//clockwise
	if( input_ADC > 700 )
	{
		up = 1;
		down = 0;
		left = 0;
		right = 0;
		motor_Go = 1;
		clockwise = 1;
		return 0;
	}
	//counter clockwise
	else if( input_ADC < 300 )
	{
		up = 0;
		down = 1;
		left = 0;
		right = 0;
		motor_Go = 1;
		clockwise = 0;
		return 0;
	}
	else
	{
		motor_Go = 0;
	}
	
	Set_A2D_Pin(1);
	input_ADC = ADC;
	
	//clockwise
	if( input_ADC > 700 )
	{
		up = 0;
		down = 0;
		left = 0;
		right = 1;
		motor_Go = 1;
		clockwise = 0;
		return 0;
	}
	//counter clockwise
	else if( input_ADC < 300 )
	{
		up = 0;
		down = 0;
		left = 1;
		right = 0;
		motor_Go = 1;
		clockwise = 1;
		return 0;
	}
	else
	{
		motor_Go = 0;
	}

	return state;
}

unsigned char setBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

unsigned char getBit(unsigned char port, unsigned char number)
{
	return ( port & (0x01 << number) );
}

void TimerOn()
{
	TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);
	TIMSK = (1<<OCIE1A); //Enables compare match interrupts
	SREG |= 0x80; //enables global interrupts
}

void TimerSet(int ms)
{
	TCNT1 = 0; //clear the timer counter
	OCR1A = ms * 125; //set the match compare value
}

void A2D_init()
{
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: Enables analog-to-digital conversion
	// ADSC: Starts analog-to-digital conversion
	// ADATE: Enables auto-triggering, allowing for constant
	//	    analog to digital conversions.
}


void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); }
}


int Tick_JoyStick(int state)
{
	//clockwise
	if( input_ADC > 600 )
	{
		motor_Go = 1;
		clockwise = 1;
		output_LCD[0] = 'F';
		output_LCD[1] = 'o';
		output_LCD[2] = 'r';
		output_LCD[3] = 'w';
		output_LCD[4] = 'a';
		output_LCD[5] = 'r';
		output_LCD[6] = 'd';
		output_LCD[7] = '\0';
	}
	//counter clockwise
	else if( input_ADC < 400 )
	{
		motor_Go = 1;
		clockwise = 0;
		output_LCD[0] = 'R';
		output_LCD[1] = 'e';
		output_LCD[2] = 'v';
		output_LCD[3] = 'e';
		output_LCD[4] = 'r';
		output_LCD[5] = 's';
		output_LCD[6] = 'e';
		output_LCD[7] = '\0';
	}
	else
	{
		motor_Go = 0;
		output_LCD[0] = 'S';
		output_LCD[1] = 't';
		output_LCD[2] = 'o';
		output_LCD[3] = 'p';
		output_LCD[4] = 'p';
		output_LCD[5] = 'e';
		output_LCD[6] = 'd';
		output_LCD[7] = '\0';
	}
	
	for(unsigned char i = 8; i < 16; ++i) output_LCD[i] = ' ';
	return state;
}
