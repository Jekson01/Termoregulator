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

#define WIFI_SSID "ASUS-8B3C"
#define WIFI_PWD "12345678"

#define FTP_LOGIN	"user"
#define FTP_PASS	"123"

namespace NetworkUnit {

	void start();
	void onIndex(HttpRequest &request, HttpResponse &response);
	void onFile(HttpRequest &request, HttpResponse &response);
	void onIPadress(IPAddress ip, IPAddress netmask, IPAddress gateway);
	void startFTP();
	void startWebServer();
	void isConnect(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason);
	void isDisconnect(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason);
	void isSystemReady();

	void onAjaxGetTemperatura(HttpRequest &request, HttpResponse &response);
	void onAjaxSetTRParams(HttpRequest &request, HttpResponse &response);
	void onAjaxGetTRParam(HttpRequest &request, HttpResponse &response);
	void getWifiSettings(HttpRequest &request, HttpResponse &response);

}  // namespace NetUnit

#endif /* APP_NETWORKUNIT_H_ */
