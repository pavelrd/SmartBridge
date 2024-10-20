#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "uart.h"
#include "temp.h"
#include "led.h"

#include "pins.h"
#include "user_types.h"
#include "user_error.h"
#include "crc.hpp"

#define OK    0
#define ERROR 1

#define SENSORS_COUNT 7

static const uint64_t address_sensor [SENSORS_COUNT] = 
{ 
	0x4d0215C2C64FFF28, // улица
	0xC50215C2C664FF28, // подвал
	0xF40215C2B504FF28, // электрощиток 1
	0xF40215C2B504FF28, // электрощиток 2
	0xF40215C2B504FF28, // блок управления
	0xF40215C2B504FF28, // помещение 1
	0xF40215C2B504FF28  // помещение 2
};
							   
static float temperatures[SENSORS_COUNT] = {0};

static requests_t requests = { 0 };

static counters_t counters = {0};

void get_temperature_data();

void send_state();

void execute_command(char command);

void init_control_pins();

void init_timer();

int main(void)
{

	init_error_messaging();

	check_reset_state();
			
	wdt_reset();

	wdt_enable(WDTO_2S);
		
	init_control_pins();
	
	DS18B20::init_temp(TEMPERATURE);
	
	Uart :: init( false, Uart::BAUD_9600 );
	
	init_timer();
	
	sei();
	
	set_sleep_mode(SLEEP_MODE_IDLE);
	
	sleep_enable();
		
	while(1)
	{
		
		sleep_cpu();


		if( requests.timerTick )
		{
			
			wdt_reset();
			

			if( requests.measureProccessed )
			{
				get_temperature_data();
				requests.measureProccessed = false;
			}
			else
			{
				if ( ! DS18B20::measure() )
				{
					show_error(ERROR_TEMPERATURE_SENSORS_RESET_FAILURE);
				}
				requests.measureProccessed = true;
			}

				
			requests.timerTick = 0;
			
		}

		if( !Uart::is_ready_read() )
		{
			continue;
		}
				
		execute_command( Uart::read_byte() );
		
	}
	
}


inline bool on_if_need( bool isOnRequested, uint8_t* safeCounter, uint8_t pinToOn, uint8_t safeInterval )
{
	
	// ------- Уменьшение значения счетчика защиты от преждевременного повторного включения

	if( *safeCounter != 0 )
	{
		*safeCounter -= 1;
	}
	
	// ------- Включение, если требуется
	//          включение происходит только при запросе и счетчике защиты в значении 0
	
	if( ( isOnRequested ) && ( *safeCounter == 0 ) )
	{
		CONTROL_PORT |= 1 << pinToOn;
		*safeCounter = safeInterval;
		return 0;
	}
	else
	{
		return 1;
	}
	
}

/**
	@brief 
*/

ISR(TIMER1_COMPA_vect)
{
	
	requests.timerTick = 1;

	// ------- Обработка запросов на включение
	
	requests.ventilation = on_if_need( requests.ventilation, &(counters.vent),      VENTILATION, VENTILATION_ON_SAFE_TIME_IN_SECONDS );
	requests.heating     = on_if_need( requests.heating,     &(counters.heat),      HEATING,     HEATING_ON_SAFE_TIME_IN_SECONDS     );
	requests.light       = on_if_need( requests.light,       &(counters.light),     LIGHT,       LIGHT_ON_SAFE_TIME_IN_SECONDS       );
	requests.reserved0   = on_if_need( requests.reserved0,   &(counters.reserved0), RESERVED_0,  RESERVED_0_ON_SAFE_TIME_IN_SECONDS  );
	requests.reserved1   = on_if_need( requests.reserved1,   &(counters.reserved1), RESERVED_1,  RESERVED_1_ON_SAFE_TIME_IN_SECONDS  );
	requests.reserved2   = on_if_need( requests.reserved2,   &(counters.reserved2), RESERVED_2,  RESERVED_2_ON_SAFE_TIME_IN_SECONDS  );

}

