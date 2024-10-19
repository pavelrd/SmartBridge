/*
 * user_error.h
 *
 * Created: 19.10.2024 20:06:38
 *  Author: user
 */ 


#ifndef USER_ERROR_H_
#define USER_ERROR_H_

enum ERROR_TYPE
{
	ERROR_TEMPERATURE_SENSORS_RESET_FAILURE,
};

void init_error_messaging();

void show_error( enum ERROR_TYPE error );

#endif /* USER_ERROR_H_ */