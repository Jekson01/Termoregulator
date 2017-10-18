/*
 * RelayRegulator.h
 *
 *  Created on: 12 окт. 2017 г.
 *      Author: r2d2
 */

#ifndef APP_RELAYREGULATOR_H_
#define APP_RELAYREGULATOR_H_

#include <inttypes.h>

class RelayRegulator {
	void(*swichRelayCallback)(bool);
	bool relOut;
	bool heating;
	int16_t tOn;
	int16_t tOff;
public:
	RelayRegulator();
	void setOnOffTemperature(int16_t _tOn, int16_t _tOff);
	void regSwitchRelayCallback(void(*callback)(bool));
	void unRegSwitchRelayCallback();
	bool check(int16_t temperature);

	int16_t getTOn() const {
		if(heating)
			return tOn;
		return tOff;
	}

	void setTOn(int16_t upT) {
		this->tOn = upT;
	}

	int16_t getTOff() const {
		if(heating)
			return tOff;
		return tOn;
	}

	void setTOff(int16_t downT) {
		this->tOff = downT;
	}

	bool isRelOut() const {
		return relOut;
	}

	bool isHeating() const {
		return heating;
	}
};

#endif /* APP_RELAYREGULATOR_H_ */
