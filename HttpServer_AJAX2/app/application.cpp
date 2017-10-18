#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <AppSettings.h>
#include <Libraries/OneWire/OneWire.h>
#include <Libraries/DS18S20/ds18s20.h>

//#include "UI.h"
#include "TM1637.h"
#include "RelayRegulator.h"

#ifdef DEBUG_DS18S20
#undef DEBUG_DS18S20
#endif


#define KEY1	232
#define KEY2	233
#define KEY3	237
#define KEY4	238

#define CHAR_A	10
#define CHAR_b	11
#define CHAR_C	12
#define CHAR_d	13
#define CHAR_E	14
#define CHAR_F	15
#define CHAR_t	16
#define CHAR_u	17
#define CHAR_P	18
#define CHAR_n	19

char charTab[] = {	0xEB, // 0
					0x09, // 1
					0xF1, // 2
					0xB9, // 3
					0x1B, // 4
					0xBA, // 5
					0xFA, // 6
					0x89, // 7
					0xFB, // 8
					0xBB, // 9
					0xDB, // A
					0x7A, // b
					0xE2, // C
					0x79, // d
					0xF2, // E
					0xD2  // F

};


// If you want, you can define WiFi settings globally in Eclipse Environment Variables

#ifndef WIFI_SSID
	#define WIFI_SSID "ASUS-8B3C" // Put you SSID and Password here
	#define WIFI_PWD "12345678"
#endif


HttpServer server;
FTPServer ftp;

BssList networks;
String network, password;

DS18S20 ReadTemp;
Timer updateTemperatureTimer;
Timer updateUITimer;
Timer connectionTimer;
Timer updateMenuTimer;

TM1637 sysio;
RelayRegulator regulator;



bool relayState = false;

int16_t Temperatura;
uint8_t sensorCount = 0;

enum menuItems{mNone, mLeft, mLeftEdit, mRight, mRightEdit};
uint8_t menuItem;
uint8_t menuCounter;

void onSwitchRelay(bool state){
	relayState = state;
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	Serial.print(">");
	Serial.println(__FUNCTION__);
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendTemplate(tmpl); // this template object will be deleted automatically
	Serial.println(__FUNCTION__);
}

void onWifiSettings(HttpRequest &request, HttpResponse &response) {
	Serial.print(">");
	Serial.println(__FUNCTION__);
	/*if (request.method == HTTP_POST) {
		AppSettings.dhcp = request.getPostParameter("dhcp") == "1";
		AppSettings.ip = request.getPostParameter("ip");
		AppSettings.netmask = request.getPostParameter("netmask");
		AppSettings.gateway = request.getPostParameter("gateway");
		AppSettings.save();
	}*/

	TemplateFileStream *tmpl = new TemplateFileStream("wifisettings.html");
	auto &vars = tmpl->variables();

	/*bool dhcp = WifiStation.isEnabledDHCP();
		vars["dhcpon"] = dhcp ? "checked='checked'" : "";
		vars["dhcpoff"] = !dhcp ? "checked='checked'" : "";

		if (!WifiStation.getIP().isNull())
		{
			vars["ip"] = WifiStation.getIP().toString();
			vars["netmask"] = WifiStation.getNetworkMask().toString();
			vars["gateway"] = WifiStation.getNetworkGateway().toString();
		}
		else
		{
			vars["ip"] = "192.168.1.77";
			vars["netmask"] = "255.255.255.0";
			vars["gateway"] = "192.168.1.1";
		}*/

		response.sendTemplate(tmpl); // will be automatically deleted
		Serial.println(__FUNCTION__);
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	Serial.print(">");
	Serial.println(__FUNCTION__);
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
	Serial.println(__FUNCTION__);
}



void onAjaxTemperatura(HttpRequest &request, HttpResponse &response){
	Serial.print(">");
	Serial.println(__FUNCTION__);
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	if (sensorCount > 0)
	{
		json["status"] = (bool)true;
		JsonObject& sensor = json.createNestedObject("sensor");
		sensor["temperatura"] = (int16_t)Temperatura;
		sensor["state"] = (bool)regulator.isRelOut();
		sensor["mode"] = (bool)regulator.isHeating();
	}
	else json["status"] = (bool)false;
	response.sendDataStream(stream, MIME_JSON);
	Serial.println(__FUNCTION__);
}

