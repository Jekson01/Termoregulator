/*
 * RelayRegulator.cpp
 *
 *  Created on: 12 окт. 2017 г.
 *      Author: r2d2
 */

#include "RelayRegulator.h"

RelayRegulator::RelayRegulator() {
	// TODO Auto-generated constructor stub
	relOut = false;
	setOnOffTemperature(250, 240);
	unRegSwitchRelayCallback();
}

void RelayRegulator::regSwitchRelayCallback(void(*callback)(bool)) {
	swichRelayCallback = callback;
}

void RelayRegulator::unRegSwitchRelayCallback() {
	swichRelayCallback = 0;
}

void RelayRegulator::setOnOffTemperature(int16_t _ton, int16_t _toff) {
	tOn = _ton;
	tOff = _toff;
	if (tOn < tOff){
		heating = false;
	} else {
		heating = true;
	}
}

bool RelayRegulator::check(int16_t temperature) {
	/*
	 * Нагрев:
	 * T < ton -> ON
	 * T > toff -> OFF
	 *
	 * Охлаждение
	 * T > ton -> ON
	 * T < toff -> OFF*/

	if (heating){
		if (temperature > tOn)
			relOut = true;
		if (temperature < tOff)
			relOut = false;
	}else{
		if (temperature < tOn)
			relOut = true;
		if (temperature > tOff)
			relOut = false;
	}

	if (swichRelayCallback){
		swichRelayCallback(relOut);
	}

	return relOut;

}
