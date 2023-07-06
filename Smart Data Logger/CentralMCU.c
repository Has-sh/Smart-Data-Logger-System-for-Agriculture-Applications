#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// maximum buffer sizes for transmission
#define USER_TR_BUFFER_SIZE 128
#define SENSOR_TR_BUFFER_SIZE 5
#define USER_MENU "\rEnter choice (and period):\r1-Mem Dump\r2-Last Entry\r3-Restart\0"
#define SENSOR_RST_INF_MSG "Sensor is Initialized\0"
#define WATCHDOG_CONFIG_ADDR 0x00
#define DEFAULT_WATCHDOG_PERIOD 128
//stack initialization
unsigned char* Stack;
// USART0 communication buffers and indexes
unsigned char user_tr_buffer[USER_TR_BUFFER_SIZE] = "";
unsigned char user_tr_index = 0; // keeps track of character index in buffer
unsigned char new_user_read_char; // reception one character at a time
unsigned char user_rv_buffer[2];//size 2 cuz only have 2 size string 1/2 and a period "."
unsigned char user_rv_index=-1;//-1 as 0 means it must have value currently
// USART1 communication buffers and indexes
unsigned char sensor_tr_buffer[SENSOR_TR_BUFFER_SIZE] = "";
unsigned char sensor_tr_index = 0;//keeps tracks of character index in buffer
unsigned char new_sensor_read_char;//reception one character at a time
// Data memory pointers
unsigned char *x; //= (unsigned char*)0x0500; // used in memory dump
unsigned char *z; //=(unsigned char*) 0x010FF; // points to end of stack(internal memory)
unsigned char *y; //=(unsigned char*) 0x10EA; // points to start of stack
//Temp Variables
unsigned char TOS;
unsigned char G=0b11010100;//G=x^5+x^4+x^2+x^0
unsigned char RESET_COMMAND=0b00000000;//reset request
unsigned char REPEAT_COMMAND=0b01100000;//error/repeat
unsigned char LOGREQUEST_COMMAND=0b00100000;//log Request
unsigned char ACKNOWLEDGE_COMMAND=0b01000000;//Acknowledge
unsigned char packet;
unsigned char  started=0;


void Sleep_and_Wait()
{
    sleep_enable(); // arm sleep mode
    sei(); // global interrupt enable
    sleep_cpu();/// put CPU to sleep
    sleep_disable();
}

// Empty the user transmit buffer using interrupt, saving precious energy // not working with sleep wait
void UserBufferOut(unsigned char msg)
{
    while(!(UCSR0A & (1<<UDRE0)));//check the value of the UDRE0 flag in the UCSR0A register and data reg empty to take value send msg to udr0
    UDR0=msg;//usart data register
}

void MessageSensor(unsigned char msg)//Transmit msg to output
{
    while(!(UCSR1A & (1<<UDRE1)));//check the value of the UDRE1 flag in the UCSR1A register and data reg empty to take value send msg to udr1
    UDR1=msg;//usart data register
}

void MessageUser()//transmit msg from sys to user
{
    unsigned char i,j;
    j=strlen((const char*)user_tr_buffer);//take length of user input and transmit to UDR
    for(i=0; i<j; i++)//null ptr
    {
        UserBufferOut(user_tr_buffer[i]);//transmit value into udr0 reg
    }
}

unsigned char CRC3(unsigned char dataInput)
{
    unsigned char i,temp;
    temp = (0b11100000 & dataInput);//mask input to 3 most significant bit enabled for crc
    for(i=0; i<3; i++)
    {
        if((temp&0b10000000))
        {
            temp = temp ^ G; //G is our crc polynomial
        }
        temp=temp<<1;//shift if temp is 0
    }
    temp=temp>>3;//shift 3 times to make remainder eg 00011111
    return temp;
}

unsigned char CRC3_Check()
{
    unsigned char temp=packet;
    unsigned char temp2=packet;
    temp2=packet;
    temp2=CRC3(temp2);//take crc3
    temp=temp&0b00011111;//mask the packet
    if(temp2==temp) //check if first 5 bits==first five bits in original packet
    {
        return 1;
    }
    return 0;
}

