/*
 * led.cpp
 *
 * Created: 13.02.2024 9:12:09
 *  Author: Pavel
 */ 


#ifndef LED_H_
#define LED_H_

class Led
{
	public:
			void init_led(); 
			void led_control(); 
			void send_sensors_data(); 
};
#endif