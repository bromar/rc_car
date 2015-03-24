/* Smart RC Car */

#include <avr/io.h>
#include <avr/interrupt.h>

#define NUM_TASKS 5

typedef struct task {
	int state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;
task tasks[NUM_TASKS];

// task periods
unsigned short period_LED_Matrix = 1;
unsigned short period_Speaker_Go = 3000;
unsigned short period_Speaker_Slow = 1500;
unsigned short period_Speaker_Stop = 500;
unsigned short period_GCD = 1;
unsigned short period_Distance = 100;

// status variables
unsigned char slow = 0;
unsigned char go = 0;
unsigned char stop = 0;
unsigned char input_C = 0;
double frequency = 0.0;	
unsigned char on_Go = 0;
unsigned char on_Slow = 0;
unsigned char on_Stop = 0;	
unsigned char speaker_Go = 0;
unsigned char speaker_Slow = 0;
unsigned char speaker_Stop = 0;

// LED Matrix									
unsigned char red[ 8 ]			=	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
unsigned char green[ 8 ]		=	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char blue[ 8 ]			=	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	
//Function definitions on bottom
unsigned char setBit(unsigned char, unsigned char, unsigned char);
unsigned char getBit(unsigned char, unsigned char);
void init_PWM();
void TimerOn();
void TimerSet(int);
int Tick_LED_Matrix(int);
int Tick_Distance(int);
int Tick_Speaker_Go(int);
int Tick_Speaker_Slow(int);
int Tick_Speaker_Stop(int);
void transmit_data_all(unsigned char, unsigned char, unsigned char, unsigned char);

void TimerISR() 
{
	//input from other avr for distance
	input_C = PINC;
	
	//tells us how far object in front is
	stop	= getBit(input_C, 0);
	slow	= getBit(input_C, 1);
	go		= getBit(input_C, 2);
	
	//loop through all tasks
	//only calling it when it is time
	unsigned char i = 0;
	for ( i = 0; i < NUM_TASKS; ++i ) 
	{
		if ( tasks[i].elapsedTime >= tasks[i].period ) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += period_GCD;
	}

	//set speaker
	set_PWM(frequency);
}

ISR( TIMER1_COMPA_vect )
{
	TimerISR();
}

int main()
{
	init_PWM();
	
	DDRA = 0xFF; PORTA = 0x00; //Red, Blue
	DDRB = 0xFF; PORTB = 0x00; //Power, Green
	DDRC = 0x00; PORTC = 0xFF; //input from other avr
	DDRD = 0xFF; PORTD = 0x00;
	
	unsigned char i = 0;
	//Distance
	tasks[i].state = -1;
	tasks[i].period = period_Distance;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Distance;
	i++;
	
	//Speaker Go
	tasks[i].state = -1;
	tasks[i].period = period_Speaker_Go;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Speaker_Go;
	i++;
	
	//Speaker Slow
	tasks[i].state = -1;
	tasks[i].period = period_Speaker_Slow;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Speaker_Slow;
	i++;
	
	//Speaker Stop
	tasks[i].state = -1;
	tasks[i].period = period_Speaker_Stop;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Speaker_Stop;
	i++;
	
	//LED Matrix
	tasks[i].state = -1;
	tasks[i].period = period_LED_Matrix;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_LED_Matrix;
	i++;
	
	TimerSet( period_GCD );
	TimerOn();
	while(1)
	{
	}
	return 0;
}

// speaker noise with no objects ahead
int Tick_Speaker_Go(int state)
{
	if(speaker_Go)
	{
		if(on_Go)
		{
			frequency = 261.63;
			on_Go = 0;
		}
		else
		{
			frequency = 0.0;
			on_Go = 1;
		}
	}
	
	return state;
}

// speaekr when object approaching
int Tick_Speaker_Slow(int state)
{
	if(speaker_Slow)
	{
		if(on_Slow)
		{
			frequency = 349.23;
			on_Slow = 0;
		}
		else
		{
			frequency = 0.0;
			on_Slow = 1;
		}
	}
	
	return state;
}

//speaker noise when object right infront of car
int Tick_Speaker_Stop(int state)
{
	if(speaker_Stop)
	{
		if(on_Stop)
		{
			frequency = 493.88;
			on_Stop = 0;
		}
		else
		{
			frequency = 0.0;
			on_Stop = 1;
		}
	}
	
	return state;
}

// handles speaker and LED matrix events
int Tick_Distance(int state)
{
	if( go )
	{
		speaker_Go = 1;
		speaker_Slow = 0;
		speaker_Stop = 0;
		
		red[0] = 0xE7; red[1] = 0xDB; red[2] = 0x99; red[3] = 0x66;
		red[4] = 0x66; red[5] = 0x99; red[6] = 0xDB; red[7] = 0xE7;
		for(unsigned i = 0; i < 8; ++i) green[i] = 0xFF;
		for(unsigned i = 0; i < 8; ++i) blue[i] = 0xFF;
	}
	//slow
	else if( slow )
	{
		speaker_Go = 0;
		speaker_Slow = 1;
		speaker_Stop = 0;
	
		for(unsigned i = 0; i < 8; ++i) red[i] = 0xFF;
		green[0] = 0xE8; green[1] = 0xEA; green[2] = 0xE2; green[3] = 0xDF;
		green[4] = 0xBF; green[5] = 0xDF; green[6] = 0xBF; green[7] = 0xDF;
		blue[0] = 0x1F; blue[1] = 0x5F; blue[2] = 0x1F; blue[3] = 0xFF;
		blue[4] = 0xE0; blue[5] = 0xEF; blue[6] = 0xEF; blue[7] = 0xEF;
	}
	
	else if( stop )
	{
		speaker_Go = 0;
		speaker_Slow = 0;
		speaker_Stop = 1;

		for(unsigned i = 0; i < 8; ++i) red[i] = 0xFF;
		green[0] = 0xC3; green[1] = 0xDB; green[2] = 0xCB; green[3] = 0xCB;
		green[4] = 0xFF; green[5] = 0xFF; green[6] = 0xFF; green[7] = 0xFF;
		blue[0] = 0xFF; blue[1] = 0xFF; blue[2] = 0xFF; blue[3] = 0xFF;
		blue[4] = 0xC3; blue[5] = 0xDB; blue[6] = 0xDB; blue[7] = 0xC3;
	}
	else
	{
		for(unsigned i = 0; i < 8; ++i)	red[i] = 0xFF;
		for(unsigned i = 0; i < 8; ++i) blue[i] = 0xFF;
		green[0] = 0xFE; for(unsigned i = 1; i < 8; ++i) green[i] = 0xFF;
	}
	
	return state;
}

enum States_LED_Matrix{ MATRIX_COL_0, MATRIX_COL_1, MATRIX_COL_2, MATRIX_COL_3, MATRIX_COL_4, MATRIX_COL_5, MATRIX_COL_6, MATRIX_COL_7};
int Tick_LED_Matrix(int state)
{
	//Transitions
	switch( state )
	{
		case -1:
			state		=	MATRIX_COL_0;
			break;
		
		case MATRIX_COL_0:
			state		=	MATRIX_COL_1;
			break;
		
		case MATRIX_COL_1:
			state		=	MATRIX_COL_2;
			break;
		
		case MATRIX_COL_2:
			state		=	MATRIX_COL_3;
			break;
		
		case MATRIX_COL_3:
			state		=	MATRIX_COL_4;
			break;
		
		case MATRIX_COL_4:
			state		=	MATRIX_COL_5;
			break;
		
		case MATRIX_COL_5:
			state		=	MATRIX_COL_6;
			break;
		
		case MATRIX_COL_6:
			state		=	MATRIX_COL_7;
			break;
		
		case MATRIX_COL_7:
			state		=	MATRIX_COL_0;
			break;
		
		default:
			state		=	MATRIX_COL_0;
			break;
	}
	
	//Actions
	switch( state )
	{
		case MATRIX_COL_0:
			transmit_data_all( 0x01, red[ 0 ], green[ 0 ], blue[ 0 ] );
			break;
		
		case MATRIX_COL_1:
			transmit_data_all( 0x02, red[ 1 ], green[ 1 ], blue[ 1 ] );
			break;
		
		case MATRIX_COL_2:
			transmit_data_all( 0x04, red[ 2 ], green[ 2 ], blue[ 2 ] );
			break;
		
		case MATRIX_COL_3:
			transmit_data_all( 0x08, red[ 3 ], green[ 3 ], blue[ 3 ] );
			break;
		
		case MATRIX_COL_4:
			transmit_data_all( 0x10, red[ 4 ], green[ 4 ], blue[ 4 ] );
			break;
		
		case MATRIX_COL_5:
			transmit_data_all( 0x20, red[ 5 ], green[ 5 ], blue[ 5 ] );
			break;
		
		case MATRIX_COL_6:
			transmit_data_all( 0x40, red[ 6 ], green[ 6 ], blue[ 6 ] );
			break;
		
		case MATRIX_COL_7:
			transmit_data_all( 0x80, red[ 7 ], green[ 7 ], blue[ 7 ] );
			break;
	}
	
	return state;
}

// send LED matrix through shift registers in parallel
void transmit_data_all( unsigned char tmp_power, unsigned char tmp_red, unsigned char tmp_green, unsigned char tmp_blue) {
	unsigned char i;
	for (i = 0; i < 8 ; ++i) {
		// BLUE
			// SRCLR to 1: set 3 to 1, SRCLK to 0: set 2 to 0
			PORTA = setBit( PORTA, 3, 1 );
			PORTA = setBit( PORTA, 2, 0 );
			// set SER = next bit of data to be sent.
			PORTA = setBit( PORTA, 0, getBit( tmp_blue, i ) );
			// SRCLK to 1: set 2 to 1
			PORTA = setBit( PORTA, 2, 1 );
		
		// RED
			// SRCLR to 1: set 7 to 1, SRCLK to 0: set 6 to 0
			PORTA = setBit( PORTA, 7, 1 );
			PORTA = setBit( PORTA, 6, 0 );
			// set SER = next bit of data to be sent.
			PORTA = setBit( PORTA, 4, getBit( tmp_red, i ) );
			// SRCLK to 1: set 6 to 1
			PORTA = setBit( PORTA, 6, 1 );
		
		// GREEN
			// SRCLR to 1: set 7 to 1, SRCLK to 0: set 6 to 0
			PORTB = setBit( PORTB, 7, 1 );
			PORTB = setBit( PORTB, 6, 0 );
			// set SER = next bit of data to be sent.
			PORTB = setBit( PORTB, 4, getBit( tmp_green, i ) );
			// SRCLK to 1: set 6 to 1
			PORTB = setBit( PORTB, 6, 1 );
		
		// POWER
			// SRCLR to 1: set 3 to 1, SRCLK to 0: set 2 to 0
			PORTB = setBit( PORTB, 3, 1 );
			PORTB = setBit( PORTB, 2, 0 );
			// set SER = next bit of data to be sent.
			PORTB = setBit( PORTB, 0, getBit( tmp_power, i ) );
			// SRCLK to 1: set 2 to 1
			PORTB = setBit( PORTB, 2, 1 );
	}

	// RCLK to 1: set 1 to 1
	PORTA = setBit( PORTA, 1, 1 );
	// clears all lines in preparation of a new transmission
	PORTA &= 0xF0;
	
	// RCLK to 1: set 5 to 1
	PORTA = setBit( PORTA, 5, 1 );
	// clears all lines in preparation of a new transmission
	PORTA &= 0x0F;
	
	// RCLK to 1: set 5 to 1
	PORTB = setBit( PORTB, 5, 1 );
	// clears all lines in preparation of a new transmission
	PORTB &= 0x0F;
	
	// RCLK to 1: set 1 to 1
	PORTB = setBit( PORTB, 1, 1 );
	// clears all lines in preparation of a new transmission
	PORTB &= 0xF0;
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

void init_PWM() {
	TCCR2 = (1 << WGM21) | (1 << COM20) | (1 << CS22);
}

void set_PWM(double frequency) {
	if (frequency < 1){
		OCR2 = 0;
	}else{
		OCR2 = (int)(8000000 / (128 * frequency)) - 1;
	}
}


enum States_Speaker { SPEAKER_INIT, SPEAKER_GO, SPEAKER_SLOW, SPEAKER_STOP };
int Tick_Speaker(int state)
{
	//Transitions
	switch(state)
	{
		case -1:
			state = SPEAKER_INIT;
			break;
			
		case SPEAKER_INIT:
			state = SPEAKER_GO;
			break;
			
		case SPEAKER_GO:
			//switch to slow
			if(speaker_Slow)
			{
				timer = 0;
				on_Freq_Flag = 1;
				state = SPEAKER_SLOW;
			}	
			//stop
			else if(speaker_Stop)
			{
				timer = 0;
				on_Freq_Flag = 1;
				state = SPEAKER_STOP;
			}
			//go
			else
			{
				//stay
			}				
			
			break;
			
		case SPEAKER_SLOW:
			//switch to stop
			if(speaker_Stop)
			{
				timer = 0;
				on_Freq_Flag = 1;
				state = SPEAKER_STOP;
			}
			//go
			else if(speaker_Go)
			{
				timer = 0;
				on_Freq_Flag = 1;
				state = SPEAKER_GO;
			}
			//slow
			else
			{
				//stay
			}
			
			break;
			
		case SPEAKER_STOP:
			//switch to slow
			if(speaker_Slow)
			{
				timer = 0;
				on_Freq_Flag = 1;
				state = SPEAKER_SLOW;
			}
			//go
			else if(speaker_Go)
			{
				timer = 0;
				on_Freq_Flag = 1;
				state = SPEAKER_GO;
			}
			//stop
			else
			{
				//stay
			}
			
			break;
			
		default:
			state = SPEAKER_INIT;
			break;
	}	
	
	//Actions
	switch(state)
	{
		case SPEAKER_INIT:
			timer = 0;
			on_Freq_Flag = 1;
			frequency = 0.0;
			
			break;
			
		case SPEAKER_GO:
			timer++;
			//time to toggle speaker frequency
			if(timer >= 5000)
			{
				timer = 0;
				//toggle freqency
				if(on_Freq_Flag)
				{
					frequency = 261.63; //C note
				}
				else
				{
					frequency = 0.0; //turn speaker off
				}
				
				on_Freq_Flag = !on_Freq_Flag;
			}
			
			break;
			
		case SPEAKER_SLOW:
			timer++;
			//time to toggle speaker freqency
			if(timer >= 2500)
			{
				timer = 0;
				//toggle frequency
				if(on_Freq_Flag)
				{
					frequency = 349.23; //F note
				}
				else
				{
					frequency = 0.0; //turn speaker off
				}
				
				on_Freq_Flag = !on_Freq_Flag;
			}
			
			break;
			
		case SPEAKER_STOP:
			timer++;
			//time to toggle speaker frequency
			if(timer >= 1000)
			{
				timer = 0;
				//toggle frequency
				if(on_Freq_Flag)
				{
					frequency = 493.88; //B note
				}
				else
				{
					frequency = 0.0; //turn speaker off
				}
				
				on_Freq_Flag = !on_Freq_Flag;					
			}
			
			break;
	}
	
	return state;
}
