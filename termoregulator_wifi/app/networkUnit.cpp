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
	Timer connectionTimer;
}

void NetworkUnit::start() {

	if (!settings.exist()){
		settings.ssid = WIFI_SSID;
		settings.password = WIFI_PWD;
		settings.save();
	}

	settings.load();
	Serial.println(settings.ssid);
	Serial.println(settings.password);

	// Включение wifi станции
	WifiStation.enable(true);
	WifiStation.config(settings.ssid, settings.password);

	WifiStation.startScan(networkScanCompleted);

	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("ESP", "", AUTH_OPEN);
	System.onReady(isSystemReady);

	WifiEvents.onStationGotIP(onIPadress);
	WifiEvents.onStationConnect(isConnect);
	WifiEvents.onStationDisconnect(isDisconnect);
	WifiEvents.onAccessPointConnect(isAPConnect);
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
	server.addPath("/ajax/get-networks", onAjaxGetNetworks);
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
	LocalUnit::saveSettings();
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

void NetworkUnit::onAjaxConnect(HttpRequest& request, HttpResponse& response) {
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	String curNet = request.getPostParameter("network");
	String curPass = request.getPostParameter("password");

	bool updating = curNet.length() > 0
			&& (WifiStation.getSSID() != curNet
					|| WifiStation.getPassword() != curPass);
	bool connectingNow = WifiStation.getConnectionStatus() == eSCS_Connecting
			|| network.length() > 0;

	if (updating && connectingNow) {
		debugf("wrong action: %s %s, (updating: %d, connectingNow: %d)", network.c_str(), password.c_str(), updating, connectingNow);
		json["status"] = (bool) false;
		json["connected"] = (bool) false;
	} else {
		json["status"] = (bool) true;
		if (updating) {
			network = curNet;
			password = curPass;
			debugf("CONNECT TO: %s %s", network.c_str(), password.c_str());
			json["connected"] = false;
			connectionTimer.initializeMs(1200, makeConnection).startOnce();
		} else {
			json["connected"] = WifiStation.isConnected();
			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName());
		}
	}

	if (!updating && !connectingNow && WifiStation.isConnectionFailed())
		json["error"] = WifiStation.getConnectionStatusName();

	response.setAllowCrossDomainOrigin("*");
	response.sendDataStream(stream, MIME_JSON);
}

void NetworkUnit::makeConnection() {
	WifiStation.enable(true);
	WifiStation.config(network, password);

	settings.ssid = network;
	settings.password = password;
	settings.save();

	network = ""; // task completed
}

void NetworkUnit::networkScanCompleted(bool succeeded, BssList list) {
	networks.clear();
	if (succeeded) {
		for (int i = 0; i < list.count(); i++)
			if (!list[i].hidden && list[i].ssid.length() > 0)
				networks.add(list[i]);
	}
	networks.sort(
			[](const BssInfo& a, const BssInfo& b) {return b.rssi - a.rssi;});
}

void NetworkUnit::getWifiSettings(HttpRequest& request,
		HttpResponse& response) {
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& param = json.createNestedObject("wifiparam");
	param["ssid"] = WifiStation.getSSID();

	response.sendDataStream(stream, MIME_JSON);
}

void NetworkUnit::onAjaxGetNetworks(HttpRequest& request,
		HttpResponse& response) {
	updateNetworkList();
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["status"] = (bool) true;

	bool connected = WifiStation.isConnected();
	json["connected"] = connected;
	if (connected) {
		// Copy full string to JSON buffer memory
		json["network"] = WifiStation.getSSID();
	}

	JsonArray& netlist = json.createNestedArray("available");
	for (int i = 0; i < networks.count(); i++) {
		if (networks[i].hidden)
			continue;
		JsonObject &item = netlist.createNestedObject();
		item["id"] = (int) networks[i].getHashId();
		// Copy full string to JSON buffer memory
		item["title"] = networks[i].ssid;
		item["signal"] = networks[i].rssi;
		item["encryption"] = networks[i].getAuthorizationMethodName();
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendDataStream(stream, MIME_JSON);
}

void NetworkUnit::isAPConnect(uint8_t[6], uint8_t) {
	Serial.println(__FUNCTION__);
}

void NetworkUnit::updateNetworkList() {
	WifiStation.startScan(networkScanCompleted);
}
