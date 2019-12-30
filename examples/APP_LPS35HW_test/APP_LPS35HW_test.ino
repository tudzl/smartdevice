/* this code is working as APP launcher compatible
    LPS35HW air pressure sensor basic test app
    Version 1.1  improve sensor GUI ,  working with m5stack，  30.12.2019
    LPS35HW pressure: 24bits, 0.000244140625 hpa/LSB
    m5stack fire arduino device test app for TCS34725 module
    Author ling zhou, 30.12.2019
    note: need real device test
*/

//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
//#include <M5TreeView.h> // https://github.com/lovyan03/M5Stack_TreeView
//#include <M5OnScreenKeyboard.h> // https://github.com/lovyan03/M5Stack_OnScreenKeyboard/

#include <M5Stack.h>
//#include "utility/Power.h"
#include <Wire.h>
#include "LPS35HW.h"

LPS35HW lps;
bool LPS_sensor_ok = false;
double Altitude =0;
POWER m5_power;
unsigned long run_cnt = 0;

void setup() {
  M5.begin();


  //for app flash back
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  //Serial.begin(9600);
  Serial.println("LPS barometer test");

  
  M5.Lcd.setBrightness(50);  //define BLK_PWM_CHANNEL 7  PWM
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.println("  LPS35HW test V1.1");

  if (!lps.begin()) {
    Serial.println("Could not find a LPS barometer, check wiring!");
    LPS_sensor_ok = false;
    while (1) {}
  }
  else {
    Serial.println("LPS35HW sensor is connectd!");
    LPS_sensor_ok = true;
  }

  lps.setLowPassFilter(LPS35HW::LowPassFilter_ODR9);  // filter last 9 samples, default off
  lps.setOutputRate(LPS35HW::OutputRate_1Hz);  // optional, default is 10Hz
   M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 200);
    M5.Lcd.setTextColor(LIGHTGREY, BLACK);
    M5.Lcd.println("Setting:LowPassFilter_ODR9+OutputRate_1Hz");
  // lps.setLowPower(true);  // optional, default is off
}

void loop() {
  float pressure = lps.readPressure();  // hPa
  float temp_lps = lps.readTemp();  // °C

   Altitude = 8.5 * (1013.25 - pressure); // in meter @ 25 degree
    //Altitude = Altitude + (T_bmp - 22) * 0.2273; // compensation for temperature,

  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.print("hPa\ttemperature: ");
  Serial.print(temp_lps);
  Serial.println("*C\n");
  Serial.printf("Altitude: %.2f m \r\n",Altitude);
  
  

  if (LPS_sensor_ok) {

    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.printf("A.P.:%.3f hPa\r\n", pressure);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.printf("T.:%.2f C\r\n", temp_lps);

    M5.Lcd.printf("Alt.: %.2f m \r\n",Altitude);
    //M5.Lcd.printf("Gain:%dX, T:%d ms \r\n", rgb_sensor.againx, rgb_sensor.atime_ms);
   

  }

  //SYS  GUI
  int Akku_level = m5_power.getBatteryLevel();
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.setTextSize(2); //size 2 to 8
  M5.Lcd.setTextColor(ORANGE, BLACK);
  M5.Lcd.printf("Akku: %3d%%   Run:%d", Akku_level, run_cnt);

  M5.update(); // This function reads The State of Button A and B and C.
  run_cnt++;
  delay(500);
}
