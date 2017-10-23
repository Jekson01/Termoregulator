/*
 * tr_settings.h
 *
 *  Created on: 23 окт. 2017 г.
 *      Author: r2d2
 */

#include <SmingCore/SmingCore.h>

#ifndef INCLUDE_TR_SETTINGS_H_
#define INCLUDE_TR_SETTINGS_H_

#define TR_SETTINGS_FILE ".tr.conf"

struct TRSettings{
	int16_t tOn;
	int16_t tOff;
	void load() {
		DynamicJsonBuffer jsonBuffer;
		if (exist()) {
			int size = fileGetSize(TR_SETTINGS_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(TR_SETTINGS_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);

			JsonObject& param = root["param"];
			tOn = param["ton"];
			tOff = param["toff"];

			delete[] jsonString;
		}
	}

	void save() {
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		JsonObject& param = jsonBuffer.createObject();
		root["param"] = param;
		param["ton"] = tOn;
		param["toff"] = tOff;

		String rootString;
		root.printTo(rootString);
		fileSetContent(TR_SETTINGS_FILE, rootString);
	}

	bool exist(){
		return fileExist(TR_SETTINGS_FILE);
	}
};

static TRSettings trSettings;



#endif /* INCLUDE_TR_SETTINGS_H_ */
