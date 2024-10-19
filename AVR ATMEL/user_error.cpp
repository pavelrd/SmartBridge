#include "user_error.h"
#include "pins.h"

void init_error_messaging()
{
	
	ERROR_DDR  |= (1<<ERROR_LED);
	ERROR_PORT &= ~(1<<ERROR_LED);
	
}

/**
	
	@brief �������� ���������, �� ���������� ��� ���� ������ � �������� ������
	
	// ���������� ���:
	//
	//    1. ������������ �� watchdog
	//    2. ���������� �������� ����������� ��� ������ ������������� ��������
	//    3. ���������� �����������
	//    4. ������� ������������ �����(��������� �������, ���������� �.�.�.) ������ �����������
	
*/

void show_error( enum ERROR_TYPE error )
{
	ERROR_DDR |= (1<<ERROR_LED);
}