void onAjaxSetTRParams(HttpRequest &request, HttpResponse &response){
	Serial.print(">");
	Serial.println(__FUNCTION__);
	int tUp = request.getQueryParameter("ton").toInt();
	int tDown = request.getQueryParameter("toff").toInt();
	regulator.setOnOffTemperature(tUp, tDown);
	Serial.println(__FUNCTION__);
}

void onAjaxGetTRParam(HttpRequest &request, HttpResponse &response) {
	Serial.print(">");
	Serial.println(__FUNCTION__);
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& param = json.createNestedObject("param");
	param["ton"] = regulator.getTOn();
	param["toff"] = regulator.getTOff();
	response.sendDataStream(stream, MIME_JSON);
	Serial.println(__FUNCTION__);
}

void onAjaxNetworkList(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["status"] = (bool)true;

	bool connected = WifiStation.isConnected();
	json["connected"] = connected;
	if (connected)
	{
		// Copy full string to JSON buffer memory
		json["network"]= WifiStation.getSSID();
	}

	JsonArray& netlist = json.createNestedArray("available");
	for (int i = 0; i < networks.count(); i++)
	{
		if (networks[i].hidden) continue;
		JsonObject &item = netlist.createNestedObject();
		item["id"] = (int)networks[i].getHashId();
		// Copy full string to JSON buffer memory
		item["title"] = networks[i].ssid;
		item["signal"] = networks[i].rssi;
		item["encryption"] = networks[i].getAuthorizationMethodName();
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendDataStream(stream, MIME_JSON);
	Serial.println(__FUNCTION__);
}

void makeConnection()
{
	Serial.print(">");
	WifiStation.enable(true);
	WifiStation.config(network, password);

	AppSettings.ssid = network;
	AppSettings.password = password;
	AppSettings.save();

	network = ""; // task completed
	Serial.println(__FUNCTION__);
}

void onAjaxConnect(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	String curNet = request.getPostParameter("network");
	String curPass = request.getPostParameter("password");

	bool updating = curNet.length() > 0 && (WifiStation.getSSID() != curNet || WifiStation.getPassword() != curPass);
	bool connectingNow = WifiStation.getConnectionStatus() == eSCS_Connecting || network.length() > 0;

	if (updating && connectingNow)
	{
		debugf("wrong action: %s %s, (updating: %d, connectingNow: %d)", network.c_str(), password.c_str(), updating, connectingNow);
		json["status"] = (bool)false;
		json["connected"] = (bool)false;
	}
	else
	{
		json["status"] = (bool)true;
		if (updating)
		{
			network = curNet;
			password = curPass;
			debugf("CONNECT TO: %s %s", network.c_str(), password.c_str());
			json["connected"] = false;
			connectionTimer.initializeMs(1200, makeConnection).startOnce();
		}
		else
		{
			json["connected"] = WifiStation.isConnected();
			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName());
		}
	}

	if (!updating && !connectingNow && WifiStation.isConnectionFailed())
		json["error"] = WifiStation.getConnectionStatusName();

	response.setAllowCrossDomainOrigin("*");
	response.sendDataStream(stream, MIME_JSON);
	Serial.println(__FUNCTION__);
}

void startWebServer()
{
	Serial.print(">");
	Serial.println(__FUNCTION__);
	server.listen(80);
	server.addPath("/", onIndex);
	//server.addPath("/wifisettings", onWifiSettings);
	server.addPath("/ajax/temperatura", onAjaxTemperatura);
	server.addPath("/ajax/setparams", onAjaxSetTRParams);
	server.addPath("/ajax/getparams", onAjaxGetTRParam);
	//server.addPath("/ajax/get-networks", onAjaxNetworkList);
	//server.addPath("/ajax/connect", onAjaxConnect);
	server.setDefaultHandler(onFile);

	//Serial.println("\r\n=== NEW WEB SERVER STARTED ===");
	//Serial.println(WifiStation.getIP());
	//Serial.println("==============================\r\n");
	Serial.println(__FUNCTION__);
}

void startFTP()
{
	Serial.print(">");
	Serial.println(__FUNCTION__);
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
	Serial.println(__FUNCTION__);
}

