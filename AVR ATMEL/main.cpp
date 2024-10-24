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

#define TEMPERATURE_TRY_COUNTER 5

#define ADC_MULTIPLIER 0.013

#define ADC_CHANNELS 8

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

struct temperatures_t
{
	float value;
	bool  isActual;	
};
			   
static temperatures_t temperatures[SENSORS_COUNT] = {0};

static control_units units = {0};

static bool measureProccessed = false;
static bool timerTick = false;

void get_temperature_data();

void send_state(bool state);

void execute_command(char command);

void init_control_pins();

void init_timer();

void clear_temperature_data();

void send_telemetry();

uint8_t get_adc_value(uint8_t i);

void init_adc();

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
	
	init_adc();
	
	sei();
	
	set_sleep_mode(SLEEP_MODE_IDLE);
	
	sleep_enable();
		
	while(1)
	{
	
		sleep_cpu();


		if( timerTick )
		{
			
			wdt_reset();
			
			if( measureProccessed )
			{
				get_temperature_data();
				measureProccessed = false;
			}
			else
			{
				
				for( uint8_t try_counter = 0 ; try_counter < TEMPERATURE_TRY_COUNTER; try_counter++ )
				{
					if ( ! DS18B20::measure() )
					{
						show_error(ERROR_TEMPERATURE_SENSORS_RESET_FAILURE);
					}
					else
					{
						measureProccessed = true;
						break;
					}
				}
				
				if( !measureProccessed )
				{
					clear_temperature_data();
				}
				
			}

			timerTick = 0;
			
		}

		if( !Uart::is_ready_read() )
		{
			continue;
		}
				
		execute_command( Uart::read_byte() );
		
	}
	
}

/**
	@brief 
*/

ISR(TIMER1_COMPA_vect)
{
	
	timerTick = 1;
	
	// Уменьшение значения счетчиков защиты
	
	if( units.ventilation.counter != 0 )
	{
		units.ventilation.counter -= 1;
	}
	
	if( units.heating.counter != 0 )
	{
		units.heating.counter -= 1;
	}
	
	if( units.light.counter != 0 )
	{
		units.light.counter -= 1;
	}
	
	if( units.reserved0.counter != 0 )
	{
		units.reserved0.counter -= 1;
	}
	
	if( units.reserved1.counter != 0 )
	{
		units.reserved1.counter -= 1;
	}
	
	if( units.reserved2.counter != 0 )
	{
		units.reserved2.counter -= 1;
	}
	
}

// Выключение немедленное

void off( control_unit_t* unit, uint8_t pinToOff )
{
	
	if( CONTROL_PIN & (1<<pinToOff) )
	{
		send_state(ERROR); // Нельзя исполнить команду, так как уже отключено
		return;
	}
	
	if( unit->isBad )
	{
		send_state(ERROR); // Ножка повреждена и находится в отключенном состоянии
		return;	
	}
		
	CONTROL_PORT &= ~(1 << pinToOff);
	
	_delay_us(100);
	
	if( CONTROL_PIN & (1 << pinToOff) )
	{
		// Не отключился!!!
		CONTROL_DDR &= ~(1<<pinToOff);
		unit->isBad  = true;
		show_error(ERROR_PIN_SHORTED_TO_GND);
		send_state(ERROR);
	}
	else
	{
		send_state(OK);
	}
	
}

// Включение только через запрос!

void on( control_unit_t* unit, uint8_t pinToOn, uint8_t safeTime )
{
	
	if( CONTROL_PIN & (1<<pinToOn) )
	{
		send_state(ERROR); // Нельзя исполнить команду, так как уже включено
	}
	else
	{
		
		asm("cli");
		
		if( unit->counter != 0 )
		{
			asm("sei");
			send_state(ERROR); // Нельзя включить, слишком частый запрос
			return;
		}
		
		unit->counter = safeTime;
		
		asm("sei");
		
		CONTROL_PORT |= (1<<pinToOn);
		
		_delay_us(100);
		
		if( CONTROL_PIN & (1<<pinToOn) )
		{
			send_state(OK);
		}
		else
		{
			// Не включился!!!
			CONTROL_DDR &= ~(1<<pinToOn);
			unit->isBad  = true;
			show_error(ERROR_PIN_SHORTED_TO_GND);
			send_state(ERROR);
		}
		
	}

}

void execute_command(char command)
{
	
	// Включение только по-запросу, из таймера, выключение немедленное

	if( command == '1' )
	{
		on( &(units.ventilation), VENTILATION, VENTILATION_ON_SAFE_TIME_IN_SECONDS );
	}
	else if( command == '2' )
	{
		on( &(units.heating), HEATING, HEATING_ON_SAFE_TIME_IN_SECONDS );
	}
	else if( command == '3' )
	{
		on( &(units.light), LIGHT, LIGHT_ON_SAFE_TIME_IN_SECONDS );
	}
	else if( command == '4' )
	{
		on( &(units.reserved0), RESERVED_0, RESERVED_0_ON_SAFE_TIME_IN_SECONDS );
	}
	else if( command == '5' )
	{
		on( &(units.reserved1), RESERVED_1, RESERVED_1_ON_SAFE_TIME_IN_SECONDS );
	}
	else if( command == '6' )
	{
		on( &(units.reserved2), RESERVED_2, RESERVED_2_ON_SAFE_TIME_IN_SECONDS );
	}
	else if( command == 'q' )
	{
		off( &(units.ventilation), VENTILATION );
	}
	else if( command == 'w' )
	{
		off( &(units.heating), HEATING );
	}
	else if( command == 'e' )
	{
		off( &(units.light), LIGHT );
	}
	else if( command == 'r' )
	{
		off( &(units.reserved0), RESERVED_0 );
	}
	else if( command == 't' )
	{
		off( &(units.reserved1), RESERVED_1 );
	}
	else if( command == 'y' )
	{
		off( &(units.reserved2), RESERVED_2 );
	}
	else if( command == 'g' ) // Получение данных температуры и состояния порта
	{
		
		send_telemetry();
		
	}
	
}

