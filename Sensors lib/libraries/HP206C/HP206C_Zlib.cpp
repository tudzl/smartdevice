/*
 * File name  : HP206C_Zlib.cpp
 * Description: Driver for I2C PRECISION BAROMETER AND ALTIMETER [HP206C]
 * Author     : Zell(Ling Zhou)  2019.10 fix bugs!
 * Author     : Oliver Wang from Seeed studio
 * Version    : V0.1
 * Create Time: 2014/04
 * Change Log : 
*/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "HP206C_Zlib.h"
#include <Wire.h>
#include <Arduino.h>
//#include "ZFilter.h"
/****************************************************************************/
/***       Local Variable                                                 ***/
/****************************************************************************/
 HP20x_dev HP20x;


/****************************************************************************/
/***       Class member Functions                                         ***/
/****************************************************************************/
/*
 **@ Function name: HP20x_dev
 **@ Description: Constructor
 **@ Input: none
 **@ OutPut: none
 **@ Retval: none
*/
HP20x_dev::HP20x_dev()
{
    //OSR_CFG = HP20X_CONVERT_OSR1024; //1280 TO 4096
    //OSR_ConvertTime = 25; //4.1 to 131.1 ms
	
	OSR_CFG = HP20X_CONVERT_OSR4096; //1280 TO 4096
    OSR_ConvertTime = 125; //4.1 to 131.1 ms

}

/*
 **@ Function name: begin2
 **@ Description: Initialize HP20x_dev  
 **@ Input: none
 **@ OutPut: none
 **@ Retval: none
*/

bool HP20x_dev::begin2(uint8_t addr)
{
	 bool rslt = false ; 
	 unsigned char error =1 ; 
   Wire.begin();
  
   Wire.beginTransmission(addr);
   error = Wire.endTransmission();
   if (error == 0)
    {
      //Serial.print("I2C device found at address | 0x");
     rslt = true;
 
  
    }
    else if (error==4) 
    {
      //Serial.print("Unknow error at address | 0x");
     return false;
    } 
    else return false;
  
  /* Reset HP20x_dev */
  HP20x.HP20X_IIC_WriteCmd(HP20X_SOFT_RST);
  HP20x.HP20X_EnableCompensate();
  return rslt;
}
/*
 **@ Function name: begin
 **@ Description: Initialize HP20x_dev
 **@ Input: none
 **@ OutPut: none
 **@ Retval: none
*/
void HP20x_dev::begin()
{
  Wire.begin();
  
  
  
  
  /* Reset HP20x_dev */
  HP20x.HP20X_IIC_WriteCmd(HP20X_SOFT_RST);
  HP20x.HP20X_EnableCompensate();

}

/*
 **@ Function name: ReadTemperature
 **@ Description: Read Temperature from HP20x_dev
 **@ Input:
 **@ OutPut: long
 **@ Retval:
*/
long HP20x_dev::ReadTemperature(void)
{   
	HP20X_IIC_WriteCmd(HP20X_WR_CONVERT_CMD|OSR_CFG);	//ADC convert
	delay(OSR_ConvertTime);			                    //difference OSR_CFG will be difference OSR_ConvertTime
	HP20X_IIC_WriteCmd(HP20X_READ_T);      
	long Temperature = HP20X_IIC_ReadData();
	return Temperature;		
}

/*
 **@ Function name: ReadPressure
 **@ Description: Read Pressure value
 **@ Input:
 **@ OutPut: 
 **@ Retval: value
*/
 
ulong HP20x_dev::ReadPressure(void)
{
    HP20X_IIC_WriteCmd(HP20X_WR_CONVERT_CMD|OSR_CFG);
    delay(OSR_ConvertTime);
    HP20X_IIC_WriteCmd(HP20X_READ_P);
    ulong Pressure = HP20X_IIC_ReadData();             
    return Pressure;
} 

