/*
 * networkUnit.cpp
 *
 *  Created on: 18 окт. 2017 г.
 *      Author: r2d2
 */

#include "networkUnit.h"


namespace NetworkUnit {

	// HTTP server
	HttpServer server;
	FTPServer ftp;
	BssList networks;
	String network, password;
	Timer connectionTimer;
	bool isLogin = false;
	Timer resetLoginTimer;

	// MQTT client
	Timer mqttTimer;
	MqttClient *mqtt;
	bool mqttReady = true;
}

void NetworkUnit::start() {
	if (!settings.exist()){
		settings.ssid = WIFI_SSID;
		settings.password = WIFI_PWD;
		settings.adminPass = ADMIN_PASS;
		settings.sensorName = "Tемпература";
		settings.save();
	}
	settings.load();

	if (!mqttSettings.exist()){
		mqttSettings.enable = true;
		mqttSettings.broker = "192.168.1.121";
		mqttSettings.clientId = "esp-" + String(system_get_chip_id());
		mqttSettings.pass = "";
		mqttSettings.port = 1883;
		mqttSettings.username = "";
		mqttSettings.save();
	}

	mqttSettings.load();
	mqttSettings.clientId = "esp-" + String(system_get_chip_id());
	mqtt = new MqttClient(mqttSettings.broker, mqttSettings.port, isMqttResive);

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
	resetLoginTimer.initializeMs(300000, resetLogin);
}

void NetworkUnit::onIndex(HttpRequest& request, HttpResponse& response) {
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendTemplate(tmpl);
}

void NetworkUnit::onFile(HttpRequest& request, HttpResponse& response) {
	String file = request.getPath();
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
	WifiAccessPoint.enable(false);
	startFTP();
	startWebServer();
	mqttTimer.initializeMs(1000, publishMqttMessage).start();
}

void NetworkUnit::startFTP() {
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser(FTP_LOGIN, ADMIN_PASS); // FTP account
}

