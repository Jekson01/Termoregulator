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
	int16_t upT;
	int16_t downT;
public:
	RelayRegulator();
	void setOnOffTemperature(int16_t _upT, int16_t _downT);
	void regSwitchRelayCallback(void(*callback)(bool));
	void unRegSwitchRelayCallback();
	bool check(int16_t temperature);

	int16_t getUpT() const {
		if(heating)
			return upT;
		return downT;
	}

	void setUpT(int16_t upT) {
		this->upT = upT;
	}

	int16_t getDownT() const {
		if(heating)
			return downT;
		return upT;
	}

	void setDownT(int16_t downT) {
		this->downT = downT;
	}

	bool isRelOut() const {
		return relOut;
	}

	bool isHeating() const {
		return heating;
	}
};

#endif /* APP_RELAYREGULATOR_H_ */
