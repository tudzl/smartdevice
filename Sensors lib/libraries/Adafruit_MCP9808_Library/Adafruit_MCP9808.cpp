/*!
 *  @file Adafruit_MCP9808.cpp
 *
 *  @mainpage Adafruit MCP9808 I2C Temp Sensor
 *
 *  @section intro_sec Introduction
 *
 * 	I2C Driver for Microchip's MCP9808 I2C Temp sensor
 *
 * 	This is a library for the Adafruit MCP9808 breakout:
 * 	http://www.adafruit.com/products/1782
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *  @section author Author
 *
 *  K.Townsend (Adafruit Industries)
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v1.0 - First release
 */

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#ifdef __AVR_ATtiny85__
#include "TinyWireM.h"
#define Wire TinyWireM
#else
#include <Wire.h>
#endif

#include "Adafruit_MCP9808.h"

/*!
 *    @brief  Instantiates a new MCP9808 class
 */
Adafruit_MCP9808::Adafruit_MCP9808() {}

/*!
 *    @brief  Setups the HW
 *    @param  *theWire
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_MCP9808::begin(TwoWire *theWire) {
  _wire = theWire;
  _i2caddr = MCP9808_I2CADDR_DEFAULT;
  return init();
}

/*!
 *    @brief  Setups the HW
 *    @param  addr
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_MCP9808::begin(uint8_t addr) {
  _i2caddr = addr;
  _wire = &Wire;
  _wire->begin();
  return init();
}

/*!   wiritten by ZL
 *    @brief  Setups the HW
 *    @param  addr
 *    @return True if initialization was successful, otherwise false.
 */
 /*
bool Adafruit_MCP9808::begin2(uint8_t addr) {
  _i2caddr = addr;
  _wire = &Wire;
  _wire->begin();
  return init();
}
*/


/*!
 *    @brief  Setups the HW
 *    @param  addr
 *    @param  *theWire
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_MCP9808::begin(uint8_t addr, TwoWire *theWire) {
  _i2caddr = addr;
  _wire = theWire;
  return init();
}

/*!
 *    @brief  Setups the HW with default address
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_MCP9808::begin() {
  _i2caddr = MCP9808_I2CADDR_DEFAULT;
  _wire = &Wire;
  return init();
}

/*!
 *    @brief  init function
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_MCP9808::init() {
  if (read16(MCP9808_REG_MANUF_ID) != 0x0054)
    return false;
  if (read16(MCP9808_REG_DEVICE_ID) != 0x0400)
    return false;

  write16(MCP9808_REG_CONFIG, 0x0);
  return true;
}

/*!   wiritten by ZL
 *    @brief  return the chip Revision
 *    @param  addr
 *    @return 16 bitset if initialization was successful, otherwise false.
 */

uint16_t Adafruit_MCP9808::getRevision(uint8_t addr)

{

  uint16_t res = read16(MCP9808_REG_DEVICE_ID);
  //0x0400)
   //revision starts from 0x00
   res &= 0x00FF;
  //write16(MCP9808_REG_CONFIG, 0x0);
  return res;
}

/*!  seems not correct!  Temperature = (mcp_temp_H * 16 + Temperature / 16.0);
 *   @brief  Reads the 16-bit temperature register and returns the Centigrade
 *           temperature as a float.
 *   @return Temperature in Centigrade.
 */
float Adafruit_MCP9808::readTempC() {
  uint16_t t = read16(MCP9808_REG_AMBIENT_TEMP);

  float temp = t & 0x0FFF;
  temp /= 16.0;
  if (t & 0x1000)
    temp -= 256;

  return temp;
}


/*!   wiritten by ZL
 *   Temperature = (mcp_temp_H * 16 + Temperature / 16.0);
 *   @brief  Reads the 16-bit temperature register and returns the raw values
 */
uint16_t Adafruit_MCP9808::readTempRaw() {
  uint16_t t = read16(MCP9808_REG_AMBIENT_TEMP);

  //float temp = t & 0x0FFF;
  //temp /= 16.0;
  //if (t & 0x1000)
  //  temp -= 256;

  return t;
}

/*!   wiritten by ZL
 *   Temperature = (mcp_temp_H * 16 + Temperature / 16.0);
 *   @brief  Reads the 16-bit temperature register and returns the Centigrade
 *           temperature as a float.
 *   @return Temperature in Centigrade.
 */
float Adafruit_MCP9808::readTemperature() {
  uint16_t t = read16(MCP9808_REG_AMBIENT_TEMP);

   //0x1F  to clear flags
  uint8_t  Hi =  (t & 0x1F00) >>8;
  uint8_t  Lo =  (t & 0x00FF) ;
  float temp =Lo;
  /*
  UpperByte = UpperByte & 0x1F; //Clear flag bits
if ((UpperByte & 0x10) == 0x10){ //TA < 0°C
UpperByte = UpperByte & 0x0F; //Clear SIGN
Temperature = 256 - (UpperByte x 16 + LowerByte / 16);
}else //TA ³ 0°C
Temperature = (UpperByte x 16 + LowerByte / 16);
//Temperature = Ambient Temperature (°C)
  */
  if ((Hi& 0x10)== 0x10){
	  //negative
	  Hi = Hi & 0x0F; //Clear SIGN
      temp = 256.0 - (Hi * 16 + temp / 16.0);
  }
  else   temp = (Hi * 16 + temp / 16.0);
  
  return temp;
}

