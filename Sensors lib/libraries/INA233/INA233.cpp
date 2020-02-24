/**************************************************************************/
/*!
  @file     INA233.cpp
  version 1.1 18.012020
  Author   improved by Zell  tudzl@hotmail.de
  
  INA233 spec:
  *Reg write:Register bytes are sent least-significant byte first, followed by the most significant byte
  *Read reg Word: LSB first
  #Vbus input: 830K Ohms, Full-scale range = 40.96 V (7FFFh) and LSB = 1.25 mV.
  #V shunt Full-scale range = +-81.92 mV (7FFFh) and LSB:2.5 μV.
  
  #INA233 offers programmable conversion times as fast as 140 μs to as long
as 8.244 ms@ "MFR_ADC_CONFIG" @D0h
  
  @author   rava (infinityPV ApS)
	@license  BSD (see license.txt)

	This is a library for all the infinityPV boards using the ADC/I2C converter
  INA233:
  - 1. LiBAT5V2A Energy Management Shield
  ----> https://infinitypv.com/products/electronics/libat5v2a

	InfinityPV makes printed organic solar cells. We invests part of our
  time and resources providing open source examples (hardware and code) to
  make easier the use of our solar cells in your projects.
	Printed organic solar cells are flexible, rollable and super light unlike
  other solar cell technologies, please visit our website to find more !

  (*)This libray is protected by BSD license, Copyright (c) 2018, infinityPV ApS
  All rights reserved
	@section  HISTORY

    v1.0  - First release Mar 2018
*/
/**************************************************************************/
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

#include "INA233.h"


/**************************************************************************/
/*!
    @brief  Writes a byte over I2C, no data are sent, only the
    PMBus comand (reg).
*/
/**************************************************************************/
void INA233::wireSendCmd(uint8_t reg)
{
  Wire.beginTransmission(ina233_i2caddr);
  #if ARDUINO >= 100
    Wire.write(reg);                       // PMBus command
  #else
    Wire.send(reg);                        // PMBus command
  #endif
  Wire.endTransmission();
}
/**************************************************************************/
/*!
    @brief  Writes a byte (value) to the specified register
    by the PMBus comand (reg) over I2C
*/
/**************************************************************************/
void INA233::wireWriteByte (uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(ina233_i2caddr);
  #if ARDUINO >= 100
    Wire.write(reg);                       // PMBus command
    Wire.write(value);                     // byte to write
  #else
    Wire.send(reg);                        // PMBus command
    Wire.send(value);                      // byte to write
  #endif
  Wire.endTransmission();
}
/**************************************************************************/
/*!
    @brief  Writes a word (value) to the specified register
    by the PMBus comand (reg) over I2C
*/
/**************************************************************************/
void INA233::wireWriteWord (uint8_t reg, uint16_t value)
{
  Wire.beginTransmission(ina233_i2caddr);
  #if ARDUINO >= 100
    Wire.write(reg);                       // PMBus command
    Wire.write(value & 0xFF);              // Lower 8-bits
    Wire.write((value >> 8) & 0xFF);       // Upper 8-bits
  #else
    Wire.send(reg);                        // PMBus command
    Wire.send(value & 0xFF);               // Lower 8-bits, LSB
    Wire.send(value >> 8);                 // Upper 8-bits
  #endif
  Wire.endTransmission();
}

/**************************************************************************/
/*!  written by zell
    @brief  Writes a word/uint16_t (data) to the specified register
    by the PMBus comand (reg_address) over I2C
*/
/**************************************************************************/
void INA233::RegWriteWord (uint8_t reg_address, uint16_t data)
{
    uint8_t hi,lo =0 ;
	hi = (data&0xFF00)>>8;
	lo = (data&0x00FF);
	Wire.beginTransmission(ina233_i2caddr);  // Initialize the Tx buffer
	Wire.write(reg_address);           // Put slave register address in Tx buffer
	Wire.write(lo);                 // Put data in Tx buffer
	Wire.write(hi);                 // Put data in Tx buffer
	Wire.endTransmission();           // Send the Tx buffer
}

/**************************************************************************/
/*!  written by zell
    @brief  read a word/uint16_t (data) to the specified register
    by the PMBus comand (reg_address) over I2C
*/
/**************************************************************************/
uint16_t INA233::RegReadWord(uint8_t reg_address)
{
	uint16_t data ;
	Wire.beginTransmission(ina233_i2caddr);   // Initialize the Tx buffer
	// Next send the register to be read. OR with 0x80 to indicate multi-read.
	Wire.write(reg_address);
	Wire.endTransmission(false);
	uint8_t i = 0;
	Wire.requestFrom(ina233_i2caddr, 2,true);  // Read bytes from slave register address, lsb first
	while (Wire.available())
	{
		data = Wire.read();
		//data = data <<8;
		data += Wire.read()<<8; // MSB second
	}
	return data;
}


