#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "temp.h"
#include "led.h"

// 4d-2-15-c2-c6-4f-ff-28

// c5-2-15-c2-c6-64-ff-28
// f4-2-15-c2-b5-4-ff-28

#define SENSORS_COUNT 3

uint64_t address_sensor [SENSORS_COUNT] = { 0x4d0215C2C64FFF28,
	                            0xC50215C2C664FF28,
	                            0xF40215C2B504FF28
							   };

int main(void)
{
	
	DS18B20::init_temp();
	
	Uart :: init(true, Uart::BAUD_9600); // Uart::BAUD_115200 
	
	Uart :: send("Program started\r\n");

	DS18B20 temp;

	// 0x4d0215C2C64FFF28

	// 4d-2-15-c2-c6-4f-ff-28-

	while(1)
	{
		
		char tempDiv[16];
		
		Uart :: send("Measure temperature\r\n");

		temp.measure();
		
		Uart :: send("Temperature sensors\r\n");
		
		for(int i = 0; i < SENSORS_COUNT; i++)
		{
			
			float temperature = 0;
			for(int j = 0; j < 3; j++)
			{
				if( temp.get_temperature(address_sensor[i], &temperature) )
				{
					dtostrf( temperature, 10, 4, tempDiv );
					Uart :: send(tempDiv);
					break; 
				}
				else
				{
					if(j == 2)
					{
						Uart::send("---");
					}
					
				}	
			}
			

			Uart :: send(" ,");
		}		
		Uart :: send("\r\n");
		//Uart :: send("Address sensor\r\n");
		//Uart :: send(temp.get_address());
		//Uart :: send("\r\n");
		_delay_ms(1000);
	}
	
}




/*
void led_indicate()
{
	PORTD |= 1 << PD5;
	_delay_ms(1);
	//_delay_us(1666);
	PORTD &= ~(1 << PD5);
	_delay_ms(9);
}

void light_off()
{
	PORTB &= ~(1 << PB5);
	
}

void light_on()
{
	PORTB |= 1 << PB5;
}

int main(void)
{
	
	DDRD  |= 1 << PD5; 
	DDRB  |= 1 << PB5;
	
	DDRD  &= ~(1 << PD3); 
	PORTD |= 1 << PD3; 
	
	bool last_button_state = false; 
	bool light_state = false;
	
	while (1)
	{
		// 1010 1011
		// 0000 1000
		//      1000
		
		// 1010 0011
		// 0000 1000
		// 0000 0000
		bool current_button_state = 0;
		
		if(PIND & (1<<PD3))
		{
			current_button_state = 1; 
		}
		else
		{
			current_button_state = 0; 
		}
		
		
		if(last_button_state == current_button_state)
		{
			_delay_ms(1);
		}
		else
		{ 
			last_button_state = current_button_state;
			if(current_button_state)
			{
				if(light_state)
				{
					light_state = false;
					light_off();
				}
				else
				{
					light_state = true;
					light_on();
				}
				 
			} 
		}
		
		
	}
	
*/

/*
	DDRD |= 1 << PD4;
	DDRD &= ~(1 << PD1); 
	
	PORTD |= 1 << PD1;
	
	int mode = 0; // 1 2 3 
	
    while (1) 
    {
		
			switch(mode)
			{
				case 1 : PORTD |= 1 << PD4; _delay_ms(2); PORTD &= ~(1 << PD4); _delay_ms(8); break; 
				case 2 : PORTD |= 1 << PD4; _delay_ms(4); PORTD &= ~(1 << PD4); _delay_ms(6); break; 
				case 3 : PORTD |= 1 << PD4; _delay_ms(6); PORTD &= ~(1 << PD4); _delay_ms(4); break; 
			}
		
			
			static bool last_button_state = false;
			bool current_button_state     =  PIND & (1 << PD1);

			if( last_button_state != current_button_state )
			{
				
				_delay_ms(150);
				
				if( current_button_state )
				{	
					
					mode = (mode + 1) % 4;	
					
				}
				
				last_button_state = current_button_state;
				
			}
	}*/


