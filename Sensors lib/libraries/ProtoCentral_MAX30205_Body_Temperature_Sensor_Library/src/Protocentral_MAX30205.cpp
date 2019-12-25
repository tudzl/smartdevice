//////////////////////////////////////////////////////////////////////////////////////////
//
//    Arduino library for the MAX30205 body temperature sensor breakout board
//
//    Author: Ashwin Whitchurch
//    Copyright (c) 2018 ProtoCentral
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//   For information on how to use, visit https://github.com/protocentral/ProtoCentral_MAX30205
/////////////////////////////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include <Wire.h>
#include "Protocentral_MAX30205.h"


// T reg: degree=value*0.00390625, in degree


float MAX30205::getTemperature(void){
	uint8_t readRaw[2] = {0};
    I2CreadBytes(MAX30205_ADDRESS,MAX30205_TEMPERATURE, &readRaw[0] ,2); // read two bytes
	int16_t raw = readRaw[0] << 8 | readRaw[1];  //combine two bytes
    temperature = raw  * 0.00390625;     // convert to temperature
	return  temperature;
}

void MAX30205::shutdown(void){
  uint8_t reg = I2CreadByte(MAX30205_ADDRESS, MAX30205_CONFIGURATION);  // Get the current register
  I2CwriteByte(MAX30205_ADDRESS, MAX30205_CONFIGURATION, reg | 0x80);
}

void MAX30205::begin(void){
  I2CwriteByte(MAX30205_ADDRESS, MAX30205_CONFIGURATION, 0x00); //mode config
  I2CwriteByte(MAX30205_ADDRESS, MAX30205_THYST , 		 0x00); // set threshold
  I2CwriteByte(MAX30205_ADDRESS, MAX30205_TOS, 			 0x00); //
}

//by zell, check seneor existence first
//have bugs regarding for REG writes
boolean MAX30205::begin2(void){
	
  Wire.beginTransmission(MAX30205_ADDRESS);
  byte  error = Wire.endTransmission();
  if (error == 0)
    {
	I2CwriteBytes(MAX30205_ADDRESS, MAX30205_CONFIGURATION, 0x00); //mode config
	//I2CwriteByte(MAX30205_ADDRESS, MAX30205_THYST , 		 0x2600); // 9728            ;def 75 degree, set overtemperature  sink threshold
	I2CwriteBytes(MAX30205_ADDRESS, MAX30205_THYST , 		 0x1D60); //29 degree 0x1D60, 30 degree 0x1e00  1 degree = 0x00A0
	//I2CwriteByte(MAX30205_ADDRESS, MAX30205_TOS, 			 0x2800); //0x2800 10240*0.00390625
	//alarm
	I2CwriteBytes(MAX30205_ADDRESS, MAX30205_TOS, 			 0x1E00);  //30 degree 0x1e00 
	//Serial.println("config Thyst and TOS finished");
	//40 degree  10240 dec, 0x2800; 38 degree , 9728  0x2600
	return true;
	}
  else return false;
}


//modified by zl
void MAX30205::printRegisters(void){
  Serial.print("Raw Temp Hi bytes:");
  Serial.println(I2CreadWord(MAX30205_ADDRESS, MAX30205_TEMPERATURE),  BIN); //16-Bit (0.00390625°C) Temperature Resolution
  Serial.print("Config Reg:");
  Serial.println(I2CreadByte(MAX30205_ADDRESS, MAX30205_CONFIGURATION),  BIN);
  Serial.print("THYST Reg:");
  Serial.println(I2CreadWord(MAX30205_ADDRESS, MAX30205_THYST), BIN);
  Serial.print("TOS Reg:");
  Serial.println(I2CreadWord(MAX30205_ADDRESS, MAX30205_TOS), BIN);

}

// Wire.h read and write protocols
void MAX30205::I2CwriteByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
	Wire.beginTransmission(address);  // Initialize the Tx buffer
	Wire.write(subAddress);           // Put slave register address in Tx buffer
	Wire.write(data);                 // Put data in Tx buffer
	Wire.endTransmission();           // Send the Tx buffer
}

//by zl
void MAX30205::I2CwriteBytes(uint8_t address, uint8_t subAddress, uint16_t data)
{   
    uint8_t hi,lo =0 ;
	hi = (data&0xFF00)>>8;
	lo = (data&0x00FF);
	Wire.beginTransmission(address);  // Initialize the Tx buffer
	Wire.write(subAddress);           // Put slave register address in Tx buffer
	Wire.write(hi);                 // Put data in Tx buffer
	Wire.write(lo);                 // Put data in Tx buffer
	Wire.endTransmission();           // Send the Tx buffer
}
uint8_t MAX30205::I2CreadByte(uint8_t address, uint8_t subAddress)
{
	uint8_t data; // `data` will store the register data
	Wire.beginTransmission(address);
	Wire.write(subAddress);
	Wire.endTransmission(false);
	Wire.requestFrom(address, (uint8_t) 1);
	data = Wire.read();
	return data;
}

void MAX30205::I2CreadBytes(uint8_t address, uint8_t subAddress, uint8_t * dest, uint8_t count)
{
	Wire.beginTransmission(address);   // Initialize the Tx buffer
	// Next send the register to be read. OR with 0x80 to indicate multi-read.
	Wire.write(subAddress);
	Wire.endTransmission(false);
	uint8_t i = 0;
	Wire.requestFrom(address, count);  // Read bytes from slave register address
	while (Wire.available())
	{
		dest[i++] = Wire.read();
	}
}

uint16_t MAX30205::I2CreadWord(uint8_t address, uint8_t subAddress)
{
	uint16_t data ;
	Wire.beginTransmission(address);   // Initialize the Tx buffer
	// Next send the register to be read. OR with 0x80 to indicate multi-read.
	Wire.write(subAddress);
	Wire.endTransmission(false);
	uint8_t i = 0;
	Wire.requestFrom(address, 2);  // Read bytes from slave register address
	while (Wire.available())
	{
		data = Wire.read();
		data = data <<8;
		data += Wire.read();
	}
	return data;
}
