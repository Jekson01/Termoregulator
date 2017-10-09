#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <inttypes.h>
#include <Libraries/OneWire/OneWire.h>

#include <Libraries/DS18S20/ds18s20.h>

#include "TM1637.h"


DS18S20 ReadTemp;
Timer procTimer;
TM1637 disp;

bool blink;

#define LED_PIN 2
//**********************************************************
// DS18S20 example, reading
// You can connect multiple sensors to a single port
// (At the moment 4 pcs - it depends on the definition in the library)
// Measuring time: 1.2 seconds * number of sensors
// The main difference with the previous version of the demo:
//  - Do not use the Delay function () which discourages by manufacturer of ESP8266
//  - We can read several sensors
// Usage:
//  Call Init to setup pin eg. ReadTemp.Init(2);   //pin 2 selected
//  Call ReadTemp.StartMeasure();
//   if ReadTemp.MeasureStatus() false read sensors
//   You can recognize sensors by the ID or index.
//***********************************************************
void readData()
{
	uint8_t a;
		uint64_t info;

		if (!ReadTemp.MeasureStatus())  // the last measurement completed
		{
	      if (ReadTemp.GetSensorsCount())   // is minimum 1 sensor detected ?
			Serial.println("******************************************");
		    Serial.println(" Reding temperature DEMO");
		    for(a=0;a<ReadTemp.GetSensorsCount();a++)   // prints for all sensors
		    {
		      Serial.print(" T");
		      Serial.print(a+1);
		      Serial.print(" = ");
		      if (ReadTemp.IsValidTemperature(a))   // temperature read correctly ?
		        {
		    	  float T = ReadTemp.GetCelsius(a);
		    	  Serial.print(T);
		    	  disp.print((uint16_t)(T * 10));
		    	  disp.update();
		    	  Serial.print(" Celsius, (");
		    	  Serial.print(ReadTemp.GetFahrenheit(a));
		    	  Serial.println(" Fahrenheit)");
		        }
		      else
		    	  Serial.println("Temperature not valid");

		      Serial.print(" <Sensor id.");

		      info=ReadTemp.GetSensorID(a)>>32;
		      Serial.print((uint32_t)info,16);
		      Serial.print((uint32_t)ReadTemp.GetSensorID(a),16);
		      Serial.println(">");
		    }
			Serial.println("******************************************");
			ReadTemp.StartMeasure();  // next measure, result after 1.2 seconds * number of sensors
		}
		else
			Serial.println("No valid Measure so far! wait please");



}

void init()
{
	pinMode(LED_PIN, OUTPUT);

	disp.setPoint(POINT2);
	disp.print(0);
	disp.update();
	ReadTemp.Init(14);  			// select PIN It's required for one-wire initialization!
	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors

	procTimer.initializeMs(1500, readData).start();   // every 10 seconds

	//procTimer.initializeMs(2000, readData).start();   // every 10 seconds
}

