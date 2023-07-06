#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#define F_CPU 8000000UL
#define BaudRate 9600
#define Alert "change battery immediately\0"

unsigned char Temperature;
unsigned char Moisture;
unsigned char Water;
unsigned char Battery;
unsigned char count=0;
unsigned char i;

void Sleep_and_Wait()
{
	sleep_enable(); // arm sleep mode
	sei(); // global interrupt enable
	sleep_cpu();/// put CPU to sleep
	sleep_disable();
}

void UserBufferOut(unsigned char msg)
{
	while(!(UCSR0A & (1<<UDRE0)));//check the value of the UDRE0 flag in the UCSR0A register and data reg empty to take value send msg to udr0
	UDR0=msg;//usart data register
}

void Configuration(){
	DDRD=0xFF;//datapin
	DDRC=0xFF;//commandpin
	PORTC=0x00;//command=0x00

	_delay_ms(1);
	PORTD=0x38;//Init LCD 2 line 5x7 matrix
	PORTC|=1;//E=1
	_delay_ms(1);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);//short delay

	_delay_ms(1);//delay 2ms
	PORTD=0x0E;//display on the cursor on
	PORTC|=1;//E=1
	_delay_ms(1);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);//short delay

	_delay_ms(1);
	PORTD=0x01;//clear LCD
	PORTC|=1;//E=1
	_delay_ms(1);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);//short delay

	_delay_ms(1);
	PORTD=0x06;//Shift cursor right
	PORTC|=1;//E=1
	_delay_ms(1);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);//short delay

	//USART INITIALIZATION FOR USART MODULE
	UBRR0L=(unsigned char)0x33;
	//configure the data frame format
	UCSR0B=(1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<TXCIE0);
	UCSR0C=(1<<UCSZ00)|(1<<UCSZ01);
	
	ADMUX = (0<<REFS1)|(1<<REFS0)|(1<<ADLAR);//VCCref,left justified
	ADCSRA =(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);//ADC enable,prescaling 128
	DDRF=0x00;//ADC port

	//timer 1 config
	OCR1A = 0x9895;
	TCCR1A = (0<<WGM10) | (0<<WGM11);//CTC mode
	TCCR1B = (0<<WGM13) | (1<<WGM12)|(1<<CS12) | (0<<CS11) | (1<<CS10);//prescaling 1024
	TIMSK = 1<<OCIE1A;
	TCNT1 = 0x00;

	//motor config
	DDRB&=~(1<<0);//PB0 input pin
	PORTB|=(1<<0);//pull up internal
	DDRA=0xFF;
	i=64;
	OCR0=i;
	TCCR0=0x61;
	DDRB|=(1<<PB4);
}

void water_motor(){
	PORTA=0x01;
	OCR0=((Moisture*255)/1023);
	_delay_ms(5000);
	PORTA=0x00;
}

void inputs(){
	
	unsigned char sensor;
	for (sensor = 0; sensor < 4; sensor++) {
		ADMUX|= (sensor<<MUX0);
		cli();
		ADCSRA|=(1<<ADSC);//start conversion
		while(!(ADCSRA&(1<<ADIF)));
		switch (sensor) {
			case 0:
			Temperature = ADCH;
			break;
			case 1:
			Moisture = ADCH;
			break;
			case 2:
			Water = ADCH;
			break;
			case 3:
			Battery = ADCH;
			break;
		}
	}
	sei();
}

void display(){
	
	unsigned char *buffer,Batteryvolt;
	unsigned char i,j;
	PORTD='T';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD='=';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD=((Temperature/16>10)?55+Temperature/16:48+Temperature/16);
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD=' ';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD='M';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD='=';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD=((Moisture/16>10)?55+Moisture/16:48+Moisture/16);
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD=' ';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD='W';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD='=';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD=((Water/16>10)?55+Water/16:48+Water/16);
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD=' ';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD='B';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD='=';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD=((Battery/16>10)?55+Battery/16:48+Battery/16);
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	PORTD=' ';
	PORTC|=(1<<2);//RS=1
	PORTC|=1;//E=1
	_delay_ms(2);//delay 2 ms
	PORTC &=~(1<<0);//E=0
	_delay_us(100);
	
	Batteryvolt = (Battery / 255) * 2.56;
	if(Batteryvolt<3.2){
		strcpy((char*)buffer,Alert);
		j=strlen((const char*)buffer);
		for(i=0; i<j; i++)
		{
			PORTD=buffer[i];
			PORTC|=(1<<2);//RS=1
			PORTC|=1;//E=1
			_delay_ms(2);//delay 2 ms
			PORTC &=~(1<<0);//E=0
			_delay_us(100);
		}
	}
}

ISR(TIMER1_COMPA_vect)//interrupt every 10 sec
{
	
	count=count+1;
	if((count%2)==0){
		_delay_ms(1);
		PORTD=0xC1;//clear LCD
		PORTC|=1;//E=1
		_delay_ms(1);//delay 2 ms
		PORTC &=~(1<<0);//E=0
		_delay_us(100);//short delay
		
		inputs();
		
		display();
		
		UserBufferOut(Temperature);
		UserBufferOut(Moisture);
		UserBufferOut(Water);
		UserBufferOut(Battery);
		
		water_motor();
	}
}


int main(void)
{
	Configuration();
	while (1)
	{
		Sleep_and_Wait();
	}
}