void clear_temperature_data()
{
	
	for(int i = 0; i < SENSORS_COUNT; i++)
	{		
		temperatures[i].isActual = false;
	}
	
}

void get_temperature_data()
{
	
	clear_temperature_data();

	for(int i = 0; i < SENSORS_COUNT; i++)
	{
		for(int j = 0; j < TEMPERATURE_TRY_COUNTER; j++) 
		{
			float temperature = 0;
			if( DS18B20::get_temperature(address_sensor[i], &temperature) )
			{
				temperatures[i].value = temperature;
				break;
			}
			else
			{
				show_error(ERROR_TEMPERATURE_SENSORS_GET_MEASURE_FAILURE);
			}
		}
	}
	
}

void send_state(bool state) // 1 - ok, 0 - error
{
	
	if( state )
	{
		Uart :: send("{\"state\":ok}");
	}
	else
	{
		Uart :: send("{\"state\":error}");
	}
		
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
	TCCR1B = (1<<WGM12) | (1<<CS12) | (0<<CS10) | (0<<CS11); // /256
	
	TIMSK |= (1<<OCIE1A);
}

uint8_t get_adc_value(uint8_t i)
{
	
	ADMUX = (1<<ADLAR) | i;
	
	ADCSRA |= (1<<ADSC);
	
	while( ! ( ADCSRA & ADIF ) )
	{}
	
	return ADCH;
	
}

void init_adc()
{
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
}

static char tempDiv[24] = {0};

void send_telemetry()
{
	
	tempDiv[0] = '{';
	
	// Телеметрия цифровых выводов
	
	if( CONTROL_PIN & (1<<VENTILATION) )
	{
		strcat(tempDiv, "\"vent\":1,");
	}
	else
	{
		strcat(tempDiv, "\"vent\":0,");
	}
	
	Uart::send(tempDiv);
	
	uint32_t crc = CRC::crc32(0, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<HEATING) )
	{
		strcpy(tempDiv, "\"heat\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"heat\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<LIGHT) )
	{
		strcpy(tempDiv, "\"light\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"light\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<RESERVED_0) )
	{
		strcpy(tempDiv, "\"res0\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"res0\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<RESERVED_1) )
	{
		strcpy(tempDiv, "\"res1\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"res1\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<RESERVED_2) )
	{
		strcpy(tempDiv, "\"res2\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"res2\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	// Данные датчиков температуры
	
	for( uint8_t i = 0 ; i < SENSORS_COUNT; i++ )
	{
		
		tempDiv[0] = '\0';
		
		if( temperatures[i].isActual )
		{
			strcat(tempDiv, "\"t");
		
			itoa( i, &(tempDiv[strlen(tempDiv)]), 10 );
		
			strcat(tempDiv, "\":");
		
			dtostrf( temperatures[i].value, 5, 2, &(tempDiv[strlen(tempDiv)]) );
		
			strcat(tempDiv, ",");
			
			Uart::send(tempDiv);
			
			crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
			
		}
		
	}
	
	// Данные цифровых входов
	
	tempDiv[0] = '\0';
	
	if( DIGITAL_SENSORS_PORT_PIN & (1<<DIGITAL_SENSORS_PIN_0) )
	{
		strcat(tempDiv, "\"d0\":1,");
	}
	else
	{
		strcat(tempDiv, "\"d0\":0,");
	}
	
	if( DIGITAL_SENSORS_PORT_PIN & (1<<DIGITAL_SENSORS_PIN_1) )
	{
		strcat(tempDiv, "\"d1\":1,");
	}
	else
	{
		strcat(tempDiv, "\"d1\":0,");
	}
	
	if( DIGITAL_SENSORS_PORT_PIN & (1<<DIGITAL_SENSORS_PIN_2) )
	{
		strcat(tempDiv, "\"d2\":1,");
	}
	else
	{
		strcat(tempDiv, "\"d2\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	for(uint8_t i = 0; i < ADC_CHANNELS; i++)
	{

		strcpy( tempDiv, "\"adc" );
		
		itoa( i, &(tempDiv[strlen(tempDiv)]), 10 );
		
		strcat( tempDiv, "\":" );
		
		dtostrf( get_adc_value(i) * ADC_MULTIPLIER , 5, 2, &(tempDiv[strlen(tempDiv)]) );
		
		if( i != ( ADC_CHANNELS - 1 ) )
		{
			strcat( tempDiv, "," );
		}
		else
		{
			strcat(tempDiv, "}");
		}
	
		Uart::send(tempDiv);
	
		crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	}

	strcpy(tempDiv, "{\"crc\":");
	
	itoa(crc, &(tempDiv[strlen(tempDiv)]) ,16);
	
	strcat(tempDiv,"}");
	
	Uart::send(tempDiv);
	
}