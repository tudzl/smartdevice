/*  M5stack fire rainbow pixels ws2812B demo app
 *   modified by zell 
 *   V 1.1  17.03.2020
    Please install FastLED library first.
    In arduino library manage search FastLED
 */
#include <M5Stack.h>
#include "FastLED.h"

//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
POWER m5_power;

#define Neopixel_PIN    26 //fire GROVE  port B
#define NUM_LEDS    64

uint8_t active_leds = 0;
CRGB leds[NUM_LEDS];
uint8_t gHue = 0;
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

unsigned long run_cnt = 0;

//increase num of leds 
void buttonC_wasPressed(void) {
  //M5.Speaker.beep();  // too laud

  Serial.println("BtnC was Pressed\r\n");
  active_leds++;
   if ( active_leds> NUM_LEDS)
    active_leds =NUM_LEDS ;
  Serial.printf("active_leds: %d\r\n", active_leds);


}

//decrease num of leds 
void buttonA_wasPressed(void) {
  //M5.Speaker.beep();  // too laud

  Serial.println("BtnA was Pressed\r\n");
  active_leds--;
   if ( active_leds< 0)
    active_leds = 0 ;
  Serial.printf("active_leds: %d\r\n", active_leds);


}



void setup() {
  M5.begin();
  M5.Power.begin();

  //for app flash back
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }



  M5.Lcd.setBrightness(50); 
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(YELLOW); M5.Lcd.setTextSize(2); M5.Lcd.setCursor(40, 0);
  M5.Lcd.println(" FastLED Neopixel demo");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 25);
  M5.Lcd.setTextColor(RED);
  M5.Lcd.println("Display rainbow effect");
  M5.Lcd.setTextColor(GREEN);
  active_leds =NUM_LEDS;
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.printf("LED Num: %d",active_leds );
   
  
  // Neopixel initialization, 2811,2812,etc....
  FastLED.addLeds<WS2812B,Neopixel_PIN,GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(10);
  xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, NULL, 1);
}

void loop()
{
  run_cnt++;
 
  M5.update(); // This function reads The State of Button A and B and C.

  //change num
  if (M5.BtnC.wasPressed() ) {
    buttonC_wasPressed();

    //delay(100);
  }
  //change gain
  if (M5.BtnA.wasPressed() ) {
    buttonA_wasPressed();

    //delay(100);
  }


  if ( run_cnt % 10 ==0){

  active_leds =NUM_LEDS;
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.printf("LED Num: %d",active_leds );

  int Akku_level = m5_power.getBatteryLevel();
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.setTextSize(2); //size 2 to 8
  M5.Lcd.setTextColor(ORANGE, BLACK);
  M5.Lcd.printf("Akku: %3d%%   Run:%d", Akku_level, run_cnt);
  }
  
}

void FastLEDshowESP32()
{
    if (userTaskHandle == 0) {
        userTaskHandle = xTaskGetCurrentTaskHandle();
        xTaskNotifyGive(FastLEDshowTaskHandle);
        const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
        ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
        userTaskHandle = 0;
    }
}

void FastLEDshowTask(void *pvParameters)
{
    for(;;) {
        fill_rainbow(leds, NUM_LEDS, gHue, 7);// rainbow effect
        FastLED.show();// must be executed for neopixel becoming effective
        EVERY_N_MILLISECONDS( 20 ) { gHue++; }
    }
}

/*
 /// fill_rainbow - fill a range of LEDs with a rainbow of colors, at
///                full saturation and full value (brightness)
void fill_rainbow( struct CRGB * pFirstLED, int numToFill,
                   uint8_t initialhue,
                   uint8_t deltahue = 5);

*/
