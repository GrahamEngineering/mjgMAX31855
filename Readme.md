# Matt Graham's MAX31855 Library

This library creates a MAX31855 object with the ability to read C and F temps for 
both the thermocouple as well as the internal cold junction.  Additional capabilites 
include fault detection

## Examples

ESP8266_Thermocouple - An example of outputs and basic configuration of the thermocouple object for an ESP8266 board.

## Compatible Hardware

This has been tested on the ESP8266 NodeMCU ESP-12E module.  It should work with the 
Arduino/Genuino UNO as well at the very least. Note that the default Hardware SPI pins 
are configured for the ESP, so if using on anything else you will definitely have to 
manually specify pins.

## Properties
- bool fault
- bool tcFault
- bool gndFault
- bool vccFault
- bool garbageFault
- double tempC
- double tempF
- double internalC
- double internalF
- uint32_t rawData

## Methods
- mjgMAX31855();
- mjgMAX31855(int csPin);
- mjgMAX31855(int csPin, int SPI_Speed);
- double readTempC();
- double readTempF();
- double readInternalC();
- double readInternalF();
- uint32_t readRaw();