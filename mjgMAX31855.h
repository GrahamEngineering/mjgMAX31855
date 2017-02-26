/*
 *  mjgMAX31855.h
 *
 */
#ifndef mjgMAX31855_h
#define mjgMAX31855_h

#include "Arduino.h"

class mjgMAX31855
{
  public:
    mjgMAX31855();
	mjgMAX31855(int);	//Allow user to specify CS pin
	mjgMAX31855(int, int);	// Allow user to specify CS Pin and Speed
	double readTempC();
	double readTempF();
	double readInternalC();
	double readInternalF();
	uint32_t readRaw();
	bool fault = 0;
	bool tcFault = 0;
	bool gndFault = 0;
	bool vccFault = 0;
	bool garbageFault = 0;
	double tempC = 0;
	double tempF = 0;
	double internalC = 0;
	double internalF = 0;
	uint32_t rawData = 0;
	
  private:
	void _init();
	void _resetVars();
	int _csPin = D8;	// Assume ESP8266 pin D8 (GPIO 15 for CS)
	int _SPISpeed = 14000000;	// Go fast or go home.
	void _setTemps();
	
};


#endif
