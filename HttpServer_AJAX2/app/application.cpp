#include <user_config.h>
#include <SmingCore/SmingCore.h>
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

TM1637 tm;


char charTab[] = {0xEB,0x09,0xF1,0xB9,
		         0x1B,0xBA,0xFA,0x89,
		         0xFB,0xBB,0xDB,0x7A,
		         0xE2,0x79,0xF2,0xD2};//0~9,A,b,C,d,E,F


RelayRegulator regulator;

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "ASUS-8B3C" // Put you SSID and Password here
	#define WIFI_PWD "12345678"
#endif

HttpServer server;
FTPServer ftp;

DS18S20 ReadTemp;
Timer procTimer;
Timer displayUpdateTimer;

bool relayState = false;

int16_t Temperatura;
uint8_t sensorCount = 0;

void onSwitchRelay(bool state){
	relayState = state;
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendTemplate(tmpl); // this template object will be deleted automatically
}

void onFile(HttpRequest &request, HttpResponse &response)
{
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
}



void onAjaxTemperatura(HttpRequest &request, HttpResponse &response){
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
}

void onAjaxSetParams(HttpRequest &request, HttpResponse &response){
	int tUp = request.getQueryParameter("tup").toInt();
	int tDown = request.getQueryParameter("tdown").toInt();
	regulator.setOnOffTemperature(tUp, tDown);
}

void onAjaxGetParam(HttpRequest &request, HttpResponse &response) {
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& param = json.createNestedObject("param");
	param["tup"] = regulator.getUpT();
	param["tdown"] = regulator.getDownT();
	response.sendDataStream(stream, MIME_JSON);
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/ajax/temperatura", onAjaxTemperatura);
	server.addPath("/ajax/setparams", onAjaxSetParams);
	server.addPath("/ajax/getparams", onAjaxGetParam);
	server.setDefaultHandler(onFile);

	//Serial.println("\r\n=== NEW WEB SERVER STARTED ===");
	//Serial.println(WifiStation.getIP());
	//Serial.println("==============================\r\n");
}

void startFTP()
{
	if (!fileExist("index.html"))
		fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

void readData()
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
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	startFTP();
	startWebServer();
}

uint8_t sensorPos = 0;

void updateUI(){
	uint8_t key = tm.scanKey();
	tm.print(Temperatura);

	tm.update();
}

void init()
{
	tm.initialize(5, 4, charTab, 2);
	tm.setPoint(POINT2);
	regulator.regSwitchRelayCallback(onSwitchRelay);
	ReadTemp.Init(14);  			// select PIN It's required for one-wire initialization!
	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors

	procTimer.initializeMs(5000, readData).start();   // every 5 seconds
	displayUpdateTimer.initializeMs(200, updateUI).start(); // Обновление дисплея и сканирование кнопок
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Enable debug output to serial

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);
}
