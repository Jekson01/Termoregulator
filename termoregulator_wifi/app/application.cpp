#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "localUnit.h"
#include "networkUnit.h"




void init()
{
	// need to debug
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	//Serial.end();
	Serial.systemDebugOutput(false); // Enable debug output to serial

	// Монтирование файловой системы
	spiffs_mount();

	// Загрузка параметров

	// Включение терморегулятора
	LocalUnit::start();

	// Старт web
	NetworkUnit::start();
}
