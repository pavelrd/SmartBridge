/*
 * temp.cpp
 *
 * Created: 13.02.2024 9:00:51
 *  Author: Pavel
 */ 

#include <util/delay.h>
#include <avr/io.h>
#include "temp.h"
#include "uart.h"

#include <stdlib.h>

#define TEMP_PORT_DDR  DDRB
#define TEMP_PORT_PORT PORTB
#define TEMP_PORT_PIN  PINB

uint8_t DS18B20::pinNumber = 0;

void DS18B20 :: init_temp( uint8_t _pinNumber )
{
	pinNumber = _pinNumber;
	TEMP_PORT_DDR &= ~(1 << pinNumber);
	TEMP_PORT_PORT &= ~(1 << pinNumber);
}

bool DS18B20 :: checkready_temperature()
{
	
	// ��������� ��� �� ����� ��� �� �� �����(�� ����)
	
	if( ! ( TEMP_PORT_PIN & (1<<pinNumber) ) )
	{
		return false;
	}
	
	TEMP_PORT_DDR |= 1 << pinNumber; // ������������� ���� � ����
	_delay_us(490);
	TEMP_PORT_DDR &= ~(1 << pinNumber); // �������e� ����
	_delay_us(70);
	
	if( ! ( PINB & (1 << pinNumber) ) ) // ���� � ���� ����� ����������, ���� ����������
	{	
		_delay_ms(1);
		return true;
	}
	
	_delay_ms(1);
	return false;
	
}

void DS18B20 :: write_bit(uint8_t bit)
{
	switch(bit)
	{
		case 0: TEMP_PORT_DDR |= 1 << pinNumber; _delay_us(60); TEMP_PORT_DDR &= ~(1 << pinNumber); break;
		case 1: TEMP_PORT_DDR |= 1 << pinNumber; _delay_us(15); TEMP_PORT_DDR &= ~(1 << pinNumber); _delay_us(45); break;
	}
	_delay_us(10);
}

void DS18B20 :: write_byte (uint8_t byte)
{
	for(int i = 0; i < 8; i++)
	{
		// (byte & 0x01) ? write_bit(1) : write_bit(0);
		if(byte & 0x01)
		{
			write_bit(1);
		}
		else
		{
			write_bit(0);
		}
		byte >>= 1;
	}
}


uint8_t DS18B20 :: read_bit()
{
	TEMP_PORT_DDR |= 1 << pinNumber;
	_delay_us(2);
	TEMP_PORT_DDR &= ~(1 << pinNumber);
	_delay_us(4);
	
	if(PINB & 1<< pinNumber)
	{
		_delay_us(61);
		return 1;
	}
	else
	{
		_delay_us(61);
		return 0;
	}
}

uint8_t DS18B20 :: read_byte()
{
	uint8_t read_byte = 0;
	
	for(int i = 0; i < 8; i++)
	{
		read_byte >>= 1; // 1000 0000
		if(read_bit())
		{
			read_byte |= 0x80;
		}
	}
	return read_byte;
}

bool DS18B20::measure()
{
	
	// ������ ��������� �����������
	
	if ( checkready_temperature() )
	{
		
		write_byte(SKIP_ROM);
		
		write_byte(CONVERT_TEMPERATURE);
		
		return true;
		
	}
	else 
	{
		
		return false;
		
	}
	
	/*
	while( !read_bit() )
	{
		_delay_ms(50);
	}
	*/
	
}

uint8_t DS18B20::crc8( uint8_t* data, uint8_t length )
{
	
    uint8_t crc = 0;

    while( length-- )
    {

	    uint8_t inbyte = *data++;

	    for( uint8_t i = 8 ; i > 0 ; i-- )
	    {
		    uint8_t mix = (crc ^ inbyte) & 0x01;
		    crc >>= 1;
		    if(mix) crc ^= 0x8C;
		    inbyte >>= 1;
	    }
    }

    return crc;
	
}

/**

	@brief �������� �������� �����������
	
	@param address     - ����� �������
	@param temperature - �������� �����������
	
	return 1 - �������� ����������� �������� �������, 0 - ������
	 
*/

bool DS18B20 :: get_temperature(uint64_t address, float *temperature)
{
	
	// ��������� �������� �����������
	
	uint8_t scratchpad[9] = {0};
	
	for(int try_count = 0; try_count < 4; try_count++) // ��������� ������, 4 ����, ����� ������������ �������� ������ ���� 1 � 2^32. ��� ��� ����������� ����� ��� �������� �� �������
	{                                                  // 2^8
		
		if ( !checkready_temperature() )
		{
			break;
		}
		
		if( address == 0 )
		{
			write_byte(SKIP_ROM);
		}
		else
		{
			write_byte(MATCH_ROM);
			for(int i = 0; i < 8 ; i++)
			{
				write_byte(address & 0xFF);
				address >>= 8;
			}
		}
		
		write_byte(READ_SCRATCHPAD);
		
		if( try_count == 0 )
		{
			
			Uart::send("\r\nFirst scratchpad: ");
			
			for( int i = 0; i < 9; i++ )
			{
				scratchpad[i] = read_byte();
				
				char str[16] = {0};
					
				itoa(scratchpad[i], str, 16);
				Uart::send( str );
				Uart::send( " " );
				
			}
			
			Uart::send(" \r\n");

			if( crc8(scratchpad, 9) != 0 )
			{
				return 0;
			}
		}
		else
		{
			
			Uart::send("Second scratchpad: ");
	
			for(int j = 0; j < 9; j++)
			{
							
				uint8_t readed_byte = read_byte() ;
			
				char str[16] = {0};
					
				itoa(readed_byte, str, 16);
				Uart::send( str );
				Uart::send(" ");
				
			}
			
			Uart::send("\r\n");
		}
	}
		
	uint8_t first_byte  = scratchpad[0]; // ls byte
	uint8_t second_byte = scratchpad[1]; // ms byte
		
	*temperature = ((first_byte >> 4) | (second_byte << 4)) + (first_byte & 0x0F) * 0.0625;
	
	if( ( *temperature < -55 ) || ( *temperature > 125 ) )
	{
		return 0;
	}
	
	return 1;
	
}

void DS18B20::set_accuracy(  enum DS18B20::ACCURACY accuracy )
{
			
			
}

uint64_t DS18B20::get_address()
{
	checkready_temperature();
	write_byte(READ_ROM);
	
	uint64_t address = 0; 
	for(uint8_t i = 0; i < 8; i++)
	{
		address <<= 8;
		address |= read_byte();  	
	}
	return address; 
}