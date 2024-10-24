/*
 * user_types.h
 *
 * Created: 19.10.2024 15:54:27
 *  Author: user
 */ 


#ifndef USER_TYPES_H_
#define USER_TYPES_H_

struct control_unit_t
{
	uint8_t counter;
	bool isBad;
};

struct control_units
{
	control_unit_t ventilation;
	control_unit_t light;
	control_unit_t heating;
	control_unit_t reserved0;
	control_unit_t reserved1;
	control_unit_t reserved2;
	control_unit_t timerTick;
};

#endif /* USER_TYPES_H_ */