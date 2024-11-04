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
//  
#define TEMPERATURE_PORT PORTD
#define TEMPERATURE_PIN  PIND
#define TEMPERATURE_DDR  DDRD

#define TEMPERATURE_PIN_0 PD2

// Number of DS18B20 sensors connected to TEMPERATURE pin
#define SENSORS_COUNT 1

 // Number of retries to read in case of errors
#define TEMPERATURE_TRY_COUNTER 5

// Address DS18B20 sensors connected to TEMPERATURE pin

#define TEMPETATURE_SENSOR_0_ADDRESS 0x0000000000000000 // example: 0x2892F148F6473CE2
#define TEMPETATURE_SENSOR_1_ADDRESS 0x0000000000000000
#define TEMPETATURE_SENSOR_2_ADDRESS 0x0000000000000000
#define TEMPETATURE_SENSOR_3_ADDRESS 0x0000000000000000
#define TEMPETATURE_SENSOR_4_ADDRESS 0x0000000000000000
#define TEMPETATURE_SENSOR_5_ADDRESS 0x0000000000000000
#define TEMPETATURE_SENSOR_6_ADDRESS 0x0000000000000000

// Number of ADC channels scan, scan started of PC0(ADC0)
#define ADC_CHANNELS 8 // PC0-PC8

// ADC multiplier if voltage divider used. 0.018 divider for 4.7V voltage
#define ADC_0_MULTIPLIER 0.018
#define ADC_1_MULTIPLIER 0.018
#define ADC_2_MULTIPLIER 0.018
#define ADC_3_MULTIPLIER 0.018
#define ADC_4_MULTIPLIER 0.018
#define ADC_5_MULTIPLIER 0.018
#define ADC_6_MULTIPLIER 0.018
#define ADC_7_MULTIPLIER 0.018

// Port for didital scan
#define DIGITAL_SENSORS_PORT PORTB
#define DIGITAL_SENSORS_DDR  DDRB
#define DIGITAL_SENSORS_PIN  PINB

#define DIGITAL_SENSORS_PIN_0 PB0
#define DIGITAL_SENSORS_PIN_1 PB1
#define DIGITAL_SENSORS_PIN_2 PB2
#define DIGITAL_SENSORS_PIN_3 PB3
#define DIGITAL_SENSORS_PIN_4 PB4

// Control port, for send enable/disable signal to relay, transistor, etc.
#define CONTROL_PORT PORTD
#define CONTROL_DDR  DDRD
#define CONTROL_PIN  PIND

#define CONTROL_0_PIN PD3
#define CONTROL_1_PIN PD4
#define CONTROL_2_PIN PD5
#define CONTROL_3_PIN PD6
#define CONTROL_4_PIN PD7

#define CONTROL_0_NAME "c0"
#define CONTROL_1_NAME "c1"
#define CONTROL_2_NAME "c2"
#define CONTROL_3_NAME "c3"
#define CONTROL_4_NAME "c4"

// Safe time intervals for prevent too often enable in seconds
//  maximum value is 255, 0 - none safetime

#define CONTROL_0_SAFE_TIME  20
#define CONTROL_1_SAFE_TIME  20
#define CONTROL_2_SAFE_TIME  20
#define CONTROL_3_SAFE_TIME 180
#define CONTROL_4_SAFE_TIME 180

#define ERROR_PORT PORTB
#define ERROR_DDR  DDRB
#define ERROR_LED  PB5

#if ( VENTILATION_ON_SAFE_TIME_IN_SECONDS > 255) || (HEATING_ON_SAFE_TIME_IN_SECONDS > 255) || (LIGHT_ON_SAFE_TIME_IN_SECONDS > 255)
	#error Safe time must be lower than 255 seconds
#endif

#if ( RESERVED_0_ON_SAFE_TIME_IN_SECONDS > 255) || (RESERVED_1_ON_SAFE_TIME_IN_SECONDS > 255) || (RESERVED_2_ON_SAFE_TIME_IN_SECONDS > 255)
	#error Safe time must be lower than 255 seconds
#endif

#endif /* I]1_H_ */