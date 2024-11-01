/*
 * user_error.h
 *
 * Created: 19.10.2024 20:06:38
 *  Author: user
 */ 


#ifndef USER_ERROR_H_
#define USER_ERROR_H_

#include <stdint.h>

enum ERROR_TYPE
{
	ERROR_NOERROR = 1,
	ERROR_TEMPERATURE_SENSORS_RESET_FAILURE,
	ERROR_TEMPERATURE_SENSORS_GET_MEASURE_FAILURE,
	ERROR_RESET_WATCHDOG,
	ERROR_RESET_BROWN_OUT,
	ERROR_RESET_EXTRF,
	ERROR_PIN_SHORTED_TO_GND,
	ERROR_PIN_SHORTED_TO_VCC
};

void init_error_messaging();

void show_error( ERROR_TYPE error );

void check_reset_state();

uint8_t get_last_reset_reasons();

uint32_t get_reset_counter();

ERROR_TYPE get_last_error();

#endif /* USER_ERROR_H_ */