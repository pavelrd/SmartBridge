/*
 * I_1.h
 *
 * Created: 19.10.2024 15:51:25
 *  Author: user
 */ 


#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define DIGITAL_SENSORS_PORT_PIN PIND

#define DIGITAL_SENSORS_PIN_0 PD4
#define DIGITAL_SENSORS_PIN_1 PD2
#define DIGITAL_SENSORS_PIN_2 PD3

#define CONTROL_PORT PORTB
#define CONTROL_DDR  DDRB

#define VENTILATION PB2
#define HEATING     PB3
#define LIGHT       PB0
#define RESERVED_0  PB4
#define RESERVED_1  PB1
#define RESERVED_2  PB5

// Защитные интервалы времени для повторного включения

// Небольшая нагрузка, менее 1 ампера, подключен к оптрону
#define VENTILATION_ON_SAFE_TIME_IN_SECONDS 180 

#define HEATING_ON_SAFE_TIME_IN_SECONDS     180
#define LIGHT_ON_SAFE_TIME_IN_SECONDS        20
#define RESERVED_0_ON_SAFE_TIME_IN_SECONDS   20
#define RESERVED_1_ON_SAFE_TIME_IN_SECONDS   20
#define RESERVED_2_ON_SAFE_TIME_IN_SECONDS   20


#define TEMPERATURE PB6

#define ERROR_PORT PORTD
#define ERROR_DDR  DDRD
#define ERROR_LED  PD5

#endif /* I]1_H_ */