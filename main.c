#include <avr/io.h>	//importing libraries to be used

#include <avr/interrupt.h>

#define AVG_RATE 4
#define M 1
#define C 0

uint8_t in[AVG_RATE] = {0, 0, 0, 0};	//array that holds the moving average value
uint8_t max1 = 0;	//max and min global variables
uint8_t max2 = 0;
uint8_t max = 0;
uint8_t min = 0;
uint8_t min1 = 0;
uint8_t min2 = 0;
uint8_t avg = 0;
uint8_t avg1 = 0;
uint8_t avg2 = 0;
uint8_t state = '3';
uint8_t buffer[1];
uint8_t start = 1;	//control variable that initially loads avg value into min variable to start comparing it with next values



uint8_t SevenSegment(uint8_t);	//prototype of function that displays result on 7-segment common cathode
uint8_t SevenSegment_PA(uint8_t);	//prototype of function that displays result on 7-segment common cathode (but using last 7 bits)
void USART_Transmit(uint8_t data);	//prototype of function for transmitting char
void USART_Transmit_String(char* str);	//prototype of function for transmitting string
uint8_t moving_average(uint8_t);	//prototype of moving average function
uint8_t linear_regression(uint8_t);	//linear regression function





ISR(TIMER1_COMPA_vect)	//ISR for compare flag for timer 1
{
	ADCSRA |= (1 << ADSC);	//starting conversion after every 1 second (sampling at every 1 second)
}



ISR(ADC_vect)	//ISR for ADC

{
	avg = moving_average(ADCH);
	avg = linear_regression(avg);
	
	avg1 = avg / 10;
	avg2 = avg % 10;
	
	if(max < avg)
	{
		max = avg;
		max1 = max / 10;
		max2 = max % 10;
	}
	
	if(start)
	{
		min = avg;
		min1 = min / 10;
		min2 = min % 10;
		
		start = 0;
	}
	
	if(min > avg)
	{
		min = avg;
		min1 = min / 10;
		min2 = min % 10;
	}
	
	if (state == '1')
	{
		PORTA = SevenSegment_PA(max1);
		PORTB = SevenSegment(max2);
		
		itoa(max1, buffer, 10);	//function that converts output to type character array, with base 10
		USART_Transmit(buffer[0]);	//sending the 1st integer value to the PC serially
		itoa(max2, buffer, 10);	//function that converts output to type character array, with base 10
		USART_Transmit(buffer[0]);	//sending the 2nd integer value to the PC serially
		USART_Transmit('C');	//sending the Temperature string to the PC serially
		USART_Transmit('\r');	//sending line break to the PC serially
	}
	
	if (state == '2')
	{
		PORTA = SevenSegment_PA(min1);
		PORTB = SevenSegment(min2);
		
		itoa(min1, buffer, 10);	//function that converts output to type character array, with base 10
		USART_Transmit(buffer[0]);	//sending the 1st integer value to the PC serially
		itoa(min2, buffer, 10);	//function that converts output to type character array, with base 10
		USART_Transmit(buffer[0]);	//sending the 2nd integer value to the PC serially
		USART_Transmit('C');	//sending the Temperature string to the PC serially
		USART_Transmit('\r');	//sending line break to the PC serially
	}
	
	if (state == '3')
	{
		PORTA = SevenSegment_PA(avg1);
		PORTB = SevenSegment(avg2);
		
		itoa(avg1, buffer, 10);	//function that converts output to type character array, with base 10
		USART_Transmit(buffer[0]);	//sending the 1st integer value to the PC serially
		itoa(avg2, buffer, 10);	//function that converts output to type character array, with base 10
		USART_Transmit(buffer[0]);	//sending the 2nd integer value to the PC serially
		USART_Transmit('C');	//sending the Temperature string to the PC serially
		USART_Transmit('\r');	//sending line break to the PC serially
	}	
}



