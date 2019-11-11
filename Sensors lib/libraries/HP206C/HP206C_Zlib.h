/*
 * File name  : HP206C_Zlib.h
 * Description: Driver for I2C PRECISION BAROMETER AND ALTIMETER [HP206C]
 * Author     : Zell(Ling Zhou)  2019.10 fix bugs!
 * Author     : Oliver Wang from Seeed studio
 * Version    : V0.1
 * Create Time: 2014/04
 * Change Log :
*/
#ifndef _HP20X_DEV_H
#define _HP20X_DEV_H
/****************************************************************************/
/***        Including Files                                               ***/
/****************************************************************************/
#include <Wire.h>
#include <Arduino.h>
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
typedef unsigned int    uint;
typedef unsigned char   uchar;
typedef unsigned long   ulong;


#define HP20X_I2C_DEV_ID       (0xEC)>>1    //CSB PIN is VDD level(address is 0x76)
#define HP20X_I2C_DEV_ID2      (0XEE)>>1    //CSB PIN is GND level(address is 0x77)
#define HP20X_SOFT_RST         0x06
//ADC_CVT (010, 3‐bit DSR, 2‐bit CHNL)
#define HP20X_WR_CONVERT_CMD   0x40    //0b010+DSR+CHNL
#define HP20X_CONVERT_OSR4096  0<<2
#define HP20X_CONVERT_OSR2048  1<<2
#define HP20X_CONVERT_OSR1024  2<<2
#define HP20X_CONVERT_OSR512   3<<2
#define HP20X_CONVERT_OSR256   4<<2
#define HP20X_CONVERT_OSR128   5<<2

#define HP20X_READ_P           0x30   //read_p command
#define HP20X_READ_A           0x31   //read_a command
#define HP20X_READ_T           0x32   //read_t command
#define HP20X_READ_PT          0x10   //read_pt command
#define HP20X_READ_AT          0x11   //read_at command  temp.+ ALTIMETER
#define HP20X_READ_CAL		   0X28	  //RE-CAL ANALOG

#define HP20X_WR_REG_MODE      0xC0
#define HP20X_RD_REG_MODE      0x80

#define ERR_WR_DEVID_NACK       0x01    
#define ERR_RD_DEVID_NACK       0x02    
#define ERR_WR_REGADD_NACK      0x04   
#define ERR_WR_REGCMD_NACK      0x08   
#define ERR_WR_DATA_NACK        0x10     
#define ERR_RD_DATA_MISMATCH    0x20 

#define I2C_DID_WR_MASK         0xFE
#define I2C_DID_RD_MASK         0x01

#define T_WIN_EN                0X01
#define PA_WIN_EN               0X02
#define T_TRAV_EN               0X04
#define PA_TRAV_EN              0X08
#define PA_RDY_EN               0X20
#define T_RDY_EN                0X10

#define T_WIN_CFG               0X01
#define PA_WIN_CFG              0X02
#define PA_MODE_P               0X00
#define PA_MODE_A               0X40

#define T_TRAV_CFG              0X04

#define OK_HP20X_DEV            0X80		//HP20x_dev CMPS_EN
#define REG_PARA                0X0F    /*This register has only one valid bit of CMPS_EN. 
																					The user can use this bit to determine whether to 
																					enable the data compensation during the conversion 
																					process (0: disable, 1: enable). If it is enabled, 
																					the 24‐bit or 48‐bit data read out by the commands 
																					are fully compensated. If it is disabled, the data 
																					read out are the raw data output.*/ 

/****************************************************************************/
/***        Class Definitions                                             ***/
/****************************************************************************/
//class HP20x_dev : public TwoWire
class HP20x_dev 
{
/* Public variables and functions */
public:
    uchar OSR_CFG;
	uint  OSR_ConvertTime;
	/* Constructor */
	HP20x_dev();	
	void begin();
	bool begin2(uint8_t addr);
	/* Read sensor data */
	long ReadTemperature(void);  // fixed bugs by zell 2019.10.10
	ulong ReadPressure(void);
	ulong ReadAltitude(void);
	long ReadAltitude2(void);  // fixed bugs by zell  2019.10.09
	/* Enable or disable compensation */	
	void HP20X_EnableCompensate(void);
	void HP20X_DisableCompensate(void);
	
    /* Private variables and functions */
private:
    /* Write a command to HP20x */
	void HP20X_IIC_WriteCmd(uchar uCmd);
	/* Read register value */
	uchar HP20X_IIC_ReadReg(uchar bReg);	
	void HP20X_IIC_WriteReg(uchar bReg,uchar bData);	 	
	ulong HP20X_IIC_ReadData(void);
	ulong HP20X_IIC_ReadData3byte(void);


};
extern HP20x_dev HP20x;
#endif