/*
 * uart.cpp
 *
 * Created: 17.01.2024 14:17:25
 *  Author: Pavel
 */ 

#include <avr/io.h>
#include "uart.h"

#include <util/delay.h>
#include <stdlib.h>

#include <avr/interrupt.h>

#define UART_DDR  DDRD
#define UART_PORT PORTD
#define UART_PIN  PIND

#define TX_PIN PD1
#define RX_PIN PD0

#define UART_SPEED 9600

#define UART_ONE_BIT_LENGTH_US ((1.0/UART_SPEED)*1000000)

bool Uart :: isProgram = false; 

static bool    byteRecieved = false;
static uint8_t bufferValue  = 0x00;

void Uart :: init(bool isProgram_, SPEED speed )
{
	
	isProgram = isProgram_; 
	
	if( isProgram )
	{
		UART_DDR |= 1 << TX_PIN;
		UART_PORT |= 1 << TX_PIN;
	
		UART_DDR &= ~(1 << RX_PIN);
		UART_PORT |= 1 << RX_PIN;
	}
	else
	{	
		
		uint16_t value = 100;
		
		if(speed == BAUD_115200)
		{
			value  = 10;
			UCSR0A |= (1<<U2X0); // double speed	
		}
		else if (speed == BAUD_9600)
		{
			value = 64; // (F_CPU / ((uint32_t)16*9600) ) - 1;
		}
		else 
		{
			return;
		}
		
		UBRR0H = value >> 8;
		UBRR0L = value; // 65
		
		// UCSRA = 0;
		
		UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
		
		// UCSRC = 0;
		// UCSRC = (1<<URSEL)|(0<<USBS)|(3<<UCSZ0);
		
	}

}

void Uart :: send_byte (uint8_t byteToSend)
{
	if( isProgram )
	{
		
		UART_PORT &= ~(1 << TX_PIN);
		_delay_us(UART_ONE_BIT_LENGTH_US);
	
		for(int i = 0; i < 8; i++)
		{
			if( byteToSend & 0x01)
			{
				UART_PORT |= 1 << TX_PIN;
			}
			else
			{
				UART_PORT &= ~(1 << TX_PIN);
			}
			_delay_us(UART_ONE_BIT_LENGTH_US);
		
			byteToSend >>= 1;
		}
	
		UART_PORT |= 1 << TX_PIN;
		_delay_us(UART_ONE_BIT_LENGTH_US);
	
	}
	else
	{
		
		while(!(UCSR0A & (1 << UDRE0)))
		{
		}

		UDR0 = byteToSend;
		
	}
	
}

uint8_t Uart :: read_byte()
{
	
	if( isProgram )
	{
			
		while(UART_PIN & (1 << RX_PIN))
		{
		}
		
		_delay_us(UART_ONE_BIT_LENGTH_US/2);
		uint8_t byte_read = 0;
	
		for(int i = 0; i < 8; i++)
		{
			_delay_us(UART_ONE_BIT_LENGTH_US);
			byte_read >>= 1;
			if(UART_PIN & (1 << RX_PIN))
			{
				byte_read |= 0x80;
			}
		}
	
		while(!(UART_PIN & (1 << RX_PIN)))
		{
		}
	
		return byte_read;
	}
	else
	{

		while(!byteRecieved)
		{
		}
		
		byteRecieved = false;
		
		return bufferValue;
		
	}

}

bool Uart::is_ready_read()
{
	return byteRecieved; 
}

void Uart :: send(const char * pArray)
{

	// int strlength = strlen(pArray);
	if( pArray == 0 )
	{
		return;
	}
	
	while( *pArray != '\0' ) //
	{
		send_byte(*pArray);
		pArray++;
	}
}

void Uart :: send (uint64_t byteToSend)
{
	
	uint8_t *pbyte = (uint8_t *)&byteToSend; 
	char save[4] = {0}; 
		
	for(uint8_t i = 0; i < 8; i++)
	{  
		itoa(pbyte[i],save,16);
		send(save);
		send("-");
	}
}

ISR(USART_RXC_vect)
{
	bufferValue  = UDR0;
	byteRecieved = true;
}

uint8_t Uart::buffer[16] = {0};
uint8_t Uart::bufferIndex = 0;
