#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/portpins.h>

#include "Classes/uart.h"
#include "Classes/temp.h"
#include "Classes/led.h"
 
#include "pins.h"
#include "user_types.h"
#include "user_error.h"
#include "crc.hpp"

#define OK    0
#define ERROR 1

#define  ASCII_NUM_START 0x30

static const uint64_t address_sensor [SENSORS_COUNT] = 
{ 
	TEMPETATURE_SENSOR_0_ADDRESS,
	TEMPETATURE_SENSOR_1_ADDRESS,
	TEMPETATURE_SENSOR_2_ADDRESS,
	TEMPETATURE_SENSOR_3_ADDRESS,
	TEMPETATURE_SENSOR_4_ADDRESS,
	TEMPETATURE_SENSOR_5_ADDRESS,
	TEMPETATURE_SENSOR_6_ADDRESS
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

static const float adc_mutlipliers[8] = 
{
	ADC_0_MULTIPLIER,
	ADC_1_MULTIPLIER,
	ADC_2_MULTIPLIER,
	ADC_3_MULTIPLIER,
	ADC_4_MULTIPLIER,
	ADC_5_MULTIPLIER,
	ADC_6_MULTIPLIER,
	ADC_7_MULTIPLIER
};

void get_temperature_data();

void send_state(bool state, const char* const description=0);

void execute_command(char command);

void init_control_pins();

void init_timer();

void clear_temperature_data();

void send_telemetry();

uint8_t get_adc_value(uint8_t i);

void init_adc();
  
void unsignedIntegerToString( char *str, uint32_t num );

// 
// C:/MicrocontrollerLibrary/utilites/avrdude-v7.2-windows-x64/avrdude.exe -c usbasp -p m16 -B 125kHz -U flash:w:"C:/Users/user/Desktop/Для github/SmartBridge/AVR ATMEL/Release/SmartBridge.hex":i
int main(void)
{
	init_error_messaging();

	check_reset_state();
			
	wdt_reset();

	wdt_enable(WDTO_2S); // WDTO_4S WDTO_8S
		
	init_control_pins();
	
	DS18B20::init_temp(TEMPERATURE);
	
	Uart :: init( false, Uart::BAUD_9600 );
	
	init_timer();
	
	init_adc();
	
	Uart::send("Program started\r\n");
	
	char st[10] = {0};
	
	itoa(get_last_reset_reasons(), st, 10);
	
	Uart::send(st);
	
	Uart::send("\r\n");
	
	sei();
	
	set_sleep_mode(SLEEP_MODE_IDLE);
	
	sleep_enable();
	
	DDRD |= (1<<PD3);

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
		
		//
		// {\"command\": ventON, \"checksum\": 0xA1B2C3D4}
		// 
		//
		
		if( !Uart::is_ready_read() )
		{
			continue;
		}
				
		execute_command( Uart::read_byte() );
		
	}
	
}

static bool flag = false;
	
/**
	@brief 
*/

