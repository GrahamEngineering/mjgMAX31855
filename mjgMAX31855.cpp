#include "Arduino.h"
#include "mjgMAX31855.h"
#include <SPI.h>

mjgMAX31855::mjgMAX31855()
{
	_init();
}

mjgMAX31855::mjgMAX31855(int userDefinedCSPin)
{
	_csPin = userDefinedCSPin;
	_init();
}

mjgMAX31855::mjgMAX31855(int userDefinedCSPin, int SPISpeed)
{
	_csPin = userDefinedCSPin;
	_SPISpeed = SPISpeed;
	_init();
}

void 		mjgMAX31855::_init()
{
	pinMode(_csPin, OUTPUT);
	digitalWrite(_csPin, HIGH);
	SPI.begin();
}

void		mjgMAX31855::_resetVars()
{
	fault = 0;
	tcFault = 0;
	gndFault = 0;
	vccFault = 0;
	garbageFault = 0;
	tempC = 0;
	tempF = 0;
	internalC = 0;
	internalF = 0;
}

uint32_t 	mjgMAX31855::readRaw(void)
{
  _resetVars();
  
  digitalWrite(_csPin, LOW);  // Begin read from MAX31855
  
  SPI.beginTransaction(SPISettings(_SPISpeed, MSBFIRST, SPI_MODE0));

  rawData = 0;

  rawData = SPI.transfer(0);
  rawData <<= 8;
  rawData += SPI.transfer(0);
  rawData <<= 8;
  rawData += SPI.transfer(0);
  rawData <<= 8;
  rawData += SPI.transfer(0);

  SPI.endTransaction();

  digitalWrite(_csPin, HIGH);

  if (bitRead(rawData, 16))
  {
	fault = 1;
  }

  // Specific fault info;
  if (bitRead(rawData, 0))
  {
    tcFault = 1;
    return NAN;
  }
  else if (bitRead(rawData, 1))
  {
	gndFault = 1;
    return NAN;
  }
  else if (bitRead(rawData, 2))
  {
    vccFault = 1;
    return NAN;
  }
  else if (bitRead(rawData, 17) || bitRead(rawData, 3))
  {
    garbageFault = 1;
    return NAN;
  }
  
  if (rawData)	// We have data, set everything.
  {
	  _setTemps();
  }
 
  return rawData;
}

double 		mjgMAX31855::readTempC(void)
{
	readRaw();
	return tempC;
}

double 		mjgMAX31855::readTempF(void)
{
	readRaw();
	return tempF;
}

double 		mjgMAX31855::readInternalC(void)
{
  readRaw();
  return internalC;
}

double 		mjgMAX31855::readInternalF(void)
{
  readRaw();
  return internalF;
}

void 		mjgMAX31855::_setTemps()
{
	uint32_t v = rawData;
	v >>= 18;
    
    if (bitRead(v, 31)) {
      // Negative value, mask it.
      v = 0xFFFFC000 + v;
    }
    
    tempC = v;

    // LSB = 0.25 degrees C
    tempC *= 0.25;   // genius - works because they use 2 bits for decimal (of which there can be four values (00, 01, 10 and 11) representing .00 .25 .50 and .75
	
	tempF = (tempC * (9/5)) + 32;
	
	v = rawData;
	// Looking for a 12-bit number in bits 15:4
    //  Shift the whole thing to the left until 15 is in the 31st bit spot 
    //  Then shift the whole thing to the right until 4 is in the 0 bit spot
    v <<= 16;
    v >>= 20;

    if (bitRead(v, 11))
    {
      // Negative number, mask and pad and whatnot
      v = 0xFFFFF000 + v;
    }

    internalC = v * .0625;

	internalF = (internalC * (9/5)) + 32;
}