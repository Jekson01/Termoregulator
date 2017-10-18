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

void RelayRegulator::setOnOffTemperature(int16_t _upT, int16_t _downT) {
	if (_upT < _downT){
		heating = false;
		tOn = _downT;
		tOff = _upT;
	} else {
		heating = true;
		tOn = _upT;
		tOff = _downT;
	}
}

bool RelayRegulator::check(int16_t temperature) {
	/*
	 * Нагрев:
	 * T > tUP -> ON
	 * T < dDown -> OFF
	 *
	 * Охлаждение
	 * T > tUp -> OFF
	 * T < tUp -> ON*/

	if (temperature > tOn)
		relOut = heating ^ true;

	if (temperature < tOff)
		relOut = heating ^ false;

	if (swichRelayCallback){
		swichRelayCallback(relOut);
	}

	return relOut;

}
