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
	ERROR_DDR |= (1<<ERROR_LED);
}