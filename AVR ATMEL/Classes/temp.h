/*
 * temp.cpp
 *
 * Created: 13.02.2024 9:01:15
 *  Author: Pavel
 */ 

#ifndef TEMP_H_
#define TEMP_H_

class DS18B20
{
private:
	enum COMMANDS
	{
		SKIP_ROM            = 0xCC,
		READ_ROM			= 0x33,
		MATCH_ROM           = 0x55,
		CONVERT_TEMPERATURE = 0x44,
		READ_SCRATCHPAD     = 0xBE
	};
	static uint8_t crc8( uint8_t* data, uint8_t length );
	static uint8_t pinNumber;
public:
	
	enum ACCURACY 
	{
		ACCURACY_9BIT,
		ACCURACY_10BIT,
		ACCURACY_11BIT,
		ACCURACY_12BIT
	};
	
public:
		DS18B20(); 
		static void init_temp(uint8_t _pinNumber); 
		static bool checkready_temperature(); 
		static void write_bit(uint8_t byte); 
		static void write_byte (uint8_t byte);
		static uint8_t read_bit(); 
		static uint8_t read_byte(); 
		static bool measure();
		static bool get_temperature(uint64_t address=0, float *temperature=0);
		void set_accuracy( enum ACCURACY accuracy );
		static uint64_t get_address();
};

#endif