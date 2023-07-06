//PORTA=CONTROL PORTB=DATA for LCD
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#define BaudRate 9600
//#define F_CPU 8000000UL

#define KEY_PRT PORTC
#define KEY_DDR DDRC
#define KEY_PIN PINC

unsigned char  new_user_read_char;
unsigned char i=0;

unsigned char keypad[4][3] =
{
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'.', '0', '#'}
};

void Sleep_and_Wait()
{
	sleep_enable(); // arm sleep mode
	sei(); // global interrupt enable
	sleep_cpu();/// put CPU to sleep
	sleep_disable();
}

void Configuration()
{
	DDRB=0xFF;//datapin
	DDRA=0xFF;//commandpin
	PORTA=0x00;//command=0x00

	_delay_ms(1);
	PORTB=0x38;//Init LCD 2 line 5x7 matrix
	PORTA|=1;//E=1
	_delay_ms(1);//delay 2 ms
	PORTA &=~(1<<0);//E=0
	_delay_us(100);//short delay

	_delay_ms(1);//delay 2ms
	PORTB=0x0E;//display on the cursor on
	PORTA|=1;//E=1
	_delay_ms(1);//delay 2 ms
	PORTA &=~(1<<0);//E=0
	_delay_us(100);//short delay

	_delay_ms(1);
	PORTB=0x01;//clear LCD
	PORTA|=1;//E=1
	_delay_ms(1);//delay 2 ms
	PORTA &=~(1<<0);//E=0
	_delay_us(100);//short delay

	_delay_ms(1);
	PORTB=0x06;//Shift cursor right
	PORTA|=1;//E=1
	_delay_ms(1);//delay 2 ms
	PORTA &=~(1<<0);//E=0
	_delay_us(100);//short delay
	
	//USART INITIALIZATION FOR USART MODULE
	UBRR0L=(unsigned char)0x33;
	//configure the data frame format
	UCSR0C=(1<<UCSZ00)|(1<<UCSZ01);
	//enable the transmitter and receiver for both
	UCSR0B=(1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<TXCIE0);

	//keypad init
	KEY_DDR = 0x0F;//rows are outputs
	KEY_PRT = 0xE0;


	//keypad interrupt config
	DDRD &= ~(1 << PD1);
	PORTD |= (1 << PD1);
	EIMSK |= (1 << INT1);
	EICRA |= (1 << ISC11);  // Set interrupt sense control for INT1 (rising edge)
	EICRA &= ~(1 << ISC10); // Clear interrupt sense control for INT1 (falling edge)
	sei();
}

void UART_TX_Message( unsigned char msg)
{
	while(!(UCSR0A &(1<<UDRE0)));
	UDR0=msg;
}

void UART_RX_Message(void)
{
	while(!(UCSR0A &(1<<RXC)));
	new_user_read_char = UDR0;
}

void input()
{
	unsigned char msg;
	if(KEY_PIN!=0xE0)
	{
		PORTC=0xEE;//make first row 0
		if((PINC&(1<<PINC5))==0) //if 1 is pressed
		{
			_delay_ms(3);//debounce;
			msg=keypad[0][0];
		}
		else if((PINC&(1<<PINC6))==0)
		{
			_delay_ms(3);
			msg=keypad[0][1];
		}
		else if((PINC&(1<<PINC7))==0)
		{
			_delay_ms(3);
			msg=keypad[0][2];
		}
		PORTC=0xED;//make first row 0
		if((PINC&(1<<PINC5))==0) //if 1 is pressed
		{
			_delay_ms(3);//debounce;
			msg=keypad[1][0];
		}
		else if((PINC&(1<<PINC6))==0)
		{
			_delay_ms(3);
			msg=keypad[1][1];
		}
		else if((PINC&(1<<PINC7))==0)
		{
			_delay_ms(3);
			msg=keypad[1][2];
		}
		PORTC=0xEB;//make first row 0
		if((PINC&(1<<PINC5))==0) //if 1 is pressed
		{
			_delay_ms(3);//debounce;
			msg=keypad[2][0];
		}
		else if((PINC&(1<<PINC6))==0)
		{
			_delay_ms(3);
			msg=keypad[2][1];
		}
		else if((PINC&(1<<PINC7))==0)
		{
			_delay_ms(3);
			msg=keypad[2][2];
		}
		PORTC=0xE7;//make first row 0
		if((PINC&(1<<PINC5))==0) //if 1 is pressed
		{
			_delay_ms(3);//debounce;
			msg=keypad[3][0];
		}
		else if((PINC&(1<<PINC6))==0)
		{
			_delay_ms(3);
			msg=keypad[3][1];
		}
		else if((PINC&(1<<PINC7))==0)
		{
			_delay_ms(3);
			msg=keypad[3][2];
		}
	}
	UART_TX_Message(msg);
	PORTB=msg;
	PORTA|=(1<<2);//RS=1
	PORTA|=1;//E=1
	_delay_ms(1);//delay 2 ms
	PORTA &=~(1<<0);//E=0
	_delay_us(100);//short delay
	
	_delay_ms(2);//delay 2ms
	PORTB=0x1C;//shift text command not working
	PORTA|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTA &=~(1<<0);//E=0
	_delay_us(100);//short delay
	_delay_ms(550);
}

ISR(USART0_RX_vect)
{
	UART_RX_Message();
	PORTB=new_user_read_char;
	PORTA|=(1<<2);//RS=1
	PORTA|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTA &=~(1<<0);//E=0
	_delay_us(100);
}

ISR(INT1_vect)
{
	input();
}

int main(void)
{
	Configuration();
	while(1)
	{
		Sleep_and_Wait();
	}
	return 0;
}