/**************************************************************************/
/*!
    @brief  Reads a Block of 6 bytes from the INA233 over I2C
    When reading a block in PMBUS the first byte from the slave is the
    block size (6 in this case), so the request must be for block_size+1
*/
/**************************************************************************/
void INA233::wireReadBlock(uint8_t reg, uint8_t value[6])
{
  uint8_t i;
  uint8_t count = 7;
  uint8_t block_size;
  //void MAX30205::I2CreadBytes(uint8_t address, uint8_t subAddress, uint8_t * dest, uint8_t count)
  	Wire.beginTransmission(ina233_i2caddr);   // Initialize the Tx buffer
	// Next send the register to be read. OR with 0x80 to indicate multi-read.
	Wire.write(reg);
	Wire.endTransmission(false);

	Wire.requestFrom(ina233_i2caddr, count,true);  // Read bytes from slave register address
  for (i=0;i<block_size;i++)
  {
    value[i]=Wire.read();
  }
  
  
  
  /* ori code
  Wire.requestFrom(ina233_i2caddr,(uint8_t)7,reg,(uint8_t)1,(uint8_t)true); // compile error
  //    uint8_t requestFrom(uint16_t address, uint8_t size, bool sendStop);
  block_size=Wire.read();
  for (i=0;i<block_size;i++)
  {
    value[i]=Wire.read();
  }
  */
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit value from the INA233 over I2C
*/
/**************************************************************************/
void INA233::wireReadWord(uint8_t reg, uint16_t *value)
{
	
		uint16_t data ;
	Wire.beginTransmission(ina233_i2caddr);   // Initialize the Tx buffer
	// Next send the register to be read. OR with 0x80 to indicate multi-read.
	Wire.write(reg);
	Wire.endTransmission(false);
	uint8_t i = 0;
	Wire.requestFrom(ina233_i2caddr, 2);  // Read bytes from slave register address
	while (Wire.available())
	{
		data = Wire.read();
		//data = data <<8;
		data += Wire.read()<<8;
	}
	*value = data;
	//return data;
	
	
	
	
	/*
  Wire.requestFrom(ina233_i2caddr,(uint8_t)2,reg,(uint8_t)1,(uint8_t)true);
  *value = Wire.read();
  *value=((Wire.read() << 8) | *value);
  */
}
/**************************************************************************/
/*!
    @brief  Reads a 8 bit value from the INA233 over I2C
*/
/**************************************************************************/
void INA233::wireReadByte(uint8_t reg, uint8_t *value)
{
	
		uint8_t data; // `data` will store the register data
	Wire.beginTransmission(ina233_i2caddr);
	Wire.write(reg);
	Wire.endTransmission(false);
	Wire.requestFrom(ina233_i2caddr, (uint8_t) 1);
	data = Wire.read();
	 *value = data;
	//return data;
	
	/*
  Wire.requestFrom(ina233_i2caddr,(uint8_t)1,reg,(uint8_t)1,(uint8_t)true);
  *value = Wire.read();
  */
}




//-------------------------MFR_ADC_CONFIG  ----------------------

uint16_t INA233::ADC_CONFIG (uint16_t data){
    //uint8_t date_lo = data&&0xFF;
	//uint8_t date_hi = (data>>8)&&0xFF;
	
    //Wire.beginTransmission(ina233_addr); //included in wireWriteWord
    RegWriteWord(MFR_ADC_CONFIG, data);
    return RegReadWord(MFR_ADC_CONFIG);
	
	
}
//-------------------------Calibration CONFIG  ----------------------
uint16_t INA233::RegSetCalibration(uint16_t data){
    //uint8_t date_lo = data&&0xFF;
	//uint8_t date_hi = (data>>8)&&0xFF;
	
    //Wire.beginTransmission(ina233_addr); //included in wireWriteWord
    RegWriteWord(MFR_CALIBRATION, data);
    return RegReadWord(MFR_CALIBRATION);
	
	
}
	
//-------------TI_MFR_ID	
uint16_t INA233::Read_ID(void){
    //5449h
    return RegReadWord(TI_MFR_ID);
	
}
	
/**************************************************************************/
/*!
    @brief  Set INA233 Calibration register for measuring based on the user's
    inputs r_shunt and i_max.
    -inputs: value of the shunt resistor and maximum current (in ohms and A)
    -inputs as outputs: measuring accuracy for current (uA) and power (mW) and
    ERROR state for possible errors during Calibration.
    -outputs: the CAL value to be written in MFR_CALIBRATION

    */
/**************************************************************************/
uint16_t INA233::setCalibration(float r_shunt,float i_max,float *Current_LSB,float *Power_LSB, int16_t *mc,int8_t *Rc, int16_t *mp, int8_t *Rp,  uint8_t *ERROR)
{
  float C_LSB=0;
  float P_LSB=0;
  float CAL=0;
  float m_c_F=0;  // mc=m_c_F=1/C_LSB
  float m_p_F=0;  //mp=m_p_F=1/P_LSB
  int32_t aux=0;
  bool round_done=false;
  int8_t local_R_c=0; //?
  int8_t local_R_p=0;  //?
  uint8_t local_ERROR=0;

  C_LSB=i_max/pow(2,15);
  P_LSB=25*C_LSB;
  *Current_LSB=C_LSB*1000000;
  *Power_LSB=P_LSB*1000;
  CAL=0.00512/(r_shunt*C_LSB);

  //Check CAL is in the uint16 range
  if (CAL>0xFFFF)
    {
      local_ERROR=1;
    }
  else
    {
	
    wireWriteWord(MFR_CALIBRATION, (uint16_t)CAL);
    }
  m_c_F=1/C_LSB;
  m_p_F=1/P_LSB;

  //Calculate m and R for maximum accuracy in current measurement
  aux=(int32_t)m_c_F;
  while ((aux>32768)||(aux<-32768))
    {
      m_c_F=m_c_F/10;
      local_R_c++;
      aux=(int32_t)m_c_F;
    }
  while (round_done==false)
    {
      aux=(int32_t)m_c_F;
      if (aux==m_c_F)
      {
        round_done=true;
      }
      else
      {
         aux=(int32_t)(m_c_F*10);             //shift decimal to the right
         if ((aux>32768)||(aux<-32768))       //m_c is out of int16 (-32768 to 32768)
         {
          round_done=true;
         }
         else
         {
          m_c_F=m_c_F*10;
          local_R_c--;
         }
      }
    }
  round_done=false;
  //Calculate m and R for maximum accuracy in power measurement
  aux=(int32_t)m_p_F;
  while ((aux>32768)||(aux<-32768))
    {
      m_p_F=m_p_F/10;
      local_R_p++;
      aux=(int32_t)m_p_F;
    }
  while (round_done==false)
    {
      aux=(int32_t)m_p_F;
      if (aux==m_p_F)
      {
        round_done=true;
      }
      else
      {
         aux=(int32_t)(m_p_F*10);          //shift decimal to the right
         if ((aux>32768)||(aux<-32768))       //m_p is out of int16 (-32768 to 32768)
         {
          round_done=true;
         }
         else
         {
          m_p_F=m_p_F*10;
          local_R_p--;
         }
      }
    }
  *mp=m_p_F;
  *mc=m_c_F;
  *Rc=local_R_c;
  *Rp=local_R_p;
  *ERROR=local_ERROR;

  m_c=int16_t(m_c_F);
  m_p=int16_t(m_p_F);
  R_c=local_R_c;
  R_p=local_R_p;

  return(uint16_t)CAL;
}
/**************************************************************************/
/*!
    @brief  Instantiates a new INA233 class
*/
/**************************************************************************/
INA233::INA233(uint8_t addr) {
  ina233_i2caddr = addr;
  m_c=0;
  R_c=0;
  m_p=0;
  R_p=0;
}
/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
void INA233::begin() {
  Wire.begin();
}



//writen by zell
bool INA233::begin2(uint8_t addr) 
{
	Wire.beginTransmission(addr);

	int error = Wire.endTransmission();   //I2C_ERROR_OK=0,
	if (0 == error)
		return true;
    else 
		return false;
}


/**************************************************************************/
/*!
    @brief  Gets the raw bus voltage (2-byte, two's complement integer
    received from the device)
*/
/**************************************************************************/
int16_t INA233::getBusVoltage_raw() {
  uint16_t value;
  wireReadWord(READ_VIN, &value);

  return (int16_t)value;
}
/**************************************************************************/
/*!
    @brief  Gets the raw shunt voltage (2-byte, two's complement integer
    received from the device)
*/
/**************************************************************************/
int16_t INA233::getShuntVoltage_raw() {
  uint16_t value;
  wireReadWord(MFR_READ_VSHUNT, &value); //0xD1
  return (int16_t)value;
}
/**************************************************************************/
/*!
    @brief  Gets the raw current value (2-byte, two's complement integer
    received from the device)
*/
/**************************************************************************/
int16_t INA233::getCurrent_raw() {
  uint16_t value;
  wireReadWord(READ_IIN, &value);
  return (int16_t)value;
}
/**************************************************************************/
/*!
    @brief  Gets the raw power value (2-byte, two's complement integer
    received from the device)
*/
/**************************************************************************/
int16_t INA233::getPower_raw() {
  uint16_t value;
  wireReadWord(READ_PIN, &value);
  return (int16_t)value;
}
/**************************************************************************/
/*!
    @brief  Gets the raw energy info from READ_EIN register power accumulator
    (2-byte), power accumulator roll over (1byte) and sample count (3bytes)

*/
/**************************************************************************/
void INA233::getEnergy_raw(uint16_t* accumulator, uint8_t* roll_over, uint32_t* sample_count) {
  uint8_t value[6];
  //uint8_t test[6] = { 0x00, 0x11,0x22,0x33,0x44,0x55};
  uint32_t aux;
  wireReadBlock(READ_EIN, value);
  *accumulator=(value[1] << 8) | value[0];
  *roll_over=value[2];
  *sample_count=uint32_t(value[5])<< 16;
  *sample_count=((uint32_t(value[4])<< 8)| *sample_count);
  *sample_count=(uint32_t(value[3])| *sample_count);

}
/**************************************************************************/
/*!
    @brief  Gets the averaged power from last reading of READ_EIN in mW
*/
/**************************************************************************/
float INA233::getAv_Power_mW() {
  uint16_t accumulator=0;
  uint8_t roll_over=0;
  uint32_t sample_count=0;
  uint32_t accumulator_24=0;
  uint32_t raw_av_power=0;
  float av_power=0;
  getEnergy_raw(&accumulator,&roll_over, &sample_count);
  accumulator_24=uint32_t(roll_over)*65536+uint32_t(accumulator);
  raw_av_power=accumulator_24/sample_count;

  av_power=(raw_av_power*pow(10,-R_p)-b_p)/m_p;
  return av_power * 1000;
}
/**************************************************************************/
/*!
    @brief  Gets the shunt voltage in mV
*/
/**************************************************************************/
float INA233::getShuntVoltage_mV() {
  //uint16_t value=getShuntVoltage_raw();  // uint16?
  int16_t value=getShuntVoltage_raw();  // uint16?
  float vshunt;
  vshunt=(value*pow(10,-R_vs)-b_vs)/m_vs;
  return vshunt * 1000;
}

/**************************************************************************/
/*!
    @brief  Gets the shunt voltage in volts
*/
/**************************************************************************/
float INA233::getBusVoltage_V() {
  uint16_t value=getBusVoltage_raw();
  float vbus;
  vbus =(value*pow(10,-R_vb)-b_vb)/m_vb;
  return vbus;
}

/**************************************************************************/
/*!
    @brief  Gets the current value in mA, taking into account the
            config settings and current LSB
*/
/**************************************************************************/
float INA233::getCurrent_mA() {
  //uint16_t value=getCurrent_raw(); //bugs
  int16_t value=getCurrent_raw();
  float current;
  current =(value*pow(10,-R_c)-b_c)/m_c;
  return current*1000;
}


/**************************************************************************/
/*!
    @brief  Calculate the current value in mA, taking into account the
            R_shunt and current LSB
*/
/**************************************************************************/
double INA233::CalCurrent_mA(double R_shunt) {
  //uint16_t value=getCurrent_raw(); //bugs
  int16_t Raw_value=getCurrent_raw();
  double current;
  double uV_LSB = 2.5 ;
  //R_shunt ohms
  current = Raw_value * 2.5 / 1000.0 ; // in mV
  current = current/R_shunt; //in mA
  return current;
}

/**************************************************************************/
/*!
    @brief  Gets the power value in mW, taking into account the
            config settings and power LSB
*/
/**************************************************************************/
float INA233::getPower_mW() {
  uint16_t value=getPower_raw();
  float power;
  power =(value*pow(10,-R_p)-b_p)/m_p;
  return power*1000;
}

