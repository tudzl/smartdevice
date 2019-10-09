/*
 * Demo name   : HP20x_dev demo 
 * Usage       : I2C PRECISION BAROMETER AND ALTIMETER [HP206C hopeRF] 
 * Author      : Oliver Wang from Seeed Studio
 * Version     : V0.1
 * Change log  : Add kalman filter 2014/04/04
*/
#include <M5Stack.h>
//#include "utility/Power.h"
#include <HP20x_dev.h>
#include "Arduino.h"
#include "Wire.h" 
#include <KalmanFilter.h>
unsigned char ret = 0;

/* Instance */
KalmanFilter t_filter;    //temperature filter
KalmanFilter p_filter;    //pressure filter
KalmanFilter a_filter;    //altitude filter


void setup()
{  

  M5.begin();
  //m5_power.begin();
  Serial.begin(9600);        // start serial for output
  
  Serial.println("****HP20x_dev demo by seeed studio****\n");
  Serial.println("Calculation formula: H = [8.5(101325-P)]/100 \n");
  /* Power up,delay 150ms,until voltage is stable */
  delay(150);
  /* Reset HP20x_dev */
  HP20x.begin();
  delay(100);
  M5.Lcd.setBrightness(50);  //define BLK_PWM_CHANNEL 7  PWM
}
 

void loop()
{
	Serial.println("------------------\n");
	long Temper = HP20x.ReadTemperature();
	Serial.println("Temper:");
	float t = Temper/100.0;
	Serial.print(t);	  
	Serial.println("C.\n");
	Serial.println("Filter:");
	Serial.print(t_filter.Filter(t));
	Serial.println("C.\n");

	long Pressure = HP20x.ReadPressure();
	Serial.println("Pressure:");
	t = Pressure/100.0;
	Serial.print(t);
	Serial.println("hPa.\n");
	Serial.println("Filter:");
	Serial.print(p_filter.Filter(t));
	Serial.println("hPa\n");
	
	long Altitude = HP20x.ReadAltitude();
	Serial.println("Altitude:");
	t = Altitude/100.0;
	Serial.print(t);
	Serial.println("m.\n");
	Serial.println("Filter:");
	Serial.print(a_filter.Filter(t));
	Serial.println("m.\n");
	Serial.println("------------------\n");
	
	delay(1000);
}
 
