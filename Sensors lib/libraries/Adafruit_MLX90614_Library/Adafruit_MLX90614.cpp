/*************************************************** 
  This is a library for the MLX90614 Temp Sensor
  modified by zell 2019.12.2
  Designed specifically to work with the MLX90614 sensors in the
  adafruit shop
  ----> https://www.adafruit.com/products/1748
  ----> https://www.adafruit.com/products/1749

  These sensors use I2C to communicate, 2 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_MLX90614.h"

Adafruit_MLX90614::Adafruit_MLX90614(uint8_t i2caddr) {
  _addr = i2caddr;
}


boolean Adafruit_MLX90614::begin(void) {
  Wire.begin();

  /*
  for (uint8_t i=0; i<0x20; i++) {
    Serial.print(i); Serial.print(" = ");
    Serial.println(read16(i), HEX);
  }
  */
  return true;
}

///////////////////by zell///////////////////////////////////

boolean Adafruit_MLX90614::begin2(void) {
  Wire.begin();


 /*
  if (id != ICM20948_ACCELGYRO_ID)
  {
	 Serial.print("ICM20948 chip ID readout: ");
     Serial.println(id, HEX);
    return false;
  }
  */
  //uint32_t ID = 0;
  uint8_t id1 = read8(MLX90614_I2CADDR,MLX90614_ID1);
  uint8_t id2 = read8(MLX90614_I2CADDR,MLX90614_ID2);
  uint8_t id3 = read8(MLX90614_I2CADDR,MLX90614_ID3);
  uint8_t id4 = read8(MLX90614_I2CADDR,MLX90614_ID4);
  //ID = id1<<24 + id2<<16+ id3<<8 +id4 ; //bugs
  if( (0!= id1)&&(0xFF!= id1) ) {
	  Serial.print("MLX90614 chip ID readout: ");
	  Serial.print(id1, HEX);
	  Serial.print(id2, HEX);
	  Serial.print(id3, HEX);
	  Serial.println(id4, HEX);
      //Serial.println(ID, HEX);
	  //    MLX90614_EMISS
	 // float EMISS=readEmissivity();
	//  Serial.printf("MLX90614 default Emissivity: %.2f\r\n",EMISS);
	  //Serial.println("");
  return true;
  }
  else return false;
  
  
}

//////////////////////////////////////////////////////

float Adafruit_MLX90614::readEmissivity(void) {
  float EMISS;
  
  EMISS = read16(MLX90614_EMISS);
  EMISS = EMISS/65535.0 ;
  return EMISS;
}


double Adafruit_MLX90614::readObjectTempF(void) {
  return (readTemp(MLX90614_TOBJ1) * 9 / 5) + 32;
}


double Adafruit_MLX90614::readAmbientTempF(void) {
  return (readTemp(MLX90614_TA) * 9 / 5) + 32;
}

double Adafruit_MLX90614::readObjectTempC(void) {
  return readTemp(MLX90614_TOBJ1);
}


double Adafruit_MLX90614::readAmbientTempC(void) {
  return readTemp(MLX90614_TA);
}

float Adafruit_MLX90614::readTemp(uint8_t reg) {
  float temp;
  
  temp = read16(reg);
  temp *= .02;
  temp  -= 273.15;
  return temp;
}

/*********************************************************************/

uint16_t Adafruit_MLX90614::read16(uint8_t a) {
  uint16_t ret;

  Wire.beginTransmission(_addr); // start transmission to device 
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(false); // end transmission
  
  Wire.requestFrom(_addr, (uint8_t)3);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  ret |= Wire.read() << 8; // receive DATA

  uint8_t pec = Wire.read(); //error bits

  return ret;
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in the Arduino wire library
			for any address
    @param address The i2c address of the device
	@param reg The register address to read from
*/
/**************************************************************************/
byte Adafruit_MLX90614::read8(byte address, byte reg)
{
  byte value;

  Wire.beginTransmission((byte)address);
  #if ARDUINO >= 100
    Wire.write((uint8_t)reg);
  #else
    Wire.send(reg);
  #endif
  if (Wire.endTransmission(false) != 0) return 0;
  Wire.requestFrom((byte)address, (byte)1);
  #if ARDUINO >= 100
    value = Wire.read();
  #else
    value = Wire.receive();
  #endif

  return value;
}
