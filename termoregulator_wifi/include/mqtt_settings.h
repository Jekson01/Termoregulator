/*
 * mqtt_settings.h
 *
 *  Created on: 7 нояб. 2017 г.
 *      Author: r2d2
 */
#include <SmingCore/SmingCore.h>
#ifndef INCLUDE_MQTT_SETTINGS_H_
#define INCLUDE_MQTT_SETTINGS_H_

#define MQTT_SETTINGS_FILE ".mqtt.conf"

struct MqttSettings{
	bool enable = false;
	String broker;
	uint16_t port = 1883;
	String clientId;
	String username;
	String pass;

	void load(){
		DynamicJsonBuffer jsonBuffer;
		if (exist()){
			int size = fileGetSize(MQTT_SETTINGS_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(MQTT_SETTINGS_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);

			JsonObject& param = root["param"];
			enable = param["enable"];
			broker = param["broker"].asString();
			port = param["port"];
			clientId = param["client-id"].asString();
			username = param["username"].asString();
			pass = param["pass"].asString();
			delete[] jsonString;
		}
	}

	void save(){
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		JsonObject& param = jsonBuffer.createObject();

		root["param"] = param;
		param["enable"] = enable;
		param["broker"] = broker.c_str();
		param["port"] = port;
		param["client-id"] = clientId.c_str();
		param["username"] = username.c_str();
		param["pass"] = pass.c_str();

		String rootString;
		root.printTo(rootString);
		fileSetContent(MQTT_SETTINGS_FILE, rootString);
	}


	bool exist(){
		return fileExist(MQTT_SETTINGS_FILE);
	}
};

static MqttSettings mqttSettings;

#endif /* INCLUDE_MQTT_SETTINGS_H_ */
