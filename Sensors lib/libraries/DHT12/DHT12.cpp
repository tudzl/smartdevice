/*  
   
    v1.0.1 improved by ling zhou Aug.16, 2019
	tudzl@hotmail.de
	DHT12.cpp - Library for DHT12 sensor.
	v0.0.1 Beta
	Created by Bobadas, July 30,2016.
	Released into the public domain.
*/
#include "Arduino.h"
#include "Wire.h"
#include "DHT12.h"




DHT12::DHT12(byte scale,byte id)
{
	if (id==0 || id>126) _id=0x5c;
	else _id=id;
	if (scale==0 || scale>3) _scale=CELSIUS;
	else _scale=scale;
}

//written by  zell 2019.aug
bool DHT12::begin(uint8_t addr) {
  //_i2caddr = addr;
  
  delay(100);
  if (DHT12::readHumidity() < 1)
    return false;
  else
    return true;
  
}

/*
bool DHT12::begin(uint8_t addr) {
  _i2caddr = addr;
   _cs = -1;
  if (_cs == -1) {
    // i2c
    _wire->begin();
  } else {
    digitalWrite(_cs, HIGH);
    pinMode(_cs, OUTPUT);

    if (_sck == -1) {
      // hardware SPI
      _spi->begin();
    } else {
      // software SPI
      pinMode(_sck, OUTPUT);
      pinMode(_mosi, OUTPUT);
      pinMode(_miso, INPUT);
    }
  }
    delay(100);
	
  if (DHT12.readHumidity() < 1)
    return false;
  else
    return true;
  
}


uint8_t DHT12::read8(byte reg) {
  uint8_t value;

  if (_cs == -1) {
    _wire->beginTransmission((uint8_t)_i2caddr);
    _wire->write((uint8_t)reg);
    _wire->endTransmission();
    _wire->requestFrom((uint8_t)_i2caddr, (byte)1);
    value = _wire->read();

  } else {
    if (_sck == -1)
      _spi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg | 0x80); // read, bit 7 high
    value = spixfer(0);
    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      _spi->endTransaction(); // release the SPI bus
  }
  return value;
}

*/

byte DHT12::read()
{  //byte _id
    uint8_t IIC_add = (uint8_t) _id;
	Wire.beginTransmission(IIC_add);
	Wire.write(0);
	if (Wire.endTransmission()!=0) return 1;  
	Wire.requestFrom(IIC_add,  (uint8_t) 5);
	//Wire.requestFrom(uint8_t _id, uint8_t 5); //ori
	for (uint8_t i=0;i<5;i++) {
		datos[i]=Wire.read();
	};
	delay(50);
	if (Wire.available()!=0) return 2;
	if (datos[4]!=(datos[0]+datos[1]+datos[2]+datos[3])) return 3;
	return 0;
}

float DHT12::readTemperature(byte scale)
{
	float resultado=0;
	byte error=read();
	if (error!=0) return (float)error/100;
	if (scale==0) scale=_scale;
	switch(scale) {
		case CELSIUS:
			resultado=(datos[2]+(float)datos[3]/10);
			break;
		case FAHRENHEIT:
			resultado=((datos[2]+(float)datos[3]/10)*1.8+32);
			break;
		case KELVIN:
			resultado=(datos[2]+(float)datos[3]/10)+273.15;
			break;
	};
	return resultado;
}

float DHT12::readHumidity()
{
	float resultado;
	byte error=read();
	if (error!=0) return (float)error/100;
	resultado=(datos[0]+(float)datos[1]/10);
	return resultado;
}

