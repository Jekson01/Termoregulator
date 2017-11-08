/*
 * localUnit.h
 *
 *  Created on: 18 окт. 2017 г.
 *      Author: r2d2
 */

#ifndef APP_LOCALUNIT_H_
#define APP_LOCALUNIT_H_
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>
#include <Libraries/DS18S20/ds18s20.h>
#include "lib/TM1637.h"
#include "lib/RelayRegulator.h"
#include "tr_settings.h"

#define KEY1	232
#define KEY2	233
#define KEY3	237
#define KEY4	238

namespace LocalUnit {
	extern RelayRegulator regulator;
	extern uint16_t arrPos;
	void start();
	int16_t getTemperature();
	void updateSensor();
	void updateMenu();
	void updateUI();
	void saveSettings();
	bool isCompleateConvert();

}  // namespace LU

#endif /* APP_LOCALUNIT_H_ */
