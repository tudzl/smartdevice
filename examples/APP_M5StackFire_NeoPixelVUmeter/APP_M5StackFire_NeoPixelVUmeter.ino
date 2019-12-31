/*
  microphone neopixel VU meter
  modified by ling zhou for lovyan03 app launcher

  Neopixel LEDS flash dependent on the microphone amplitude

   hardwware:  M5StackFire

  please install the Adafruit library first!

  September 2018, ChrisMicro, MIT License

*/

#include <Wire.h>
#include "Adafruit_TCS34725.h"
//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
//#include <M5TreeView.h> // https://github.com/lovyan03/M5Stack_TreeView
//#include <M5OnScreenKeyboard.h> // https://github.com/lovyan03/M5Stack_OnScreenKeyboard/

#include <M5Stack.h>


#include <Adafruit_NeoPixel.h>

//Hardware pins
#define M5STACK_FIRE_NEO_NUM_LEDS  10
#define M5STACK_FIRE_NEO_DATA_PIN  15
#define M5STACKFIRE_MICROPHONE_PIN 34

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);


POWER m5_power;
unsigned long run_cnt = 0;


void setup()
{

  M5.begin();
  M5.Lcd.setBrightness(50);  //define BLK_PWM_CHANNEL 7  PWM
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(PINK, BLACK);
  M5.Lcd.println(" Fire Neopixel VU meter");

  //for app flash back
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

  Serial.begin(115200);
  pixels.begin();
}

#define NUMBEROFSAMPLES 1000
uint16_t micValue[NUMBEROFSAMPLES];

void loop()
{
  uint32_t power = 0;
  uint32_t meanValue = 0;
  for (uint32_t n = 0; n < NUMBEROFSAMPLES; n++)
  {
    int value = analogRead(M5STACKFIRE_MICROPHONE_PIN);
    micValue[n] = value;
    meanValue += value;
    delayMicroseconds(20);
  }
  meanValue /= NUMBEROFSAMPLES;
  for (uint32_t n = 0; n < NUMBEROFSAMPLES; n++)
  {
    power += (micValue[n] - meanValue) * (micValue[n] - meanValue);
  }
  power /= NUMBEROFSAMPLES;

  Serial.println(power);

  int threshold = 1000;
  for (uint8_t n = 0; n < M5STACK_FIRE_NEO_NUM_LEDS; n++)
  {
    pixels.setPixelColor(n, pixels.Color(0, 0, 1));
    if (power > threshold)  pixels.setPixelColor(n, pixels.Color(100, 0, 0));
    threshold *= 5;
  }
  pixels.show();
  delay(10);

  //SYS  GUI
  int Akku_level = m5_power.getBatteryLevel();
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.setTextSize(2); //size 2 to 8
  M5.Lcd.setTextColor(ORANGE, BLACK);
  M5.Lcd.printf("Akku: %3d%%   Run:%d", Akku_level, run_cnt);

  M5.update(); // This function reads The State of Button A and B and C.
  run_cnt++;

}
