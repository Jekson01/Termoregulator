#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <Libraries/DS18S20/ds18s20.h>
#include "TM1637.h"

TM1637 disp;

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "ASUS-8B3C" // Put you SSID and Password here
	#define WIFI_PWD "12345678"
#endif

HttpServer server;
FTPServer ftp;

DS18S20 ReadTemp;
Timer procTimer;

float Temperatura;

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
	json["status"] = (bool)true;
	json["value"] = Temperatura;

	response.sendDataStream(stream, MIME_JSON);
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/ajax/temperatura", onAjaxTemperatura);
	server.setDefaultHandler(onFile);

	Serial.println("\r\n=== NEW WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
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
	uint8_t a;
		uint64_t info;

		if (!ReadTemp.MeasureStatus())  // the last measurement completed
		{
	      if (ReadTemp.GetSensorsCount())   // is minimum 1 sensor detected ?
			//Serial.println("******************************************");
		    //Serial.println(" Reding temperature DEMO");
		    for(a=0;a<ReadTemp.GetSensorsCount();a++)   // prints for all sensors
		    {
		      //Serial.print(" T");
		      //Serial.print(a+1);
		      //Serial.print(" = ");
		      if (ReadTemp.IsValidTemperature(a))   // temperature read correctly ?
		        {
		    	  float T = ReadTemp.GetCelsius(a);
		    	  //Serial.print(T);
		    	  Temperatura = T;
		    	  disp.print((uint16_t)(T * 10));
		    	  disp.update();
		    	  //Serial.print(" Celsius, (");
		    	  //Serial.print(ReadTemp.GetFahrenheit(a));
		    	  //Serial.println(" Fahrenheit)");
		        }
		      else
		    	  Serial.println("Temperature not valid");

		      //Serial.print(" <Sensor id.");

		      //info=ReadTemp.GetSensorID(a)>>32;
		      //Serial.print((uint32_t)info,16);
		      //Serial.print((uint32_t)ReadTemp.GetSensorID(a),16);
		      //Serial.println(">");
		    }
			Serial.println("******************************************");
			ReadTemp.StartMeasure();  // next measure, result after 1.2 seconds * number of sensors
		}
		else{
			//Serial.println("No valid Measure so far! wait please");
		}



}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	startFTP();
	startWebServer();
}

void init()
{
	disp.setPoint(POINT2);
	disp.print(0);
	disp.update();
	ReadTemp.Init(14);  			// select PIN It's required for one-wire initialization!
	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors

	procTimer.initializeMs(5000, readData).start();   // every 10 seconds
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);
}