ISR(USART_RXC_vect)	//ISR for receive complete of USART
{
	USART_Transmit('\r');
	
	switch(UDR)
	{
		case '1' :	
		{
			state = '1';
			break;			
		}
		
		case '2' :
		{
			state = '2';
			break;
		}
		
		case '3' :
		{
			state = '3';
			break;
		}
		
		default :
		{
			USART_Transmit_String("Error! \r");
			
			state = state;
			break;
		}	
	}
}



int main(void)

{
	DDRA = 254;	//Enabling PORTA for input & output
	DDRB = 255;	//Enabling PORTB for output
	
	TCNT1 = 0;	//initializing counter
	TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);	//setting timer to work in CTC mode, and with a prescaler of clk/64
	TIMSK = (1 << OCIE1A);	//enabling interrupt for compare flag for timer1
	OCR1A = 15625;	//value at which it will stop comparing (after passing of every 1s)
	

	UBRRL = 12;	//setting baudrate to 4800	
	UCSRB = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);	//enabling receive interrupt, transmission, and reception of data
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);	//setting register select to UCSRC, asynchronous mode, no parity, 1 stop bit, and 9 bit character size

	ADMUX = (1 << ADLAR) | (1 << REFS0) | (1 << REFS1);	//setting internal 2.56V, input channel ADC0, and result left adjusted (to divide the result by 4)	
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);	//enabling ADC, interrupt, and setting prescaler to clk/128

	USART_Transmit_String("WELCOME! \r FOLLOWING KEYS ARE BINDED TO FOLLOWING COMMANDS: \r 1: MAX \r 2: MIN \r 3: NOMINAL \r \r");
	
	sei();
	
	PORTB = SevenSegment(0);
	PORTA = SevenSegment_PA(0);

	while(1);	//keep the AVR active
}



uint8_t SevenSegment(uint8_t digit)	//function that gives output for 7-segment common-anode display corresponding to the digit passed as parameter to it

{
	uint8_t out = 0;
	
	switch(digit)
	
	{
		case 0: out = ~0x01;
		break;
		case 1: out = ~0x4F;
		break;
		case 2: out = ~0x12;
		break;
		case 3: out = ~0x06;
		break;
		case 4: out = ~0x4C;
		break;
		case 5: out = ~0x24;
		break;
		case 6: out = ~0x20;
		break;
		case 7: out = ~0x0F;
		break;
		case 8: out = ~0x00;
		break;
		case 9: out = 0b01111011;
		break;	
	}
	
	return out;
}

uint8_t SevenSegment_PA(uint8_t digit)
{
	uint8_t out = 0;
	
	switch(digit)
	{
		case 0: out = 0b11111100;
		break;
		case 1: out = 0b01100000;
		break;
		case 2: out = 0b11011010;
		break;
		case 3: out = 0b11110010;
		break;
		case 4: out = 0b01100110;
		break;
		case 5: out = 0b10110110;
		break;
		case 6: out = 0b10111110;
		break;
		case 7: out = 0b11100000;
		break;
		case 8: out = 0b11111110;
		break;
		case 9: out = 0b11110110;
		break;
	}
	
	return out;
}

void USART_Transmit(uint8_t data)	//function for transmitting char
{
	// Wait for empty transmit buffer
	while (!(UCSRA & (1 << UDRE)));
	
	// Put data into buffer, sends the data
	UDR = data;
}

void USART_Transmit_String(char* str)	//function for transmitting string
{
	// Send each character of the string over USART
	for (int i = 0; str[i] != '\0'; i++)
	{
		USART_Transmit(str[i]);	
	}
}



uint8_t moving_average(uint8_t input)

{
	int sum = 0;	//using int datatype for sum because it may exceed 256

	for(uint8_t i = AVG_RATE - 1; i > 0; i--)
	
	{
		in[i] = in[i - 1];
	}
	
	in[0] = input;
	
	for(uint8_t i = 0; i < AVG_RATE; i ++)
	{
		sum += in[i];
	}
	
	return sum / AVG_RATE;
}

uint8_t linear_regression(uint8_t input)
{
	return ( (M * input) + C);
}