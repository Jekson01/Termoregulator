/*
 * settings.h
 *
 *  Created on: 23 окт. 2017 г.
 *      Author: r2d2
 */

#include <SmingCore/SmingCore.h>

#ifndef INCLUDE_SETTINGS_H_
#define INCLUDE_SETTINGS_H_


#define APP_SETTINGS_FILE ".wifi.conf" // leading point for security reasons :)

struct AppSettings{
	String ssid;
	String password;
	String adminPass;
	String sensorName;
	void load() {
		DynamicJsonBuffer jsonBuffer;
		if (exist()) {
			int size = fileGetSize(APP_SETTINGS_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(APP_SETTINGS_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);

			JsonObject& param = root["param"];
			ssid = param["ssid"].asString();
			password = param["pass"].asString();
			adminPass = param["apass"].asString();
			sensorName = param["name"].asString();
			delete[] jsonString;
		}
	}

	void save() {
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		JsonObject& param = jsonBuffer.createObject();
		root["param"] = param;
		param["ssid"] = ssid.c_str();
		param["pass"] = password.c_str();
		param["apass"] = adminPass.c_str();
		param["name"] = sensorName.c_str();

		String rootString;
		root.printTo(rootString);
		fileSetContent(APP_SETTINGS_FILE, rootString);
	}

	bool exist(){
		return fileExist(APP_SETTINGS_FILE);
	}
};

static AppSettings settings;


#endif /* INCLUDE_SETTINGS_H_ */