/*!   wiritten by ZL, test only 
 *   Temperature = (mcp_temp_H * 16 + Temperature / 16.0);
 *   @brief  Reads the 16-bit temperature register , abstract low 8 bits and returns the Centigrade
 *           temperature as a float.
 *   @return Temperature in Centigrade.
 */
float Adafruit_MCP9808::readTemperature2() {
  uint16_t t = read16(MCP9808_REG_AMBIENT_TEMP);

   //0x1F  to clear flags
  //uint8_t  Hi =  (t & 0x1F00) >>8;
  uint8_t  Lo =  (t & 0x00FF) ;
  float temp =(float)Lo/16.0;
  

  
  return temp;
}


/*!
 *   @brief  Reads the 16-bit temperature register and returns the Fahrenheit
 *           temperature as a float.
 *   @return Temperature in Fahrenheit.
 */
float Adafruit_MCP9808::readTempF() {
  uint16_t t = read16(MCP9808_REG_AMBIENT_TEMP);

  float temp = t & 0x0FFF;
  temp /= 16.0;
  //bit 12 Sign: Sign bit
  if (t & 0x1000)
    temp -= 256;

  temp = temp * 9.0 / 5.0 + 32;

  return temp;
}

/*!
 *   @brief  Set Sensor to Shutdown-State or wake up (Conf_Register BIT8)
 *   @param  sw true = shutdown / false = wakeup
 *   In shutdown, all power-consuming activities are disabled, though all registers can be written to or read.
 */
void Adafruit_MCP9808::shutdown_wake(boolean sw) {
  uint16_t conf_shutdown;
  uint16_t conf_register = read16(MCP9808_REG_CONFIG);
  if (sw == true) {
    conf_shutdown = conf_register | MCP9808_REG_CONFIG_SHUTDOWN;
    write16(MCP9808_REG_CONFIG, conf_shutdown);
  }
  if (sw == false) {
    conf_shutdown = conf_register & ~MCP9808_REG_CONFIG_SHUTDOWN;
    write16(MCP9808_REG_CONFIG, conf_shutdown);
  }
}

/*!
 *   @brief  Shutdown MCP9808
 */
void Adafruit_MCP9808::shutdown() { shutdown_wake(true); }

/*!
 *   @brief  Wake up MCP9808
 *   delay 250mS
 */
void Adafruit_MCP9808::wake() {
  shutdown_wake(false);
  delay(250);
}

/*!
 *   @brief  Wake up MCP9808
 *   NO delays
 */
void Adafruit_MCP9808::wakeup() {
  shutdown_wake(false);
  //delay(250);
}
/*!
 *   @brief  Get Resolution Value
 *   @return Resolution value
 */
uint8_t Adafruit_MCP9808::getResolution() {
  return read8(MCP9808_REG_RESOLUTION);
}

/*!
 *   @brief  Set Resolution Value
 *   @param  value
 */
void Adafruit_MCP9808::setResolution(uint8_t value) {
  write8(MCP9808_REG_RESOLUTION, value & 0x03);
}

/*!
 *    @brief  Low level 16 bit write procedures
 *    @param  reg
 *    @param  value
 */
void Adafruit_MCP9808::write16(uint8_t reg, uint16_t value) {
  _wire->beginTransmission(_i2caddr);
  _wire->write((uint8_t)reg);
  _wire->write(value >> 8);
  _wire->write(value & 0xFF);
  _wire->endTransmission();
}

/*!
 *    @brief  Low level 16 bit read procedure
 *    @param  reg
 *    @return value
 */
uint16_t Adafruit_MCP9808::read16(uint8_t reg) {
  //uint16_t val = 0xFFFF;  //ori
  uint16_t val ;
  uint8_t state;

  _wire->beginTransmission(_i2caddr);
  _wire->write((uint8_t)reg);
  state = _wire->endTransmission();

  if (state == 0) {
    _wire->requestFrom((uint8_t)_i2caddr, (uint8_t)2);
    val = _wire->read();
    val <<= 8;
    val |= _wire->read();
  }

  return val;
}

/*!
 *    @brief  Low level 8 bit write procedure
 *    @param  reg
 *    @param  value
 */
void Adafruit_MCP9808::write8(uint8_t reg, uint8_t value)
{
  _wire -> beginTransmission(_i2caddr);
  _wire -> write((uint8_t)reg);
  _wire -> write(value);
  _wire -> endTransmission();
}

/*!
 *    @brief  Low level 8 bit read procedure
 *    @param  reg
 *    @return value
 */
uint8_t Adafruit_MCP9808::read8(uint8_t reg)
{
  uint8_t val = 0xFF;
  uint8_t state;

  _wire -> beginTransmission(_i2caddr);
  _wire -> write((uint8_t)reg);
  state = _wire -> endTransmission();

  if (state == 0)
  {
    _wire -> requestFrom((uint8_t)_i2caddr, (uint8_t)1);
    val = _wire -> read();
  }

  return val;
}
