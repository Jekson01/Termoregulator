/*
 * networkUnit.h
 *
 *  Created on: 18 окт. 2017 г.
 *      Author: r2d2
 */

#ifndef APP_NETWORKUNIT_H_
#define APP_NETWORKUNIT_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "localUnit.h"
#include "settings.h"
#include "lib/Convertor.h"
#include "mqtt_settings.h"

#define WIFI_SSID "ASUS-8B3C"
#define WIFI_PWD "12345678"
#define ADMIN_PASS	"admin"

#define FTP_LOGIN	"admin"

namespace NetworkUnit {

	void start();
	void onIndex(HttpRequest &request, HttpResponse &response);
	void onFile(HttpRequest &request, HttpResponse &response);
	void onIPadress(IPAddress ip, IPAddress netmask, IPAddress gateway);
	void startFTP();
	void startWebServer();
	void startMqttClient();

	void isConnect(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason);
	void isDisconnect(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason);
	void isSystemReady();
	void isAPConnect(uint8_t[6], uint8_t);

	void onAjaxGetTemperatura(HttpRequest &request, HttpResponse &response);
	void onAjaxSetTRParams(HttpRequest &request, HttpResponse &response);
	void onAjaxGetTRParam(HttpRequest &request, HttpResponse &response);
	void onAjaxGetNetworks(HttpRequest &request, HttpResponse &response);
	void onAjaxConnect(HttpRequest &request, HttpResponse &response);
	void onAjaxLogin(HttpRequest &request, HttpResponse &response);
	void onAjaxCheckLogin(HttpRequest &request, HttpResponse &response);
	void onAjaxSaveSettings(HttpRequest &request, HttpResponse &response);
	void onAjaxMqttSettings(HttpRequest &request, HttpResponse &response);
	void makeConnection();
	void networkScanCompleted(bool succeeded, BssList list);
	void updateNetworkList();

	void isMqttResive(String topic, String message);
	void publishMqttMessage();

	void resetLogin();

}  // namespace NetUnit

#endif /* APP_NETWORKUNIT_H_ */
