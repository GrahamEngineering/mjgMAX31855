/*
	mjgMAX31855 test on ESP8266
	
	To use on an Arduino, you will have to manually declare SPI pins
	
	This assumes you have SPI.h installed
	
	It will read information from the MAX31855 every 2.5 seconds and print it to serial

	>>>>> For the ESP8266, it is best to not have the MAX31855 connected during upload.  <<<<<

>>>>> If you read that, then you're going to save a lot of hassle. <<<<<
*/
#include <mjgMAX31855.h>

mjgMAX31855 tc = mjgMAX31855();
int timer = 2500;
long lastFireTime = millis();

void setup() 
{
	Serial.begin(115200);
	while (!Serial)
	{
		;	// Wait for Serial to begin
	}
	Serial.println("-----------------------");
	Serial.println("  mjgMAX31855 Example");
	Serial.println("-----------------------");
}
	

void loop()
{
	if (millis() - lastFireTime > timer)
	{
		lastFireTime = millis();
		
		tc.readRaw();

		if (tc.fault || tc.garbageFault || tc.rawData == 0)
		{
			Serial.println("---------------[ Fault ]---------------");
			Serial.println("Fault: " + String(tc.fault));
			Serial.println("tcFault: " + String(tc.tcFault));
			Serial.println("gndFault: " + String(tc.gndFault));
			Serial.println("vccFault: " + String(tc.vccFault));
			Serial.println("Garbage: " + String(tc.garbageFault));
			Serial.println("Raw Decimal: " + String(tc.rawData, DEC));
			Serial.println("Raw Binary: " + String(tc.rawData, BIN));
		}
		else
		{
			Serial.println("---------------[ Working ]---------------");
			Serial.println("Celsius: " + String(tc.tempC));
			Serial.println("Farenheit: " + String(tc.tempF));
			Serial.println("Internal C: " + String(tc.internalC));
			Serial.println("Internal F: " + String(tc.internalF));
			Serial.println("RAW Decimal: " + String(tc.rawData, DEC));
			Serial.println("RAW Binary: " + String(tc.rawData, BIN));
		}
	}
}