void NetworkUnit::startWebServer() {
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/mqttcfg", onAjaxMqttSettings);
	server.addPath("/ajax/temperatura", onAjaxGetTemperatura);
	server.addPath("/ajax/setparams", onAjaxSetTRParams);
	server.addPath("/ajax/getparams", onAjaxGetTRParam);
	server.addPath("/ajax/get-networks", onAjaxGetNetworks);
	server.addPath("/ajax/connect", onAjaxConnect);
	server.addPath("/ajax/login", onAjaxLogin);
	server.addPath("/ajax/getlogin", onAjaxCheckLogin);
	server.addPath("/ajax/savesettings", onAjaxSaveSettings);

	server.setDefaultHandler(onFile);

	//LocalUnit::onConversionCompleate(&publishMqttMessage);
	/*Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");*/
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

void NetworkUnit::isConnect(String ssid, uint8_t ssidLength, uint8_t *bssid,
		uint8_t reason) {

}

void NetworkUnit::isDisconnect(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason) {
	if (!WifiAccessPoint.isEnabled()){
		WifiAccessPoint.enable(true);
	}
}

void NetworkUnit::isSystemReady() {
	startFTP();
	startWebServer();
}

void NetworkUnit::onAjaxGetTRParam(HttpRequest& request,
		HttpResponse& response) {
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& param = json.createNestedObject("param");
	param["ton"] = LocalUnit::regulator.getTOn();
	param["toff"] = LocalUnit::regulator.getTOff();
	response.sendDataStream(stream, MIME_JSON);

}

void NetworkUnit::onAjaxConnect(HttpRequest& request, HttpResponse& response) {
	network = request.getQueryParameter("network");
	password = request.getQueryParameter("password");
	if (network.length() > 0){
		WifiStation.disconnect();
		connectionTimer.initializeMs(1200, makeConnection).startOnce();
	}
}

void NetworkUnit::makeConnection() {
	WifiStation.config(network, password);
	WifiStation.connect();
	WifiStation.enable(true);
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

void NetworkUnit::onAjaxMqttSettings(HttpRequest& request,
		HttpResponse& response) {
	if (request.method == HTTP_POST){

		mqttSettings.enable = request.getPostParameter("mqtt") == "on";
		mqttSettings.broker = request.getPostParameter("broker");
		mqttSettings.port = (uint16_t)request.getPostParameter("port").toInt();
		mqttSettings.username = request.getPostParameter("mqttuser");
		mqttSettings.pass = request.getPostParameter("mqttpass");
		mqtt = new MqttClient(mqttSettings.broker, mqttSettings.port, isMqttResive);
		mqttSettings.save();
	}

	TemplateFileStream *tmpl = new TemplateFileStream("mqttcfg.html");
	auto &vars = tmpl->variables();

	vars["mqtt"] = mqttSettings.enable ? "checked='checked'" : "";
	vars["broker"] = mqttSettings.broker;
	vars["port"] = mqttSettings.port;
	vars["mqttuser"] = mqttSettings.username;
	vars["mqttpass"] = mqttSettings.pass;
	vars["clientid"] = mqttSettings.clientId;

	response.sendTemplate(tmpl);
}

void NetworkUnit::isAPConnect(uint8_t[6], uint8_t) {
}

void NetworkUnit::updateNetworkList() {
	WifiStation.startScan(networkScanCompleted);
}

void NetworkUnit::onAjaxLogin(HttpRequest& request, HttpResponse& response) {
	isLogin = false;
	String tmpPass = request.getQueryParameter("password");
	if (tmpPass == settings.adminPass ){
		isLogin = true;
		resetLoginTimer.startOnce();
	}else
	{
		isLogin = false;
	}
}

void NetworkUnit::onAjaxCheckLogin(HttpRequest& request,
		HttpResponse& response) {

	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& param = json.createNestedObject("state");
	param["islogin"] = isLogin;
	param["title"] = settings.sensorName;
	response.sendDataStream(stream, MIME_JSON);
}

void NetworkUnit::onAjaxSaveSettings(HttpRequest& request,
		HttpResponse& response) {
	String newName = urlToString(request.getQueryParameter("newtitle"));
	String newPass = request.getQueryParameter("newpass");

	//Serial.println(newName);
	//Serial.println(newPass);
	if (!isLogin)
		return;

	if (newPass.length() > 0){
		settings.adminPass = newPass;
	}
	if (newName.length() > 0){
		settings.sensorName = newName;
	}
	settings.save();
	isLogin = false;
}


void NetworkUnit::startMqttClient() {
	if(mqtt->connect(mqttSettings.clientId, mqttSettings.username, mqttSettings.pass )){
		mqtt->subscribe(mqttSettings.clientId + "/p/#");
	}

}


void NetworkUnit::publishMqttMessage() {

	if (!mqttSettings.enable)
		return;

	if (mqtt == null)
		return;

	if (mqttReady){
		mqttReady = false;
		if (mqtt->getConnectionState() != eTCS_Connected){
			startMqttClient();
		}

		if (LocalUnit::isCompleateConvert()){
			float t =((float)LocalUnit::getTemperature()) / 10;
			mqtt->publish(mqttSettings.clientId + "/temperatura", String(t));
			mqtt->publish(mqttSettings.clientId + "/relay", String(LocalUnit::regulator.isRelOut()));
			mqtt->publish(mqttSettings.clientId + "/heatihg", String(LocalUnit::regulator.isHeating()));

		}
		mqttReady = true;
	}
}

void NetworkUnit::isMqttResive(String topic, String message) {
	//Serial.println(topic);
	//Serial.println(message);
	if (topic == mqttSettings.clientId + "/p/ton"){
		int16_t t = (int16_t)(message.toFloat() * 10);
		LocalUnit::regulator.setOnOffTemperature(t, LocalUnit::regulator.getTOff());
	}

	if (topic == mqttSettings.clientId + "/p/toff"){
		int16_t t = (int16_t)(message.toFloat() * 10);
		LocalUnit::regulator.setOnOffTemperature(LocalUnit::regulator.getTOn(), t);
	}
}

void NetworkUnit::resetLogin() {
	isLogin = false;
}
