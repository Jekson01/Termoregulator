#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "localUnit.h"
#include "networkUnit.h"

void init()
{

	// Монтирование файловой системы
	spiffs_mount();

	// Загрузка параметров

	// Включение терморегулятора
	LocalUnit::start();

	// Старт web
	NetworkUnit::start();
}
