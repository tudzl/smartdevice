/*  Smart device Main PCB/ m5stack iic debug scan code for ESP32 wrover and 6 sensors
    this code is working as APP launcher compatibale
    Version 0.3 config Wifi ssid through MQTT 2019.11.11

    need to improve BME680 T reading accuracy!
    m5stack fire arduino device test app for ENV unit and BME680 module +HP206C  module
    Author ling zhou, 16.8.2019

    ###buttonA_longPressed to swirch MQTT active
    ###device ID need change for different boaards!!!
    note: ONboard sensors: BME280+ICM-20948+MCP9808T-E/MS+BH1750+MAX44009EDT+analog Mic ICS-40181

    note: need add library Adafruit_BME680 from library manage

    Sketch uses 866978 bytes (27%) of program storage space. Maximum is 3145728 bytes.
    Global variables use 42080 bytes (12%) of dynamic memory, leaving 285600 bytes for local variables. Maximum is 327680 bytes.

*/



#include <M5Stack.h>
//#include "utility/Power.h"
//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/


#include <Wire.h>



void setup()
{

  M5.begin();

  Wire.begin();
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  Serial.begin(115200);
  Serial.println("Smart device | i2C SCANNER ");
  Serial.println("=========================");
  Serial.println("Starting ....");
  Serial.println("");
  M5.lcd.setBrightness(30);
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2); //size 2 to 8
  M5.Lcd.println("Smart device i2C SCANNER ");
  M5.Lcd.setTextColor(WHITE, BLACK);
}

void loop()
{
  byte error, address;
  int devices;

  Serial.println("Scanning...");
  M5.Lcd.println("Scanning...");

  devices = 0;
  for (address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address | 0x");
      M5.Lcd.print("I2C device found at address | 0x");
      if (address < 16) {
        Serial.print("0");
        M5.Lcd.print("0");
      }
      Serial.print(address, HEX);
      M5.Lcd.print(address, HEX);
      Serial.println("  !");
      M5.Lcd.println("  !");

      devices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknow error at address | 0x");
      M5.Lcd.print("Unknow error at address | 0x");
      if (address < 16) {
        Serial.print("0");
        M5.Lcd.print("0");
      }
      Serial.println(address, HEX);
      M5.Lcd.println(address, HEX);
    }
  }
  if (devices == 0) {
    Serial.println("No I2C devices found\n");
    M5.Lcd.println("No I2C devices found\n");
  }
  else {
    Serial.printf("Total device found:%d\n", devices);
    M5.Lcd.printf("Total device found:%d\n", devices);
  }

  delay(5000);
}