unsigned char CRC11_Check(unsigned char commandInput,unsigned char dataInput)
{
    unsigned char tempCommand,tempData,i,temp;
    tempCommand=commandInput;
    tempData=dataInput;
    tempCommand=tempCommand&0b11100000;

    unsigned short result = ((unsigned short)tempData << 8) | tempCommand;

    for(i=0; i<12; i++)
    {
        if((result&0b1000000000000000))
        {
            result=result^G;
        }
        result=result<<1;
    }
    result=result>>3;
    temp=commandInput;
    temp=temp&0b00011111;
    if((unsigned char)result==temp)
    {
        return 1;
    }
    return 0;
}

void Repeat_Request()
{
    MessageSensor(CRC3(REPEAT_COMMAND));
}

void DataPacketType()
{
    if(TOS>-1) //stackEmpty?
    {
        TOS--;//popStack
    }
    TOS++;//As TOS = -1
    Stack[TOS]=new_sensor_read_char;
}

void CommandPacketType()
{
    unsigned char check;
    if((Stack[TOS]&0b10000000))
    {
        check=CRC11_Check(new_sensor_read_char,Stack[TOS]);
        if(check!=1)
        {
            if((new_sensor_read_char & 0b00100000))
            {
                if(x!=(unsigned char*)0x10EA)
                {
                   *x=new_sensor_read_char;
				   x++;
		    Stack[TOS]= CRC3(ACKNOWLEDGE_COMMAND);
                    MessageSensor(Stack[TOS]);
                }
                else
                {
                    x=(unsigned char*)0x1100;
                   *x=new_sensor_read_char;//should i pop stack
				   x++;
                    Stack[TOS]= CRC3(ACKNOWLEDGE_COMMAND);
                    MessageSensor(Stack[TOS]);
                }
            }
            else
            {
                TOS--;
                Repeat_Request();
            }
        }
    }
    else
    {
        check=CRC3_Check(new_sensor_read_char);
        if(check==1)
        {
            if((new_sensor_read_char&0b01000000))
            {
                if(TOS>=0)
                {
                    TOS--;
                }
            }
            else if((new_sensor_read_char&0b01100000))
            {
                if(TOS>=0)
                {
                    MessageSensor(Stack[TOS]);
                }
            }
        }
        else
        {
            Repeat_Request();
        }
    }
}


void MemoryDump()
{
    y=(unsigned char*)0x0500;
   size_t data_size = sizeof(*y); 
   while((y!=x)&&(y!=z))
    {
       data_size=sizeof(*y);
       memcpy(user_tr_buffer, y, data_size);
       //strcpy(user_tr_buffer, *y);
        MessageUser();
        y++;
    }
    if(y==z){
    y=(unsigned char*)0x1100;
    while(y!=x)//0x1100+2047
    {
       data_size=sizeof(*y);
       memcpy(user_tr_buffer, y, data_size);
       //strcpy(user_tr_buffer, *y);
        MessageUser();
        y++;
    }
    }
    user_rv_index=-1;
    //stack start = 10CE-0x10FF
    //external memory ptr =0x1100
}

void lastEntry()
{
   size_t data_size = sizeof(*x); 
   memcpy(user_tr_buffer, (x-1), data_size);

   //strcpy(user_tr_buffer, *x);
   MessageUser();
   user_rv_index=-1;
}

void UserTrBufferInit()
{
    unsigned char i;
    for(i=0; i<128; i++)
    {
        user_tr_buffer[i]=0;
    }
    user_tr_index=0;
   
}
void SensorTrBufferInit()
{
    unsigned char i;
    for(i=0; i<5; i++)
    {
        sensor_tr_buffer[i]=0;
    }
    sensor_tr_index=0;
}

void Init_RemoteSensor(void)   // A function to initialize remote sensor
{
    // Note how TOS becomes an 8-bit variable in Lab module 3:
    TOS++;
    Stack[TOS] = CRC3(RESET_COMMAND);
    // Send sensor an initialization packet
    MessageSensor(Stack[TOS]);// Be nice enough to inform user about initializing the sensor
    strcpy((char*)user_tr_buffer, SENSOR_RST_INF_MSG);
    MessageUser();
}

