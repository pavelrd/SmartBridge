/*
 * uart.h
 *
 * Created: 17.01.2024 14:17:36
 *  Author: Pavel
 */ 


#ifndef UART_H_
#define UART_H_


class Uart
{		
public:
	enum SPEED
	{
		BAUD_9600,
		BAUD_115200
	};
	static void    init(bool isProgram_,  SPEED speed = BAUD_9600); 
	static void    send_byte (uint8_t byteToSend); 
	static uint8_t read_byte(); 
	static void    send(const char *pArray);
	static void    send(uint64_t byteToSend); 
	static bool    is_ready_read();
private:
	static bool isProgram;
};

#endif /* UART_H_ */