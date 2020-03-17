/************************************************************************

  M5StackFire Discovery simple Spectrum example

  The microphone signal is sampled and a frequency analysis is performed.

  Real device range test： 100 to  5500K, show 1026K @ 1K
  Please install the arduinoFFT library.
  You will find it in the library manager or you can get it from github:
  https://github.com/kosme/arduinoFFT


  M5StackFire         September 2018, ChrisMicro

************************************************************************/


#include <Wire.h>
//#include "Adafruit_TCS34725.h"
//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
//#include <M5TreeView.h> // https://github.com/lovyan03/M5Stack_TreeView
//#include <M5OnScreenKeyboard.h> // https://github.com/lovyan03/M5Stack_OnScreenKeyboard/

#include "FastLED.h"

#include <M5Stack.h>
#include "arduinoFFT.h"
//#include <WiFi.h>

#include <Adafruit_NeoPixel.h>

//Hardware pins
#define M5STACK_FIRE_NEO_NUM_LEDS  10
#define M5STACK_FIRE_NEO_DATA_PIN  15
#define M5STACKFIRE_MICROPHONE_PIN 34
#define M5STACKFIRE_SPEAKER_PIN 25 // speaker DAC, only 8 Bit

#define External_Neopixel_PIN    26 //fire GROVE  port B
#define External_Neopixel_NUM_LEDS  17
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel Ext_neo_pixels = Adafruit_NeoPixel(External_Neopixel_NUM_LEDS, External_Neopixel_PIN, NEO_GRB + NEO_KHZ800);




arduinoFFT FFT = arduinoFFT(); /* Create FFT object */



#define HORIZONTAL_RESOLUTION 320
#define VERTICAL_RESOLUTION   240
#define POSITION_OFFSET_Y      20
#define SIGNAL_LENGTH 512

double oldSignal[SIGNAL_LENGTH];
double adcBuffer[SIGNAL_LENGTH];
double vImag[SIGNAL_LENGTH];

#define SAMPLINGFREQUENCY 40000
#define SAMPLING_TIME_US     ( 1000000UL/SAMPLINGFREQUENCY )
#define ANALOG_SIGNAL_INPUT        M5STACKFIRE_MICROPHONE_PIN


POWER m5_power;
unsigned long run_cnt = 0;

void setup()
{
  M5.begin();


  M5.Lcd.setBrightness(50);  //define BLK_PWM_CHANNEL 7  PWM
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.println(" Fire Mic Spectrum V1.0");

  //for app flash back
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  //WiFi.mode(WIFI_OFF);
  //btStop();
  dacWrite(M5STACKFIRE_SPEAKER_PIN, 0); // make sure that the speaker is quite
  M5.Lcd.begin();
  M5.Lcd.fillScreen( BLACK );
  M5.Lcd.fillRect(10, 1, 150, 160, BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(GREEN);  //M5.Lcd.setTextSize(3);
  M5.Lcd.setTextSize(1);

  M5.Lcd.println("analog input MIC");
  M5.Lcd.print("max. frequency: "); M5.Lcd.print(SAMPLINGFREQUENCY / 2); M5.Lcd.println(" Hz");
  M5.Lcd.setTextSize(2);

  pixels.begin();
  Ext_neo_pixels.begin();

  m5_power.begin();
}

void showSignal()
{
  int n;

  int oldx;
  int oldy;
  int oldSig;
  int x, y;

  for (n = 0; n < SIGNAL_LENGTH / 2; n++)
  {
    x = n;
    y = map(adcBuffer[n], 0, 512, VERTICAL_RESOLUTION, POSITION_OFFSET_Y);

    if (n > 0)
    {
      // delete old line element
      M5.Lcd.drawLine(oldx , oldSig, x, oldSignal[n], BLACK );

      // draw new line element
      if (n < SIGNAL_LENGTH - 1) // don't draw last element because it would generate artifacts
      {
        M5.Lcd.drawLine(oldx,    oldy, x,            y, GREEN );
      }
    }
    oldx = x;
    oldy = y;
    oldSig = oldSignal[n];
    oldSignal[n] = y;
  }
}

double AdcMeanValue = 0;

void loop(void)
{
  int n;
  uint32_t nextTime = 0;
  uint32_t power = 0;
  //uint32_t meanValue = 0;
  // record signal
  for (n = 1; n < SIGNAL_LENGTH; n++)
  {
    double v = analogRead( ANALOG_SIGNAL_INPUT );
    AdcMeanValue += (v - AdcMeanValue) * 0.001;
    adcBuffer[n] = v - AdcMeanValue;

    // wait for next sample
    while (micros() < nextTime);
    nextTime = micros() + SAMPLING_TIME_US;
  }

  FFT.Windowing(adcBuffer, SIGNAL_LENGTH, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
  FFT.Compute(adcBuffer, vImag, SIGNAL_LENGTH, FFT_FORWARD); /* Compute FFT */
  FFT.ComplexToMagnitude(adcBuffer, vImag, SIGNAL_LENGTH); /* Compute magnitudes */
  //int x = FFT.MajorPeak(adcBuffer, SIGNAL_LENGTH, 1000000UL / SAMPLING_TIME_US);//SAMPLINGFREQUENCY
  int x = FFT.MajorPeak(adcBuffer, SIGNAL_LENGTH, SAMPLINGFREQUENCY);

  int maxAmplitudeDB = 0;
  for (n = 1; n < SIGNAL_LENGTH; n++)
  {
    power += adcBuffer[n]*adcBuffer[n];
    int a = log10(adcBuffer[n]) * 20 - 54.186; // convert amplitude to dB scale, dB relative to log10(512samples)*20=54.186dB
    if (a > maxAmplitudeDB) maxAmplitudeDB = a;
    adcBuffer[n] = (a + 30) * 5; // scale for TFT display
    vImag[n] = 0; // clear imaginary part
  }
  power /= SIGNAL_LENGTH;
  int threshold = 1000;

  //display pixel color based on mic power, threshold increase by 5 times every pixel!
  for (uint8_t n = 0; n < M5STACK_FIRE_NEO_NUM_LEDS; n++)
  {
    //  static uint32_t   Color(uint8_t r, uint8_t g, uint8_t b) {
    //return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
    //}
    pixels.setPixelColor(n, pixels.Color(0, 0, 1));
    if (power > threshold)  pixels.setPixelColor(n, pixels.Color(100+n*10, n*5, 0));
    threshold *= 5;
  }
  pixels.show();


    //display External_Neopixel_NUM_LEDS pixel color based on mic power, threshold increase by 5 times every pixel!
  for (uint8_t n = 0; n < External_Neopixel_NUM_LEDS; n++)
  {
    //  static uint32_t   Color(uint8_t r, uint8_t g, uint8_t b) {
    //return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
    //}
    Ext_neo_pixels.setPixelColor(n, Ext_neo_pixels.Color(0, 0, 1));
    if (power > threshold)  Ext_neo_pixels.setPixelColor(n, Ext_neo_pixels.Color(100+n*10, n*5, 0));
    threshold *= 3;
  }
  Ext_neo_pixels.show();




  
  showSignal();

  M5.Lcd.fillRect(200, 0, 119, 60, BLUE);
  M5.Lcd.setCursor(210, 1);
  M5.Lcd.print(x); M5.Lcd.print(" Hz");
  M5.Lcd.setCursor(210, 21);
  M5.Lcd.print(maxAmplitudeDB); M5.Lcd.print(" dB");
  M5.Lcd.setCursor(210, 41);
  M5.Lcd.print("Mic:"); M5.Lcd.printf("%.1f",AdcMeanValue);

}
