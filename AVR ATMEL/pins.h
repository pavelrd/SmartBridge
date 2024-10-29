/*
 * I_1.h
 *
 * Created: 19.10.2024 15:51:25
 *  Author: user
 */ 


#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

// Pin which the sensors DS18B20 are connected
#define TEMPERATURE PB6

// Number of DS18B20 sensors connected to TEMPERATURE pin
#define SENSORS_COUNT 7

 // Number of retries to read in case of errors
#define TEMPERATURE_TRY_COUNTER 5

// Address DS18B20 sensors connected to TEMPERATURE pin
#define TEMPETATURE_SENSOR_0_ADDRESS 0x4d0215C2C64FFF28
#define TEMPETATURE_SENSOR_1_ADDRESS 0x4d0215C2C64FFF28
#define TEMPETATURE_SENSOR_2_ADDRESS 0x4d0215C2C64FFF28
#define TEMPETATURE_SENSOR_3_ADDRESS 0x4d0215C2C64FFF28
#define TEMPETATURE_SENSOR_4_ADDRESS 0x4d0215C2C64FFF28
#define TEMPETATURE_SENSOR_5_ADDRESS 0x4d0215C2C64FFF28
#define TEMPETATURE_SENSOR_6_ADDRESS 0x4d0215C2C64FFF28

// Number of ADC channels scan, scan started of PC0(ADC0)
#define ADC_CHANNELS 8 // PC0-PC8

// ADC multiplier if voltage divider used
#define ADC_0_MULTIPLIER 0.013
#define ADC_1_MULTIPLIER 0.013
#define ADC_2_MULTIPLIER 0.013
#define ADC_3_MULTIPLIER 0.013
#define ADC_4_MULTIPLIER 0.013
#define ADC_5_MULTIPLIER 0.013
#define ADC_6_MULTIPLIER 0.013
#define ADC_7_MULTIPLIER 0.013

// Port for didital scan
#define DIGITAL_SENSORS_PORT_PIN PIND

#define DIGITAL_SENSORS_PIN_0 PD4
#define DIGITAL_SENSORS_PIN_1 PD2
#define DIGITAL_SENSORS_PIN_2 PD3

// Control port, for send enable/disable signal to relay, transistor, etc.
#define CONTROL_PORT PORTB
#define CONTROL_DDR  DDRB
#define CONTROL_PIN  PINB

#define CONTROL_0_PIN PB2
#define CONTROL_1_PIN PB3
#define CONTROL_2_PIN PB0
#define CONTROL_3_PIN PB4
#define CONTROL_4_PIN PB1
#define CONTROL_5_PIN PB5

#define CONTROL_0_NAME "vent"
#define CONTROL_1_NAME "heat"
#define CONTROL_2_NAME "light"
#define CONTROL_3_NAME "res0"
#define CONTROL_4_NAME "res1"
#define CONTROL_5_NAME "res2"

// Safe time intervals for prevent too often enable in seconds
//  maximum value is 255, 0 - none safetime

#define CONTROL_0_SAFE_TIME 180
#define CONTROL_1_SAFE_TIME 180
#define CONTROL_2_SAFE_TIME  20
#define CONTROL_3_SAFE_TIME  20
#define CONTROL_4_SAFE_TIME  20
#define CONTROL_5_SAFE_TIME  20

#define ERROR_PORT PORTD
#define ERROR_DDR  DDRD
#define ERROR_LED  PD5

#if ( VENTILATION_ON_SAFE_TIME_IN_SECONDS > 255) || (HEATING_ON_SAFE_TIME_IN_SECONDS > 255) || (LIGHT_ON_SAFE_TIME_IN_SECONDS > 255)
	#error Safe time must be lower than 255 seconds
#endif

#if ( RESERVED_0_ON_SAFE_TIME_IN_SECONDS > 255) || (RESERVED_1_ON_SAFE_TIME_IN_SECONDS > 255) || (RESERVED_2_ON_SAFE_TIME_IN_SECONDS > 255)
	#error Safe time must be lower than 255 seconds
#endif

#endif /* I]1_H_ */