/* this code is working as APP launcher compatible
    TCS34725 color sensor basic test app
    Version 1.5  Improve driver, add lux calc, 2020.2.29 
    Version 1.4  improve GUI
    Version 1.3  dual CCT, auto int time switch   31.12.2019
    Version 1.2  improve  GUI , add loop time   31.12.2019
    Version 1.1  improve sensor GUI ,  working with m5stack，  30.12.2019

    m5stack fire arduino device test app for TCS34725 module
    Author ling zhou, 30.12.2019
    note: need real device test
*/
#include <Wire.h>
#include "Adafruit_TCS34725.h"
//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
//#include <M5TreeView.h> // https://github.com/lovyan03/M5Stack_TreeView
//#include <M5OnScreenKeyboard.h> // https://github.com/lovyan03/M5Stack_OnScreenKeyboard/

#include <M5Stack.h>
//#include "utility/Power.h"

/* Example code for the Adafruit TCS34725 breakout library */

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */

/* Initialise with default values (int time = 2.4ms, gain = 1x) */
//  void setIntegrationTime(tcs34725IntegrationTime_t it);
//  void setGain(tcs34725Gain_t gain);
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_1X);


POWER m5_power;
unsigned long run_cnt = 0;

bool TCS_sensor_ok = false;
uint16_t R_pre, G_pre, B_pre, C_pre, colorTemp_pre, lux_pre; // store previous values
uint16_t r, g, b, c, colorTemp, colorTemp_uni, lux, lux_tao;
uint16_t low_light_thre = 5000 ;
uint16_t mid_light_thre = 40000 ;
int lux_range = 1; //def 1 for low light; 2, 3
bool auto_range = true;
uint8_t TCS_gain = 1; //1,4,16,60X


// to store current value to previous vars
void buttonB_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);

  Serial.println("BtnB was Pressed\r\n");
  R_pre = r;
  G_pre = g;
  B_pre = b;
  C_pre = c;
  colorTemp_pre = colorTemp;
  lux_pre = lux;

}

//CCT measure mode, Atime fixed to 702ms
void buttonC_wasPressed(void) {
  //M5.Speaker.beep();  // too laud

  Serial.println("BtnC was Pressed\r\n");
  //fixed mode for CCT measurement
  tcs.setGain(TCS34725_GAIN_1X);
  tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_700MS);
  auto_range = not auto_range;
  //play_startup_music();
  Serial.println("Atime: 700ms, Gain: 1x,fixed mode for CCT measurement\r\n");


}

//gain set
void buttonA_wasPressed(void) {
  //M5.Speaker.beep();  // too laud

  Serial.println("BtnA was Pressed\r\n");
  //fixed mode for CCT measurement

  TCS_gain *= 4;

  if (TCS_gain == 1) {
    tcs.setGain(TCS34725_GAIN_1X);
    Serial.println("TCS34725 set Gain to: 1X\r\n");
  }
  else if (TCS_gain == 4) {
    tcs.setGain(TCS34725_GAIN_4X);
    Serial.println("TCS34725 set Gain to: 4X\r\n");
  }
  else if (TCS_gain == 16) {
    tcs.setGain(TCS34725_GAIN_16X);
    Serial.println("TCS34725 set Gain to: 16X\r\n");
  }
  else if (TCS_gain == 64) {
    tcs.setGain(TCS34725_GAIN_60X);
    Serial.println("TCS34725 set Gain to: 60X\r\n");
    TCS_gain = 1;
  }
  else {
    tcs.setGain(TCS34725_GAIN_1X);
    Serial.println("TCS34725 set Gain to: 1X\r\n");
  }



  //play_startup_music();


}



void setup(void) {
  //Serial.begin(9600);


  M5.begin();
  M5.Lcd.setBrightness(50);  //define BLK_PWM_CHANNEL 7  PWM
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.println(" TCS34725 APP V1.2");

  //for app flash back
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }



  if (tcs.begin()) {
    Serial.println("Found sensor");
    TCS_sensor_ok = true;
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setCursor(0, 200);
    M5.Lcd.println("TCS Sensor OK!");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.println("TCS Sensor N.A.!");
    TCS_sensor_ok = false;
    while (1);
  }

  TCS_gain = 1;

  // Now we're ready to get readings!
}

