/*
 * led.cpp
 *
 * Created: 13.02.2024 9:11:55
 *  Author: Pavel
 */ 

#include <avr/io.h>
#include <stdlib.h>

#include "led.h"
#include "uart.h"

#define VENTILATION PD2
#define HEATING     PD3
#define LIGHT       PD4

void Led :: init_led()
{
	
	DDRD |= ( 1 << VENTILATION ) | (1 << HEATING) | (1 << LIGHT);
	
	PORTD |= (1 << VENTILATION) | (1 << HEATING) | (1 << LIGHT);
}

void Led :: send_sensors_data()
{
	//
	uint8_t digitalSensorsState = PINB;
	
	char buf[16];
	
	itoa(digitalSensorsState, buf , 2);
	
	Uart :: send(buf);
	
}

void Led :: led_control()
{
	
	if( !Uart::is_ready_read() )
	{
		return;
	}
	
	uint8_t echo = Uart :: read_byte();
	
	switch(echo)
	{
		case '1': PORTD &= ~(1 << VENTILATION); Uart :: send_byte(echo); break;
		case '2': PORTD &= ~(1 << HEATING);     Uart :: send_byte(echo); break;
		case '3': PORTD &= ~(1 << LIGHT);       Uart :: send_byte(echo); break;
		case 'q': PORTD |= (1 << VENTILATION);  Uart :: send_byte(echo); break;
		case 'w': PORTD |= (1 << HEATING);      Uart :: send_byte(echo); break;
		case 'e': PORTD |= (1 << LIGHT);        Uart :: send_byte(echo); break;
		case 'g': Uart :: send_byte(echo); send_sensors_data(); break;
	}
	
}