/*  BUGs!
 **@ Function name: ReadAltitude
 **@ Description: Read Pressure value, return errors when negative alt.
 **@ Input:
 **@ OutPut: 
 **@ Retval: value
*/
ulong HP20x_dev::ReadAltitude(void)
{
    HP20X_IIC_WriteCmd(HP20X_READ_A);
    ulong Altitude = HP20X_IIC_ReadData();   
    return Altitude;		
} 
 
 
 /*  Bug fixed!
 **@ Function name: ReadAltitude
 **@ Description: Read Pressure value
 **@ Input:
 **@ OutPut: 
 **@ Retval: value
*/
long HP20x_dev::ReadAltitude2(void)
{
    HP20X_IIC_WriteCmd(HP20X_READ_A);
    long Altitude = HP20X_IIC_ReadData();   
    return Altitude;		
} 
/*
void ReadPressureAndTemperature(void)
{
        HP20X_IIC_WriteCmd(HP20X_WR_CONVERT_CMD|OSR_CFG);
        Timer_Delayxms(OSR_ConvertTime*2);
        HP20X_IIC_WriteCmd(HP20X_READ_PT);
        
        Temperature=HP20X_IIC_ReadData();
       
        Pressure=HP20X_IIC_ReadData3byte();       
}

void IIC_ReadAltitudeAndTemperature(void)
{

       HP20X_IIC_WriteCmd(HP20X_WR_CONVERT_CMD|OSR_CFG);
       Timer_Delayxms(OSR_ConvertTime*2);
       HP20X_IIC_WriteCmd(HP20X_READ_AT);
        
        Temperature=HP20X_IIC_ReadData();
        IIC_ACK();
        Altitude=HP20X_IIC_ReadData3byte();
        IIC_NoAck();      
        IIC_Stop();  
                   
}*/
/****************************************************************************/
/***       Local Functions                                                ***/
/****************************************************************************/

/*
 **@ Function name: HP20X_IIC_WriteCmd
 **@ Description:
 **@ Input:
 **@ OutPut:
 **@ Retval:
*/
void HP20x_dev::HP20X_IIC_WriteCmd(uchar uCmd)
{		
	/* Port to arduino */
	Wire.beginTransmission(HP20X_I2C_DEV_ID);
	Wire.write(uCmd);
	Wire.endTransmission();
}

/*
 **@ Function name: HP20X_IIC_ReadReg
 **@ Description:
 **@ Input:
 **@ OutPut:
 **@ Retval:  
*/
uchar HP20x_dev::HP20X_IIC_ReadReg(uchar bReg)
{
    /* Port to arduino */
    uchar Temp = 0;
	
	/* Send a register reading command */
    HP20X_IIC_WriteCmd(bReg|HP20X_RD_REG_MODE);	
	 
	Wire.requestFrom(HP20X_I2C_DEV_ID, 1);	 
	// while(Wire.available())
	// {
	//     Temp = Wire.read();
	// }
	Temp = Wire.read();
	 
	return Temp;
} 
/*
 **@ Function name: HP20X_IIC_WriteReg
 **@ Description:
 **@ Input:
 **@ OutPut:
 **@ Retval:
*/
void HP20x_dev::HP20X_IIC_WriteReg(uchar bReg,uchar bData)
{       
	Wire.beginTransmission(HP20X_I2C_DEV_ID);
	Wire.write(bReg|HP20X_WR_REG_MODE);
	Wire.write(bData);
	Wire.endTransmission();
}


/*
 **@ Function name: HP20X_IIC_ReadData
 **@ Description:
 **@ Input:
 **@ OutPut:
 **@ Retval:
*/
ulong HP20x_dev::HP20X_IIC_ReadData(void)
{                        
	/* Port to arduino */	 
	ulong Temp = HP20X_IIC_ReadData3byte(); 
	return Temp;
}

/*
 **@ Function name: HP20X_IIC_ReadData3byte
 **@ Description:
 **@ Input:
 **@ OutPut:
 **@ Retval:
*/
ulong HP20x_dev::HP20X_IIC_ReadData3byte(void)
{	
	ulong TempData = 0;
	ulong tmpArray[3]={0};
	int cnt = 0;
	
	/* Require three bytes from slave */
	Wire.requestFrom(HP20X_I2C_DEV_ID, 3);      

    while(Wire.available())     // slave may send less than requested
    { 
      uchar c = Wire.read();    // receive a byte as character	  	  
	  tmpArray[cnt] = (ulong)c;
	  cnt++;
	}
	
	/* MSB */
	TempData = tmpArray[0]<<16 | tmpArray[1]<<8 | tmpArray[2];

	
    if(TempData&0x800000)
    {
	    TempData|=0xff000000;
	}

 /* 	// 24 bit to 32 bit 
	if(TempData&0x800000)
	{
	  // 1:minus 
	  TempData |= 0x80000000;
	  TempData &= 0xff7fffff;
	}
	else
	{
	  // 0:plus 
	  //do noting
	}  */
	return TempData;
} 

/**
 * @brief Enable Compensation by set CMPS_EN bit on 0x0F PARA register 
*/
void HP20x_dev::HP20X_EnableCompensate(void)
{
	HP20X_IIC_WriteReg(REG_PARA, OK_HP20X_DEV);
}

/**
 * @brief Disable Compensation by clear CMPS_EN bit on 0x0F PARA register 
*/
void HP20x_dev::HP20X_DisableCompensate(void)
{
	HP20X_IIC_WriteReg(REG_PARA, 0);
}

/**************************************END OF FILE**************************************/