void loop(void) {

  if ((c < low_light_thre) && (lux_range > 1) ) {
    //tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_700MS); // for low light
    lux_range -= 1 ;
  }
  else if (c < mid_light_thre) {

    // tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_154MS); // for mid light
    lux_range += 0;
  }
  else {
    //tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_24MS); // for higher light
    lux_range += 1;
  }
  lux_range = min(3, lux_range);
  lux_range = max(1, lux_range);
  if (auto_range) {
    switch (lux_range) {
      case 1:
        tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_700MS);
        //tcs.setGain(TCS34725_GAIN_1X);
        break;
      case 2:
        tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_154MS);
        break;
      case 3:
        tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_24MS);
        break;
      default:
        tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_700MS);
        //tcs.setGain(TCS34725_GAIN_1X);
        break;
    }
  }
  long Zeit_anfang = millis();
  tcs.getRawData(&r, &g, &b, &c);
  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  long Zeit_process = millis();
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c); //algorithm described in DN40 from Taos (now AMS).
  colorTemp_uni = tcs.calculateColorTemperature(r, g, b); //general algorithm
  lux = tcs.calculateLux(r, g, b);
  lux_tao = tcs.calculateLux_tao(r, g, b, c);
  /* AMS RGB sensors have no IR channel, so the IR content must be */
  /* calculated indirectly. */
  uint16_t ir = (r + g + b > c) ? (r + g + b - c) / 2 : 0;
  //compensate data rgb
  r = r - ir;
  g = g - ir;
  b = b - ir;


  Serial.print("Color Temp(DN40): "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.printf("Lux_tao: %.1f", lux_tao); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  //ir
  Serial.print("IR: "); Serial.print(ir, DEC); Serial.print(" ");
  Serial.println(" ");
  if (TCS_sensor_ok) {

    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Lux:%d  \r\n", lux);
    M5.Lcd.printf("Lux_t:%d  \r\n", lux_tao);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.printf("CCT(DN40):%dK \r\n", colorTemp);
    M5.Lcd.setTextColor(GREENYELLOW, BLACK);
    M5.Lcd.printf("CCT(ref):%dK \r\n", colorTemp_uni);
    //M5.Lcd.printf("Gain:%dX, T:%d ms \r\n", rgb_sensor.againx, rgb_sensor.atime_ms);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.printf("R:%d ", r);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.printf("G:%d ", g);
    M5.Lcd.setTextColor(BLUE, BLACK);
    M5.Lcd.printf("B:%d \r\n",  b);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Clear L.:%d \r\n",  c);
    M5.Lcd.setTextColor(PINK, BLACK);
    M5.Lcd.printf("IR:%d   \r\n",  ir);
    M5.Lcd.println("");
  }
  //history GUI


  long now = millis();
  //SYS  GUI
  M5.Lcd.setCursor(0, 180);
  M5.Lcd.setTextSize(2); //size 2 to 8
  M5.Lcd.setTextColor(LIGHTGREY, BLACK);
  M5.Lcd.printf("T.S.:%dms, T.LP.:%dms\r\n", Zeit_process - Zeit_anfang, now - Zeit_process );
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setCursor(0, 200);
  M5.Lcd.printf("Sensor gain: %dX", TCS_gain);

  int Akku_level = m5_power.getBatteryLevel();
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.setTextSize(2); //size 2 to 8
  M5.Lcd.setTextColor(ORANGE, BLACK);
  M5.Lcd.printf("Akku: %3d%%   Run:%d", Akku_level, run_cnt);

  M5.update(); // This function reads The State of Button A and B and C.

  //CCT measure mode
  if (M5.BtnC.wasPressed() ) {
    buttonC_wasPressed();

    delay(100);
  }
  //change gain
  if (M5.BtnA.wasPressed() ) {
    buttonA_wasPressed();

    delay(100);
  }


  //store vars
  if (M5.BtnB.wasPressed() ) {
    //Serial.println("BtnB was Pressed\r\n");
    Serial.printf("Lux_pre: %d ;CCT_pre: %d\r\n", lux_pre, colorTemp_pre );
    buttonB_wasPressed();
    Serial.printf("Lux_new: %d ;CCT_new: %d\r\n", lux, colorTemp );
    delay(100);
  }
  run_cnt++;
}