ISR(TIMER1_COMPA_vect)
{
	
	if(flag)
	{
		PORTD |= (1<<PD3);
		flag = false;
	}
	else
	{
		PORTD &= ~(1<<PD3);
		flag = true;
	}
			
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

void off( control_unit_t* unit, uint8_t pinToOff, uint8_t safeTime )
{
		
	if( ! ( CONTROL_PIN & (1<<pinToOff) ) )
	{
		send_state(ERROR, "already off"); // Cannot be turned on, because already off / RU: Нельзя включить, слишком частый запрос
		return;
	}
	
	if( unit->isBad )
	{
		send_state(ERROR, "pin corrupted"); // Pin courrupted and disabled / RU: Ножка повреждена и находится в отключенном состоянии
		return;	
	}
	
	CONTROL_PORT &= ~(1 << pinToOff);
	
	_delay_us(100);
	
	if( CONTROL_PIN & (1 << pinToOff) )
	{
		// Не отключился!!!
		CONTROL_DDR &= ~(1<<pinToOff);
		unit->isBad  = true;
		show_error(ERROR_PIN_SHORTED_TO_VCC);
		send_state(ERROR, "shorted to VCC");
	}
	else
	{
		asm("cli");
		unit->counter = safeTime;
		asm("sei");
		send_state(OK, "off pin ok");
	}
	
}

// Включение только через запрос!

void on( control_unit_t* unit, uint8_t pinToOn )
{
	
	if( CONTROL_PIN & (1<<pinToOn) )
	{
		send_state(ERROR, "already on"); // Cannot be turned on, because already on / RU: Нельзя включить, слишком частый запрос
	}
	else if( unit->isBad )
	{
		send_state(ERROR, "pin corrupted"); // Pin courrupted and disabled / RU: Ножка повреждена и находится в отключенном состоянии
		return;
	}
	else 
	{
		
		asm("cli");
		
		if( unit->counter != 0 )
		{
			asm("sei");
			send_state(ERROR, "too often"); // Cannot be turned on, because too often request / RU: Нельзя включить, слишком частый запрос
			return;
		}
				
		asm("sei");
				
		CONTROL_PORT |= (1<<pinToOn);
		
		_delay_us(100);
		
		if( CONTROL_PIN & (1<<pinToOn) )
		{
			send_state(OK, "on pin ok");
		}
		else
		{
			// Не включился!!!
			CONTROL_DDR &= ~(1<<pinToOn);
			unit->isBad  = true;
			show_error(ERROR_PIN_SHORTED_TO_GND);
			send_state(ERROR, "shorted to GND");
		}
		
	}

}

void execute_command(char command)
{
	
	if( command == '1' )
	{
		on( &(units.ventilation), CONTROL_0_PIN );
	}
	else if( command == '2' )
	{
		on( &(units.heating), CONTROL_1_PIN );
	}
	else if( command == '3' )
	{
		on( &(units.light), CONTROL_2_PIN );
	}
	else if( command == '4' )
	{
		on( &(units.reserved0), CONTROL_3_PIN );
	}
	else if( command == '5' )
	{
		on( &(units.reserved1), CONTROL_4_PIN );
	}
	else if( command == '6' )
	{
		on( &(units.reserved2), CONTROL_5_PIN );
	}
	else if( command == 'q' )
	{
		off( &(units.ventilation), CONTROL_0_PIN, CONTROL_0_SAFE_TIME );
	}
	else if( command == 'w' )
	{
		off( &(units.heating), CONTROL_1_PIN, CONTROL_1_SAFE_TIME );
	}
	else if( command == 'e' )
	{
		off( &(units.light), CONTROL_2_PIN, CONTROL_2_SAFE_TIME );
	}
	else if( command == 'r' )
	{
		off( &(units.reserved0), CONTROL_3_PIN, CONTROL_3_SAFE_TIME );
	}
	else if( command == 't' )
	{
		off( &(units.reserved1), CONTROL_4_PIN, CONTROL_4_SAFE_TIME );
	}
	else if( command == 'y' )
	{
		off( &(units.reserved2), CONTROL_5_PIN, CONTROL_5_SAFE_TIME );
	}
	else if( command == 'g' )
	{
		
		send_telemetry();
		
	}
	else if( command == 'a' )
	{
		
		char st[10] = {0};
		
		itoa(get_last_reset_reasons(), st, 10);
		
		Uart::send("{\"last_reset_reasons\":");
		
		Uart::send(st);
		
		Uart::send(", \"reset_counter\":");
		
		itoa(get_reset_counter(), st, 10);
		
		Uart::send(st);
		
		Uart::send("}\r\n");
		
	}
	else
	{
		send_state(ERROR, "bad command");
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

void send_state(bool state, const char * const description) // 1 - ok, 0 - error
{

	if(state == OK)
	{
		Uart::send("{\"state\":\"ok\"");
	}
	else
	{
		Uart::send("{\"state\":\"error\"");
	}

	if(description)
	{
		Uart :: send(",\"message\":\"");
		Uart :: send(description);
		Uart :: send("\"}\r\n");
	}
	else
	{
		Uart :: send("}\r\n");
	}

}

void init_control_pins()
{
	CONTROL_PORT &= ~( (1 << CONTROL_0_PIN) | (1 << CONTROL_1_PIN) | (1 << CONTROL_2_PIN) | (1<<CONTROL_3_PIN) | (1<<CONTROL_4_PIN) | (1<<CONTROL_5_PIN) );

	CONTROL_DDR  |= ( 1 << CONTROL_0_PIN ) | (1 << CONTROL_1_PIN) | (1 << CONTROL_2_PIN) | (1<<CONTROL_3_PIN) | (1<<CONTROL_4_PIN) | (1<<CONTROL_5_PIN);
}

void init_timer()
{
	TCNT1  = 0x0000;
	
	OCR1A   = F_CPU / 1024;
	
	TCCR1A = 0;
	TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10) | (0<<CS11); // /1024
	
	TIMSK |= (1<<OCIE1A);
}

uint8_t get_adc_value(uint8_t i)
{
	
	ADMUX = (1<<ADLAR) | i;
	
	ADCSRA |= (1<<ADSC);
	
	while( ! ( ADCSRA & ADIF ) )
	{}
	
	uint8_t adc_result = ADCH;
	
	return adc_result;
	
}

void init_adc()
{
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
}

static char tempDiv[24] = {0};

void send_telemetry()
{

    strcpy(tempDiv, "{");
	
	// Телеметрия цифровых выводов
	
	if( CONTROL_PIN & (1<<CONTROL_0_PIN) )
	{
		strcat(tempDiv, "\"");
		strcat(tempDiv, CONTROL_0_NAME);
		strcat(tempDiv,"\":1,");
	}
	else
	{
		strcat(tempDiv, "\"");
		strcat(tempDiv, CONTROL_0_NAME);
		strcat(tempDiv,"\":0,");
	}
	
	Uart::send(tempDiv);
	
	uint32_t crc = CRC::crc32(0, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<CONTROL_1_PIN) )
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_1_NAME);
		strcat(tempDiv,"\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_1_NAME);
		strcat(tempDiv,"\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<CONTROL_2_PIN) )
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_2_NAME);
		strcat(tempDiv,"\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_2_NAME);
		strcat(tempDiv,"\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<CONTROL_3_PIN) )
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_3_NAME);
		strcat(tempDiv,"\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_3_NAME);
		strcat(tempDiv,"\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<CONTROL_4_PIN) )
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_4_NAME);
		strcat(tempDiv,"\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_4_NAME);
		strcat(tempDiv,"\":0,");
	}
	
	Uart::send(tempDiv);
	
	crc = CRC::crc32(crc, (const uint8_t*) tempDiv, strlen(tempDiv));
	
	if( CONTROL_PIN & (1<<CONTROL_5_PIN) )
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_5_NAME);
		strcat(tempDiv,"\":1,");
	}
	else
	{
		strcpy(tempDiv, "\"");
		strcat(tempDiv, CONTROL_5_NAME);
		strcat(tempDiv,"\":0,");
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
		
		dtostrf( get_adc_value(i) * adc_mutlipliers[i], 5, 2, &(tempDiv[strlen(tempDiv)]) );
		
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

	strcpy(tempDiv, "\r\n{\"crc\":");
	
	unsignedIntegerToString( &(tempDiv[strlen(tempDiv)]), crc );
	
	strcat(tempDiv,"}\r\n");
	
	Uart::send(tempDiv);

}

void unsignedIntegerToString( char *str, uint32_t num )
{

	uint8_t pt = 0;

	uint8_t flag = 0;

	if( num >= 1000000000 )
	{
		flag = 1;
		uint8_t dec = 0;
		while(num >= 1000000000)
		{
			num -= 1000000000;
			dec += 1;
		}
		str[pt] = dec;
		str[pt] += ASCII_NUM_START;
		pt += 1;
	}
	
	if( 1 == flag || num >= 100000000 )
	{
		flag = 1;
		uint8_t dec = 0;
		while(num >= 100000000)
		{
			num -= 100000000;
			dec += 1;
		}
		str[pt] = dec;
		str[pt] += ASCII_NUM_START;
		pt += 1;
	}

	if( 1 == flag || num >= 10000000 )
	{
		flag = 1;
		uint8_t dec = 0;
		while(num >= 10000000)
		{
			num -= 10000000;
			dec += 1;
		}
		str[pt] = dec;
		str[pt] += ASCII_NUM_START;
		pt += 1;
	}

	if( 1 == flag || num >= 1000000 )
	{
		flag = 1;
		uint8_t dec = 0;
		while(num >= 1000000)
		{
			num -= 1000000;
			dec += 1;
		}
		str[pt] = dec;
		str[pt] += ASCII_NUM_START;
		pt += 1;
	}

	if( 1 == flag || num >= 100000 )
	{
		flag = 1;
		uint8_t dec = 0;
		while(num >= 100000)
		{
			num -= 100000;
			dec += 1;
		}
		str[pt] = dec;
		str[pt] += ASCII_NUM_START;
		pt += 1;
	}

	if( 1 == flag || num >= 10000 )
	{
		flag = 1;
		uint8_t dec = 0;
		while(num >= 10000)
		{
			num -= 10000;
			dec += 1;
		}
		str[pt] = dec;
		str[pt] += ASCII_NUM_START;
		pt += 1;
	}

	if( 1 == flag || num >= 1000 )
	{
		flag = 1;
		uint8_t dec = 0;
		while(num >= 1000)
		{
			num -= 1000;
			dec += 1;
		}
		str[pt] = dec;
		str[pt] += ASCII_NUM_START;
		pt += 1;
	}

	if( 1 == flag || num >= 100 )
	{
		flag = 1;
		uint8_t dec = 0;
		while(num >= 100)
		{
			num -= 100;
			dec += 1;
		}
		str[pt] = dec;
		str[pt] += ASCII_NUM_START;
		pt += 1;
	}

	if( 1 == flag || num >= 10 )
	{
		flag = 1;
		uint8_t dec = 0;
		while(num >= 10)
		{
			num -= 10;
			dec += 1;
		}
		str[pt] = dec;
		str[pt] += ASCII_NUM_START;
		pt += 1;
	}

	str[pt] = num;
	str[pt] += ASCII_NUM_START;

	pt += 1;
	
	str[pt] = '\0';

}