void execute_command(char command)
{
	
	// Включение только по-запросу, из таймера, выключение немедленное

	if( command == '1' )
	{
		requests.ventilation = true;
		send_state();
	}
	else if( command == '2' )
	{
		requests.heating = true;
		send_state();
	}
	else if( command == '3' )
	{
		requests.light = true;
		send_state();
	}
	else if( command == '4' )
	{
		requests.reserved0 = true;
		send_state();
	}
	else if( command == '5' )
	{
		requests.reserved1 = true;
		send_state();
	}
	else if( command == '6' )
	{
		requests.reserved2 = true;
		send_state();
	}
	else if( command == 'q' )
	{
		asm("cli"); 
		requests.ventilation = 0; // CLI чтобы по прерыванию не включилось вновь после этого
		CONTROL_PORT &= ~(1 << VENTILATION); // и тут же мгновенно выключилось вот здесь
		asm("sei");
		send_state();
	}
	else if( command == 'w' )
	{
		asm("cli");
		requests.heating = 0;
		CONTROL_PORT &= ~(1 << HEATING);
		asm("sei");
		send_state();
	}
	else if( command == 'e' )
	{
		asm("cli");
		requests.light = 0;
		CONTROL_PORT &= ~(1 << LIGHT);
		asm("sei");
		send_state();
	}
	else if( command == 'r' )
	{
		asm("cli");
		requests.reserved0 = 0;
		CONTROL_PORT &= ~(1 << RESERVED_0);
		asm("sei");
		send_state();
	}
	else if( command == 't' )
	{
		asm("cli");
		requests.reserved1 = 0;
		CONTROL_PORT &= ~(1 << RESERVED_1);
		asm("sei");
		send_state();
	}
	else if( command == 'y' )
	{
		asm("cli");
		requests.reserved2 = 0;
		CONTROL_PORT &= ~(1 << RESERVED_2);
		asm("sei");
		send_state();
	}
	else if( command == 'g' ) // Получение данных температуры и состояния порта
	{
		
		Uart::send("{");
			
		char tempDiv[24] = {0};
			
		for( uint8_t i = 0 ; i < SENSORS_COUNT; i++ )
		{
				
			Uart::send("sensor_");
				
			itoa(i, tempDiv, 10);
				
			Uart::send(tempDiv);
				
			Uart::send(":");
				
			dtostrf( temperatures[i], 10, 4, tempDiv );
				
			Uart::send(tempDiv);
				
			Uart::send(",");
				
		}
			
		// send sensors data
			
		if( DIGITAL_SENSORS_PORT_PIN & (1<<DIGITAL_SENSORS_PIN_0) )
		{
			Uart :: send("d0:1,");
		}
		else
		{
			Uart :: send("d0:0,");
		}
			
		if( DIGITAL_SENSORS_PORT_PIN & (1<<DIGITAL_SENSORS_PIN_1) )
		{
			Uart :: send("d1:1,");
		}
		else
		{
			Uart :: send("d1:0,");
		}
			
		if( DIGITAL_SENSORS_PORT_PIN & (1<<DIGITAL_SENSORS_PIN_2) )
		{
			Uart :: send("d2:1,");
		}
		else
		{
			Uart :: send("d2:0,");
		}
			
		Uart::send(",");
		
		ADCSRA = (1<<ADEN) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
		
		for(uint8_t i = 0; i < 8; i++)
		{
			
			char tempDiv[8] = {0};
				
			ADMUX = (1<<ADLAR) | i;
			
			ADCSRA |= (1<<ADSC);
			
			Uart::send("adc_0:");
			
			while( ! ( ADCSRA & ADIF ) )
			{}
			
			// ADCH 
			itoa(i, tempDiv, 10);
			
			Uart::send(",");
			
		}
		
		ADCSRA &= (1<<ADEN);
		
		Uart::send("}");
		
	}
	
}

void get_temperature_data()
{
	for(int i = 0; i < SENSORS_COUNT; i++)
	{
		
		float temperature = 0;
		
		temperatures[i] = -255;
		
		for(int j = 0; j < 3; j++)
		{
			if( DS18B20::get_temperature(address_sensor[i], &temperature) )
			{
				temperatures[i] = temperature;
				break;
			}
		}
	}
}

void send_state()
{
	
	Uart :: send("{state:ok}");

}

void init_control_pins()
{
	CONTROL_PORT &= ~( (1 << VENTILATION) | (1 << HEATING) | (1 << LIGHT) | (1<<RESERVED_0) | (1<<RESERVED_1) | (1<<RESERVED_2) );

	CONTROL_DDR  |= ( 1 << VENTILATION ) | (1 << HEATING) | (1 << LIGHT) | (1<<RESERVED_0) | (1<<RESERVED_1) | (1<<RESERVED_2);
}

void init_timer()
{
	TCNT1  = 0x0000;
	
	OCR1A   = 15625;
	
	TCCR1A = 0;
	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS11);
	
	TIMSK |= (1<<OCIE1A);
}