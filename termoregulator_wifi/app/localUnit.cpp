/*
 * localUnit.cpp
 *
 *  Created on: 18 окт. 2017 г.
 *      Author: r2d2
 */

#include "localUnit.h"

namespace LocalUnit {

	TM1637 sysio;
	RelayRegulator regulator;
	DS18S20 sensor;

	// меню
	enum menuItems {
		mNone, mLeft, mLeftEdit, mRight, mRightEdit
	};
	uint8_t menuItem;
	uint8_t menuCounter;

	int16_t Temperatura;

	// Таймеры
	Timer updateUITimer;
	Timer updateMenuTimer;
	Timer updateSensorTimer;

}  // namespace LU

void LocalUnit::updateSensor(){
	if (!sensor.MeasureStatus()){
		if (sensor.GetSensorsCount()){
			if (sensor.IsValidTemperature(0)){
				float T = sensor.GetCelsius(0);
				Temperatura = (int16_t)(T * 10);
				regulator.check(Temperatura);
			}
		}
		sensor.StartMeasure();
	}
}

void LocalUnit::updateMenu(){
	if (menuItem == mLeft){
		menuItem = mLeftEdit;
		return;
	}

	if (menuItem == mRight){
		menuItem = mRightEdit;
		return;
	}
	menuItem = mNone;
}

void LocalUnit::start() {
	sysio.initialize(5, 4, 2);
	sysio.setPoint(POINT2);

	sensor.Init(14);
	sensor.StartMeasure();

	updateUITimer.initializeMs(200, updateUI).start();
	updateMenuTimer.initializeMs(3000, updateMenu).start();
	updateSensorTimer.initializeMs(5000, updateSensor).start();
}

void LocalUnit::updateUI(){
	uint8_t key = sysio.scanKey();

		if (key != 0xFF){
			updateMenuTimer.restart();
			if (menuItem == mNone){
				menuCounter++;
				if (menuCounter >= 15){
					menuCounter = 0;
					if (key == KEY1){
						menuItem = mLeft;
					}

					if (key == KEY2){
						menuItem = mRight;
					}
				}
			}

			int8_t addVal = 0;
			if (key == KEY1){
				addVal = -1;
			}
			if (key == KEY2){
				addVal = 1;
			}

			if (menuItem == mLeftEdit) {
				int16_t t = regulator.getTOn();
				t+=addVal;
				regulator.setTOn(t);
			}
			if (menuItem == mRightEdit) {
				int16_t t = regulator.getTOff();
				t+=addVal;
				regulator.setTOff(t);
			}
		}

		switch (menuItem) {
			case mLeft:{
				sysio.print(111);
				break;
			}

			case mLeftEdit:{
				sysio.print(regulator.getTOn());
				break;
			}

			case mRight:{
				sysio.print(222);
				break;
			}
			case mRightEdit:{
				sysio.print(regulator.getTOff());
				break;
			}
			default:
				sysio.print(Temperatura);
				break;
		}

		sysio.update();
}

int16_t LocalUnit::getTemperature() {
	return Temperatura;
}