// Sample Interrupt handler for USART0 (User side) when a TX is done
ISR(USART0_TX_vect)
{
    if (user_tr_index == 0)  // transmit buffer is emptied
    {
        UserTrBufferInit(); // reinitialize the buffer for next time
        //UCSR0B &= ~((1 << TXEN0) | (1 << TXCIE0)); // disable interrupt
    }
}
// Sample Interrupt handler for USART0 (User side) when an RX is done
ISR(USART0_RX_vect)
{
     while (! (UCSR0A & (1<<RXC0))) {}; // Double checking flag
    new_user_read_char = UDR0;
    user_rv_index++;//user receive value index
    user_rv_buffer[user_rv_index]=new_user_read_char;
     if(user_rv_buffer[user_rv_index]=='.'){
	  if(user_rv_buffer[0]=='1')
            {
               MemoryDump();
            }
            else if (user_rv_buffer[0]=='2')
            {
                lastEntry();
            }
            else if(user_rv_buffer[0]=='3')
            {
                //reset
	       started=0;
            }
	}
}
// Sample Interrupt handler for USART1 (Sensor side) when a TX is done
ISR(USART1_TX_vect)
{
    if (sensor_tr_index == 0)  // transmit buffer is emptied
    {
        SensorTrBufferInit(); // reinitialize the buffer for next time
        //UCSR1B &= ~((1 << TXEN1) | (1 << TXCIE1)); // disable interrupt
    }
}
// Sample Interrupt handler for USART1 (Sensor side) when an RX is done
ISR(USART1_RX_vect)
{
    while (! (UCSR1A & (1<<RXC1))) {}; // Double checking flag
    new_sensor_read_char = UDR1;
    if((new_sensor_read_char&0b10000000))
    {
        DataPacketType();
    }
    else
    {
        CommandPacketType();
    }
}

int main(void)
{
   while(1){
    x=(unsigned char*)0x0500;
    y=(unsigned char*)0x0500;
    z=(unsigned char*)0x10CE;
   Stack=(unsigned char*)(0x1100-50);

    TOS=-1;

    //External Memory Initialization
    MCUCR |= (1 << SRE);
    XMCRB =(1<<XMM1)|(1<<XMM0);

    //re-enable interrupt
    sei();

    //USART INITIALIZATION FOR TWO USART MODULES
    //set the baud rate of USART0 and USART1 to 9600
    UBRR0L=(unsigned char)0x33;
    UBRR1L=(unsigned char)0x33;
    //configure the data frame format
    UCSR0C=(1<<UCSZ00)|(1<<UCSZ01);
    UCSR1C=(1<<UCSZ00)|(1<<UCSZ01);
    //enable the transmitter and receiver for both USART0 and USART1, and also enable the USART receive and transmit complete interrupts
    UCSR0B=(1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<TXCIE0);
    UCSR1B=(1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1);

    /*watchdog timer
    //Read the watchdog period from EEPROM
    unsigned char watchdogPeriod = eeprom_read_word((unsigned char*)WATCHDOG_CONFIG_ADDR);
    if (watchdogPeriod == 0xFFFF)
    {
    	// EEPROM is uninitialized, set default watchdog period
    	watchdogPeriod = DEFAULT_WATCHDOG_PERIOD;
    	// Write the default watchdog period to EEPROM
    	eeprom_busy_wait();
    	eeprom_write_word((uint16_t*)WATCHDOG_CONFIG_ADDR, watchdogPeriod);
    }
    // Enable the watchdog timer with the configured period
   
    wdt_enable(watchdogPeriod);*/
    
    user_rv_index=-1;//user receive value index
    user_rv_buffer[0]="";
    user_rv_buffer[1]="";
    //initialize remote sensor
    UserTrBufferInit();
    SensorTrBufferInit();
    
    Init_RemoteSensor();
    strcpy((char *)user_tr_buffer, USER_MENU);
    MessageUser();
    started=1;
    while (started)
    {
      Sleep_and_Wait();
    }
 }
}