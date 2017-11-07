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
	uint8_t isChangeParams = 0;

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

	if (!isChangeParams)
			return;
	isChangeParams = 0;
	saveSettings();
}

void LocalUnit::start() {

	if(!trSettings.exist()){
		trSettings.tOn = 240;
		trSettings.tOff = 250;
		trSettings.save();
	}

	trSettings.load();

	sysio.initialize(5, 4, 2);
	//sysio.setPoint(POINT2);
	regulator.setOnOffTemperature(trSettings.tOn, trSettings.tOff);
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

			if (menuItem == mRightEdit) {
				int16_t t = regulator.getTOn();
				t+=addVal;
				regulator.setOnOffTemperature(t, regulator.getTOff());
				isChangeParams = 1;
			}
			if (menuItem == mLeftEdit) {
				int16_t t = regulator.getTOff();
				t+=addVal;
				regulator.setOnOffTemperature(regulator.getTOn(), t);
				isChangeParams = 1;
			}
		}

		switch (menuItem) {
			case mRight:{
				sysio.printChar(0, S_t);
				sysio.printChar(1, S_o);
				sysio.printChar(2, S_n);
				break;
			}

			case mRightEdit:{
				sysio.printT(regulator.getTOn());
				break;
			}

			case mLeft:{
				sysio.printChar(0, S_t);
				sysio.printChar(1, S_o);
				sysio.printChar(2, S_F);
				break;
			}
			case mLeftEdit:{
				sysio.printT(regulator.getTOff());
				break;
			}
			default:
				sysio.printT(Temperatura);
				break;
		}

		sysio.update();
}

int16_t LocalUnit::getTemperature() {
	return Temperatura;
}

void LocalUnit::saveSettings() {
	trSettings.tOn = regulator.getTOn();
	trSettings.tOff = regulator.getTOff();
	trSettings.save();
}
