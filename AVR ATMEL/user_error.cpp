#include "user_error.h"
#include "pins.h"

void init_error_messaging()
{
	
	ERROR_DDR  |= (1<<ERROR_LED);
	ERROR_PORT &= ~(1<<ERROR_LED);
	
}

/**
	
	@brief Включает светодиод, он показывает что была ошибка в процессе работы
	
	// Загорается при:
	//
	//    1. перезагрузке по watchdog
	//    2. пропадании датчиков температуры или приема неправильного значения
	//    3. превышении температуры
	//    4. попытки переключения ножек(включения нагрева, вентиляции и.т.д.) больше допустимого
	
*/

static ERROR_TYPE lastError = ERROR_NOERROR;

void show_error( ERROR_TYPE error )
{
	ERROR_PORT |= (1<<ERROR_LED);
	lastError = error;
}

static uint8_t  lastReset = 0;
static uint32_t resetCounter __attribute__ ((section (".noinit")));

void check_reset_state()
{
	
	lastReset = MCUCSR;
		
	MCUCSR &= ~(1<<JTRF|1<<WDRF|1<<BORF|1<<EXTRF|1<<PORF);
	
	if( lastReset & (1<<JTRF) )
	{
		show_error(ERROR_RESET_JTAG);
	}
	else if( lastReset & (1<<WDRF) )
	{
		show_error(ERROR_RESET_WATCHDOG);
	}
	else if( lastReset & (1<<BORF) )
	{
		show_error(ERROR_RESET_BROWN_OUT);
		
	}
	else if( lastReset & (1<<EXTRF) )
	{
		show_error(ERROR_RESET_EXTRF);
	}
	
	if( lastReset & (1<<PORF) )
	{
		// First power on
		resetCounter = 0;
	}
	else
	{
		// Reboot while power is on(watchdog, jtag, external reset)
		resetCounter += 1;	
	}

}

uint8_t get_last_reset_reasons()
{
	return lastReset;
}

uint32_t get_reset_counter()
{
	return resetCounter;
}

ERROR_TYPE get_last_error()
{
	return lastError;
}