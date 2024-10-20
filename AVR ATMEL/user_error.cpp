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

void show_error( enum ERROR_TYPE error )
{

	ERROR_PORT |= (1<<ERROR_LED);
}
	
void check_reset_state()
{
	
	if( MCUCSR & (1<<JTRF) )
	{
		show_error(ERROR_RESET_JTAG);
	}
	else if( MCUCSR & (1<<WDRF) )
	{
		show_error(ERROR_RESET_WATCHDOG);
	}
	else if( MCUCSR & (1<<BORF) )
	{
		show_error(ERROR_RESET_BROWN_OUT);
		
	}
	else if( MCUCSR & (1<<EXTRF) )
	{
		show_error(ERROR_RESET_EXTRF);
	}
	//else if( MCUCSR & PORF )
	//{
	//  Нормальный режим работы, включение питания
	//}
	
}