void readTemperature()
{
	uint64_t info = 0;

	if (!ReadTemp.MeasureStatus())  // the last measurement completed
	{
	  if (ReadTemp.GetSensorsCount())   // is minimum 1 sensor detected ?
		  sensorCount = ReadTemp.GetSensorsCount();
		for(uint8_t a = 0; a < sensorCount; a++)   // prints for all sensors
		{
		  if (ReadTemp.IsValidTemperature(a))   // temperature read correctly ?
			{
			  float T = ReadTemp.GetCelsius(a);
			  //Serial.println(T);
			  Temperatura = (int16_t)(T * 10);
			  regulator.check(Temperatura);
			}
		}
		ReadTemp.StartMeasure();  // next measure, result after 1.2 seconds * number of sensors
	}
	//Serial.println(__FUNCTION__);
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	Serial.print(">");
	Serial.println(__FUNCTION__);
	startFTP();
	startWebServer();
	Serial.println(__FUNCTION__);
}

void updateMenu(){
	if (menuItem == mLeft){
		menuItem = mLeftEdit;
		return;
	}

	if (menuItem == mRight){
		menuItem = mRightEdit;
		return;
	}
	menuItem = mNone;
}

void updateUI(){
	uint8_t key = sysio.scanKey();

	if (key != 0xFF){
		updateMenuTimer.restart();
		if (menuItem == mNone){
			menuCounter++;
			if (menuCounter >= 15){
				menuCounter = 0;
				if (key == KEY1){
					menuItem = mLeft;
				}

				if (key == KEY2){
					menuItem = mRight;
				}
			}
		}

		int8_t addVal = 0;
		if (key == KEY1){
			addVal = -1;
		}
		if (key == KEY2){
			addVal = 1;
		}

		if (menuItem == mLeftEdit) {
			int16_t t = regulator.getTOn();
			t+=addVal;
			regulator.setTOn(t);
		}
		if (menuItem == mRightEdit) {
			int16_t t = regulator.getTOff();
			t+=addVal;
			regulator.setTOff(t);
		}
	}

	switch (menuItem) {
		case mLeft:{
			sysio.print(111);
			break;
		}

		case mLeftEdit:{
			sysio.print(regulator.getTOn());
			break;
		}

		case mRight:{
			sysio.print(222);
			break;
		}
		case mRightEdit:{
			sysio.print(regulator.getTOff());
			break;
		}
		default:
			sysio.print(Temperatura);
			break;
	}


	sysio.update();
}

void startServers() {
	Serial.print(">");
	Serial.println(__FUNCTION__);

	startFTP();
	startWebServer();
	Serial.println(__FUNCTION__);
}

void onStationDisconnect(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason){
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("ESP", "", AUTH_OPEN);
	Serial.println(__FUNCTION__);
}

void onStationConnect(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason){
	WifiAccessPoint.enable(false);
	Serial.println(__FUNCTION__);
}

void networkScanCompleted(bool succeeded, BssList list)
{
	if (succeeded)
	{
		for (int i = 0; i < list.count(); i++)
			if (!list[i].hidden && list[i].ssid.length() > 0)
				networks.add(list[i]);
	}
	networks.sort([](const BssInfo& a, const BssInfo& b){ return b.rssi - a.rssi; } );
	Serial.println(__FUNCTION__);
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	// Настройка wifi подключения
	// Если есть сохраненные настройки - загружаем.
	/*if (AppSettings.exist()){
		AppSettings.load();
		WifiAccessPoint.enable(false);
		WifiStation.config(AppSettings.ssid, AppSettings.password);
		if (!AppSettings.dhcp && !AppSettings.ip.isNull())
			WifiStation.setIP(AppSettings.ip, AppSettings.netmask, AppSettings.gateway);

		WifiStation.enable(true);
	}
	*/

	//WifiAccessPoint.config("ESP", "", AUTH_OPEN);
	//WifiAccessPoint.enable(true);

	sysio.initialize(5, 4, charTab, 2);
	sysio.setPoint(POINT2);
	regulator.regSwitchRelayCallback(onSwitchRelay);
	ReadTemp.Init(14);  			// select PIN It's required for one-wire initialization!
	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors
	updateTemperatureTimer.initializeMs(5000, readTemperature).start();   // every 5 seconds
	updateUITimer.initializeMs(200, updateUI).start(); // Обновление дисплея и сканирование кнопок

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Enable debug output to serial
	updateMenuTimer.initializeMs(2000, updateMenu).start();

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	//WifiStation.startScan(networkScanCompleted);

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);

	//WifiEvents.onStationDisconnect(onStationDisconnect);
	//WifiEvents.onStationConnect(onStationConnect);

	//System.onReady(startServers);
	Serial.println(__FUNCTION__);
}
