/*!
 *  @file Adafruit_MCP9808.h
 *  modified by Zell, 11.11.2019, tudzl@hotmail.de
 * 	I2C Driver for Microchip's MCP9808 I2C Temp sensor
 *
 * 	This is a library for the Adafruit MCP9808 breakout:
 * 	http://www.adafruit.com/products/1782
 *
 * 	Adafruit invests time and resources providing this open source code,
 *please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *
 *	BSD license (see license.txt)
 */

#ifndef _ADAFRUIT_MCP9808_H
#define _ADAFRUIT_MCP9808_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
/*
0000 = RFU, Reserved for Future Use (Read-Only register)
0001 = Configuration register (CONFIG)
0010 = Alert Temperature Upper Boundary Trip register (TUPPER)
0011 = Alert Temperature Lower Boundary Trip register (TLOWER)
0100 = Critical Temperature Trip register (TCRIT)
0101 = Temperature register (TA)
0110 = Manufacturer ID register
0111 = Device ID/Revision register
1000 = Resolution register
*/
//Reg address
#define MCP9808_I2CADDR_DEFAULT 0x19 ///< chip I2C address
#define MCP9808_REG_CONFIG 0x01      ///< MCP9808 config register
//config Reg values
#define MCP9808_REG_CONFIG_Hyst 0x0200        ///< Limit Hysteresis bits, THYST 1.5 C
#define MCP9808_REG_CONFIG_SHUTDOWN 0x0100   ///< shutdown config
#define MCP9808_REG_CONFIG_SHUTDOWN 0x0100   ///< shutdown config
#define MCP9808_REG_CONFIG_CRITLOCKED 0x0080 ///< critical trip lock
#define MCP9808_REG_CONFIG_WINLOCKED 0x0040  ///< alarm window lock
#define MCP9808_REG_CONFIG_INTCLR 0x0020     ///< interrupt clear
#define MCP9808_REG_CONFIG_ALERTSTAT 0x0010  ///< alert output status
#define MCP9808_REG_CONFIG_ALERTCTRL 0x0008  ///< alert output control
#define MCP9808_REG_CONFIG_ALERTSEL 0x0004   ///< alert output select
#define MCP9808_REG_CONFIG_ALERTPOL 0x0002   ///< alert output polarity
#define MCP9808_REG_CONFIG_ALERTMODE 0x0001  ///< alert output to interrupt mode
//Reg address
#define MCP9808_REG_UPPER_TEMP 0x02   ///< upper alert boundary
#define MCP9808_REG_LOWER_TEMP 0x03   ///< lower alert boundery
#define MCP9808_REG_CRIT_TEMP 0x04    ///< critical temperature
#define MCP9808_REG_AMBIENT_TEMP 0x05 ///< ambient temperature/Temperature register (TA)
#define MCP9808_REG_MANUF_ID 0x06     ///< manufacture ID   The Manufacturer ID for the MCP9808 is 0x0054
#define MCP9808_REG_DEVICE_ID 0x07    ///< device ID  The Device ID for the MCP9808 is 0x04 (hex).
#define MCP9808_REG_RESOLUTION 0x08   ///< resolutin


/*!
 *    @brief  Class that stores state and functions for interacting with
 *            MCP9808 Temp Sensor
 */
class Adafruit_MCP9808 {
public:
  Adafruit_MCP9808();
  bool begin();
  //bool begin2(uint8_t addr);
  bool begin(TwoWire *theWire);
  bool begin(uint8_t addr);
  bool begin(uint8_t addr, TwoWire *theWire);

  bool init();
  uint16_t getRevision(uint8_t addr);
  float readTemperature();
  float readTemperature2();
  uint16_t readTempRaw();
  float readTempC();
  float readTempF();
  uint8_t getResolution(void);
  void setResolution(uint8_t value);

  void shutdown_wake(boolean sw);
  void shutdown(); //In shutdown, all power-consuming activities are disabled, though all registers can be written to or read.
  void wake();
  void wakeup();

  void write16(uint8_t reg, uint16_t val);
  uint16_t read16(uint8_t reg);

  void write8(uint8_t reg, uint8_t val);
  uint8_t read8(uint8_t reg);

private:
  TwoWire *_wire;
  uint8_t _i2caddr;
};

#endif
