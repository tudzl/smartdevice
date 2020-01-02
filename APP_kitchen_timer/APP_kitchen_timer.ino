/* this code is working as APP launcher compatible
    kitchen_timer APP, by https://github.com/hideaki-kawahara/kitchen_timer/tree/develop
    Version 1.1   working with m5stack，  2.1.2020
    m5stack fire arduino device test app for kitchen_timer
    Author ling zhou, 30.12.2019
    note: need real device test
*/

#include <M5Stack.h>
//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
//#include <M5TreeView.h> // https://github.com/lovyan03/M5Stack_TreeView
//#include <M5OnScreenKeyboard.h> // https://github.com/lovyan03/M5Stack_OnScreenKeyboard/



#include <Adafruit_NeoPixel.h>

//Hardware pins
#define M5STACK_FIRE_NEO_NUM_LEDS  10
#define M5STACK_FIRE_NEO_DATA_PIN  15
#define M5STACKFIRE_MICROPHONE_PIN 34

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
POWER m5_power;


boolean but_A = false, but_LEFT = false, but_RIGHT = false;
uint32_t targetTime = 0;
byte omm = 99, oss = 99;
uint8_t mm = 2;
int8_t ss = 0;
bool hasStop = true;
bool hasEnd = false;
void setup() {
  M5.begin();
  M5.Lcd.setBrightness(50);  //define BLK_PWM_CHANNEL 7  PWM
  M5.Lcd.fillScreen(TFT_IVORY); //In_eSPI.H

  //for app flash back
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }


  targetTime = millis() + 1000;

  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  M5.Lcd.setCursor( 60, 200);
  M5.Lcd.print("M");
  M5.Lcd.setCursor( 150, 200);
  M5.Lcd.print("S");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor( 240, 200);
  M5.Lcd.print("START");
  M5.Lcd.setCursor( 240, 220);
  M5.Lcd.print("/STOP");

  Serial.println("START");
}

void loop() {
  if (M5.BtnC.wasPressed() && !hasStop) {
    hasStop = true;
  } else if (M5.BtnC.wasPressed() && hasStop) {
    hasStop = false;
  }
  if (M5.BtnB.wasPressed() && hasStop) {
    ss++;
    hasEnd = false;
    if (ss == 60) {
      ss = 0;
    }
    draw_seconds(ss);
  }
  if (M5.BtnA.wasPressed() && hasStop) {
    mm++;
    hasEnd = false;
    if (mm == 60) {
      mm = 0;
    }
    draw_minutes(mm);
  }

  //long press BtnB to reset timer!
  if (M5.BtnB.pressedFor(300) && hasStop) {
    buttonB_longPressed();
    M5.Speaker.beep();
  }


  M5.update();


  if (targetTime < millis()) {
    targetTime = millis() + 1000;

    // 時間停止してない
    if (!hasStop) {
      ss--;
      if (ss == -1) {
        // 分秒ともに0なら表示停止＆timerストップ
        // Timer count down to zero
        if (mm == 0) {
          ss = 0;
          hasStop = true;
          hasEnd = true;
          M5.Speaker.beep();
          delay(100);
          M5.Speaker.beep();

        } else {
          ss = 59;
          omm = mm;
          mm--;
        }
      }
    }

    if (hasEnd) {
      // timerストップの表示
      M5.Lcd.setCursor( 100, 150);
      M5.Lcd.setTextFont(2);
      M5.Lcd.print("Timer Stop");
      //rgb
      for (uint8_t n = 0; n < M5STACK_FIRE_NEO_NUM_LEDS; n++)
      {
        pixels.setPixelColor(n, pixels.Color(100 + n * 10, 10, 10));
      }
    } else {
      // 表示を消す
      M5.Lcd.setCursor( 100, 150);
      M5.Lcd.setTextFont(2);
      M5.Lcd.print("          ");
    }

    // 7seg font
    M5.Lcd.setTextFont(7);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);

    // 分の更新
    if (omm != mm) {
      omm = mm;
      draw_minutes(mm);
    }

    // 秒の更新
    if (oss != ss) {
      oss = ss;

      // インジケーターの点滅
      if (ss % 2 && !hasStop) {
        M5.Lcd.setCursor( 150, 40);
        M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_LIGHTGREY);
        M5.Lcd.print(":");
        M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
      }
      else {
        M5.Lcd.setCursor( 150, 40);
        M5.Lcd.print(":");
      }

      draw_seconds(ss);
    }
  }
}

// 分の表示
void draw_minutes(int mm) {
  if (mm < 10) {
    M5.Lcd.setCursor( 0, 40);
    M5.Lcd.print("0");
    M5.Lcd.print(mm);
  } else {
    M5.Lcd.setCursor( 0, 40);
    M5.Lcd.print(mm);
  }
}

// 秒の表示
void draw_seconds(int ss) {
  if (ss < 10) {
    M5.Lcd.setCursor( 180, 40);
    M5.Lcd.print("0");
    M5.Lcd.print(ss);
  } else {
    M5.Lcd.setCursor( 180, 40);
    M5.Lcd.print(ss);
  }
}

//reset timer
void buttonB_longPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  //M5.lcd.setBrightness(30);


  Serial.println(" --^^^--- buttonB is longPressed over 0.3s !!!");
  mm = 0;
  ss = 0;
  Serial.println(" Reset Timer !!");

}
