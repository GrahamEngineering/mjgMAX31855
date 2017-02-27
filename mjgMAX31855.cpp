#include "Arduino.h"
#include "mjgMAX31855.h"
#include <SPI.h>

/*
	Author: matthew.j.graham@gmail.com
	
	Updated: 27 Feb 2017
		
	A lot of this was lifted from Adafruit's MAX31855 library - but I couldn't get that one to compile in my environment, so I wrote this one.
	
	They had some nice bit-shifting methods for extracting the temperature values, so re-used those, although they did have some unneccessary masking going on, so I simplified it so hopefully a new user can understand it.
	
	The SPI interface will send 32 bits in 8-bit chunks.  The left-most bits (31:23) come in first, then get shifted to the left 8 bits.  
	
		Example:
			The first eight bits are all ones (11111111)
			Receive them into the variable, then bit-shift them left 8 bits.  The variable now holds:
				1111111100000000
			Read the next eight bits.  For this example, they are (10101010)
			Variable now holds:
				1111111110101010
			Shift the bits to the left eight more bits. The variable now holds:
				111111111010101000000000
			Read the next eight bits (00001111)
			The variable now holds:
				111111111010101000001111
			Shift the 8 bits to the left one final time and the var holds:
				11111111101010100000111100000000
			Receive the last 8 bits (11001100)
			The var now holds:
				11111111101010100000111111001100
			For a total of 32 bits.  
			
			31:18 is the 14-bit signed integer that holds the Temperature in degrees C
				11111111101010
			17 is Reserved and should always be 0 (This code throws a garbage fault if it is not 0)
							  1
			16 is the fault indicator.  A 1 signals that the code needs to check for a specific type of fault
							   0
			15:4 is the 12-bit signed int for the reference junction temp
				                000011111100
			3 is Reserved and should always be 0 (This code throws a garbage fault if it is not 0)
											1
			2 represents a short to VCC when set to 1
											 1
			1 represents a short to ground when set to 1
											  0
			0 represents a thermocouple open (no connection) when set to 1
											   0
											   
											   
			MAX31855 Datasheet: https://datasheets.maximintegrated.com/en/ds/MAX31855.pdf
			
			A note about Hardware SPI:
				On the ESP-12E:
					- connect the DO pin from the MAX31855 to the MISO (Master-In Slave Out) pin (D6, GPIO12)
					- connect the CLK/SCK pin from the MAX31855 to the CLK pin of the ESP-12E (D5, GPIO14)
					- connect the CS pin from the MAX31855 to the designated CS pin in the constructor.  If no CS Pin is declared in the constructor, default is D8, GPIO15
				On the Arduino/Genuino UNO:
					Not documented or tested.  Need to see if it works sometime and update here.
*/

mjgMAX31855::mjgMAX31855()
{
	/*
		Default constructor
			- SPI Speed: 14000000
			- CS Pin: ESP8266 D8 (GPIO 15)
	*/
	_init();
}

mjgMAX31855::mjgMAX31855(int userDefinedCSPin)
{
	/*
		Constructor with CS Pin specified
			- SPI Speed: 14000000
			- CS Pin: user-provided
	*/
	_csPin = userDefinedCSPin;
	_init();
}

mjgMAX31855::mjgMAX31855(int userDefinedCSPin, int SPISpeed)
{
	/*
		Constructor where user specified speed and CS Pin
	*/
	_csPin = userDefinedCSPin;
	_SPISpeed = SPISpeed;
	_init();
}

void 		mjgMAX31855::_init()
{
	/*
		Initializes the thermocouple object.
		Verify that the CS Pin is set to HIGH so that it doesn't think it's in reading mode.
	*/
	pinMode(_csPin, OUTPUT);
	digitalWrite(_csPin, HIGH);
	SPI.begin();
}

void		mjgMAX31855::_resetVars()
{
	/*
		Clear all properties
	*/
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
	/*
		Reads the raw thermocouple 32-bit value.
		
		Checks for faults as well, including garbage bits set on 17 and 3
	*/
	
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
  
  if (rawData)
  {
	  /*
		Set all property values with the new data.
	  */
	  _setTemps();
  }
 
  return rawData;
}

double 		mjgMAX31855::readTempC(void)
{
	/*
		Read the thermocouple (which sets all properties, then return the tempC property)
	*/
	readRaw();
	return tempC;
}

double 		mjgMAX31855::readTempF(void)
{
	/*
		Read the thermocouple (which sets all properties, then return the tempF property)
	*/
	readRaw();
	return tempF;
}

double 		mjgMAX31855::readInternalC(void)
{
	/*
		Read the thermocouple (which sets all properties, then return the internalC property)
	*/
  readRaw();
  return internalC;
}

double 		mjgMAX31855::readInternalF(void)
{
	/*
		Read the thermocouple (which sets all properties, then return the internalF property)
	*/
  readRaw();
  return internalF;
}

void 		mjgMAX31855::_setTemps()
{
	/*
		Set all temperature values (AKA: Where the magic happens)
	*/
	uint32_t v = rawData;
	v >>= 18;
    
    if (bitRead(v, 31)) {
      // Negative value, mask it.
      v = 0xFFFFC000 + v;
    }
    
    tempC = v;

    // LSB = 0.25 degrees C
    tempC *= 0.25;   // This works because they use 2 bits for decimal (of which there can be four values (00, 01, 10 and 11) representing .00 .25 .50 and .75
	
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