/*
 * networkUnit.cpp
 *
 *  Created on: 18 окт. 2017 г.
 *      Author: r2d2
 */

#include "networkUnit.h"


namespace NetworkUnit {

	HttpServer server;
	FTPServer ftp;
	BssList networks;
	String network, password;
}

void NetworkUnit::start() {

	// need to debug
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Enable debug output to serial

	// Включение wifi станции и отключение точки доступа
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	//System.onReady(isSystemReady);

	WifiEvents.onStationGotIP(onIPadress);
	WifiEvents.onStationConnect(isConnect);
	WifiEvents.onStationDisconnect(isDisconnect);
}

void NetworkUnit::onIndex(HttpRequest& request, HttpResponse& response) {
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendTemplate(tmpl);
}

void NetworkUnit::onFile(HttpRequest& request, HttpResponse& response) {
	String file = request.getPath();
	Serial.println(file);
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void NetworkUnit::onIPadress(IPAddress ip, IPAddress netmask, IPAddress gateway) {
	Serial.println(__FUNCTION__);
	startFTP();
	startWebServer();
}

void NetworkUnit::startFTP() {
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser(FTP_LOGIN, FTP_PASS); // FTP account
}

void NetworkUnit::startWebServer() {
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/ajax/temperatura", onAjaxGetTemperatura);
	server.addPath("/ajax/setparams", onAjaxSetTRParams);
	server.addPath("/ajax/getparams", onAjaxGetTRParam);
	server.setDefaultHandler(onFile);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
}

void NetworkUnit::onAjaxGetTemperatura(HttpRequest& request,
	HttpResponse& response) {
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& sensor = json.createNestedObject("sensor");
	sensor["temperatura"] = LocalUnit::getTemperature();
	sensor["state"] = (bool)LocalUnit::regulator.isRelOut();
	sensor["mode"] = (bool)LocalUnit::regulator.isHeating();
	response.sendDataStream(stream, MIME_JSON);
}

void NetworkUnit::onAjaxSetTRParams(HttpRequest& request,
		HttpResponse& response) {
	int tOn = request.getQueryParameter("ton").toInt();
	int tOff = request.getQueryParameter("toff").toInt();
	LocalUnit::regulator.setOnOffTemperature(tOn, tOff);
}

void NetworkUnit::isConnect(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason) {
	Serial.println(__FUNCTION__);
}

void NetworkUnit::isDisconnect(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason) {
	Serial.println(__FUNCTION__);
}

void NetworkUnit::isSystemReady() {
	Serial.println(__FUNCTION__);
	startFTP();
	startWebServer();
}

void NetworkUnit::onAjaxGetTRParam(HttpRequest& request,
		HttpResponse& response) {
	Serial.print(">");
	Serial.println(__FUNCTION__);
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& param = json.createNestedObject("param");
	param["ton"] = LocalUnit::regulator.getTOn();
	param["toff"] = LocalUnit::regulator.getTOff();
	response.sendDataStream(stream, MIME_JSON);
	Serial.println(__FUNCTION__);
}
