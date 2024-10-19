/*
 * user_types.h
 *
 * Created: 19.10.2024 15:54:27
 *  Author: user
 */ 


#ifndef USER_TYPES_H_
#define USER_TYPES_H_

union requests_t
{
	uint8_t ventilation       : 1;
	uint8_t light             : 1;
	uint8_t heating           : 1;
	uint8_t reserved0         : 1;
	uint8_t reserved1         : 1;
	uint8_t reserved2         : 1;
	uint8_t timerTick         : 1;
	uint8_t measureProccessed : 1;
};

struct counters_t
{
	uint8_t vent;
	uint8_t heat;
	uint8_t light;
	uint8_t reserved0;
	uint8_t reserved1;
	uint8_t reserved2;
};



#endif /* USER_TYPES_H_ */