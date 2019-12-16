/*
    v1.0.1 improved by ling zhou Aug.16, 2019
	tudzl@hotmail.de
	DHT12.h - Library for DHT12 sensor.
	v0.0.1 Beta
	Created by Bobadas, July 30,2016.
	Released into the public domain.
*/
#ifndef DHT12_h
#define DHT12_h
#include "Arduino.h"
#include "Wire.h"

#define CELSIUS		1
#define KELVIN		2
#define FAHRENHEIT	3
#define DHT12_CHIPID (0x5c)
//extern int8_t _cs;
//extern uint8_t _i2caddr;
class DHT12
{
	public:
		DHT12(byte scale=0,byte id=0);
		float readTemperature(byte scale=0);
		float readHumidity();
		bool begin(uint8_t addr);
		//uint8_t read8(byte reg); 
		//DHT12::DHT12(TwoWire *theWire = &Wire);
		//TwoWire *_wire; /**< Wire object */
	private:
		byte read();
		byte datos[5];
		byte _id;
		byte _scale;
};

#endif
