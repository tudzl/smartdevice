/***************************************************
  This is a example sketch demonstrating the graphics
  capabilities of the SSD1331 library  for the 0.96"
  16-bit Color OLED with SSD1331 driver chip
  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/684
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

  with software SPI works!
  ESP32 use HSPI mode 3 not working!
  mode3 means when CPOL=1, CPHA=1. When SPI is idle, the clock output is logic high; data changes on
  the falling edge of the SPI clock and is sampled on the rising edge.
 ****************************************************/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>





//pin def for motor control
#define OUT1 12
#define OUT2 13
#define OUT3 32
#define OUT4 33



//define the pins used by the 0.95 RGB OLED SPI ssd1331 module

//#define FSPI  1 //SPI bus attached to the flash (can use the same data lines but different SS)
//#define HSPI  2 //SPI bus normally mapped to pins 12 - 15, but can be matrixed to any pins, micro SD card interface for TTGO?
//#define VSPI  3 //SPI bus normally attached to pins 5, 18, 19 and 23, but can be matrixed to any pins


//ori lib def vspi,,TTGO T18 pins
#define SCK 18
#define MOSI 23
#define DC 27
#define CS 15
#define RST 4
//SCLK-----GPIO #14
//MOSI-----GPIO #13
//MISO-----NONE
//CS-------GPIO #15
//D/C------GPIO #21
//RST------GPIO #4


//pin def for smartdevice PCB v2
//#define smartdevice_PCB
//#define SCK 18
//#define MOSI 23
//#define CS 14
//#define DC 27 //_C low for CMD
//#define RST 33



//#define SPI_Freq 7000000  //6.671M  150nS!!!  time used: 12264 ms
#define SPI_Freq 5000000  //5M    time used: 12438 ms
//#define SPI_Freq 6000000  // 6M-->5.714MHz
//#define SPI_Freq 4000000  //4M  faster then SW SPI time used: 12638 ms
//#define SPI_Freq 1000000  //1M  slower then SW SPI  time used: 15290 ms

#define LED_pin 2

//ESP32_SSD1331 display = ESP32_SSD1331(SCK, MISO,MOSI,CS,DC,RST);

// Color definitions
#define  BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF
#define MilkWhite           0xCFCF
bool HardwareSPI_EN = true;
const uint8_t   OLED_pin_scl_sck        = 18;
const uint8_t   OLED_pin_sda_mosi       = 23;
const uint8_t   OLED_pin_cs_ss          = 15;
const uint8_t   OLED_pin_res_rst        = 4;
const uint8_t   OLED_pin_dc_rs          = 27;
// Option 1: use any pins but a little slower
//Adafruit_SSD1331 display = Adafruit_SSD1331(cs, dc, mosi, sclk, rst);
//Adafruit_SSD1331 display = Adafruit_SSD1331(CS, DC, MOSI, SCK, RST);
// declare the display with soft SPI @ about 1M, test ok!
//Adafruit_SSD1331 display =
//    Adafruit_SSD1331(
//        OLED_pin_cs_ss,
//        OLED_pin_dc_rs,
//        OLED_pin_sda_mosi,
//        OLED_pin_scl_sck,
//        OLED_pin_res_rst
//     );
//---------------------------------------------------------------------
// Option 2: must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)

//SPIClass hwspi(HSPI); //hspi->begin(0, 2, 4, 33); //SCLK, MISO, MOSI, SS
SPIClass hwspi(VSPI); //SCLK = 18, MISO = 19, MOSI = 23, SS = 5

Adafruit_SSD1331 display = Adafruit_SSD1331(&hwspi, CS, DC, RST); // working! default generate 8M


float p = 3.1415926;
uint16_t  run_cnt = 0;
uint8_t led_status = 0;
void LED_run(void) {

  led_status = 1 - led_status;
  if (led_status)
    digitalWrite(LED_pin, HIGH);
  else
    digitalWrite(LED_pin, LOW);

}


void setup(void) {

  pinMode(LED_pin, OUTPUT); //RES
  digitalWrite(LED_pin, LOW);


  pinMode(OUT1, OUTPUT); //RES
  digitalWrite(OUT1, LOW);

  pinMode(OUT2, OUTPUT); //RES
  digitalWrite(OUT2, LOW);

  pinMode(OUT3, OUTPUT); //RES
  digitalWrite(OUT3, LOW);

  pinMode(OUT4, OUTPUT); //RES
  digitalWrite(OUT4, LOW);


  if (HardwareSPI_EN) {
    //display = Adafruit_SSD1331(&hwspi, CS, DC, RST); // working! default generate 8M
    hwspi.begin(18, 19, 23, 32); //SCLK, MISO, MOSI, SS
    hwspi.setFrequency(SPI_Freq);
    display.begin(SPI_Freq); //adfruit
  }
  else
    display.begin();

  //SPI.begin(SCK, MISO, MOSI, CS);
  Serial.begin(115200);
  //Serial.println("hello! Adafruit_SSD1331 HW SPI driver test");


  Serial.println("OLED init");

  Serial.println("fillScreen blue"); //18.18ms, about 2.6 us/pixel

  LED_run();
  // display.writeFillRect(0, 20, 40, 40, BLACK); //test ok!  writeFillRectPreclipped
  display.fillScreen(BLUE);

  LED_run();

  Serial.println("fillScreen black");
  //display.startWrite();
  display.fillScreen(BLACK);
  Serial.println("OLED motor L298N test");
  //TT =   millis();
  display.setCursor(0, 0);
  display.print(" OLED motor driver test v0.1");//GFX--> write -- not working

}

void loop() {

  uint32_t tt;
  tt = millis();
  LED_run();
  run_cnt++;

  motor_run_CW(5);
  tt = millis() - tt;
  Serial.printf("loop %d,time cost: %d ms\r\n", run_cnt, tt);


  delay(2);
}


void motor_run_CW(uint16_t delay_time) {


  //正转：
  //（1）四拍：A 1000，A- 0010,B 0100,B- 0001.
  //（2）八拍：A 1000，AA- 1010,A- 0010,A-B 0110,B 0100,BB- 0101,B- 0001,B-A 1001.
  //  digitalWrite(OUT1, LOW);
  //  digitalWrite(OUT2, LOW);
  //  digitalWrite(OUT3, LOW);
  //  digitalWrite(OUT4, LOW);

  if (delay_time < 1)
    delay_time = 4;
  //step1
  digitalWrite(OUT1, HIGH);
  digitalWrite(OUT2, LOW);
  digitalWrite(OUT3, LOW);
  digitalWrite(OUT4, LOW);
  delay(delay_time);
  //step2
  digitalWrite(OUT1, LOW);
  digitalWrite(OUT2, LOW);
  digitalWrite(OUT3, HIGH);
  digitalWrite(OUT4, LOW);
  delay(delay_time);
    //step3
  digitalWrite(OUT1, LOW);
  digitalWrite(OUT2, HIGH);
  digitalWrite(OUT3, LOW);
  digitalWrite(OUT4, LOW);
  delay(delay_time);
  //step4
  digitalWrite(OUT1, LOW);
  digitalWrite(OUT2, LOW);
  digitalWrite(OUT3, LOW);
  digitalWrite(OUT4, HIGH);
  delay(delay_time);

}

void testlines(uint16_t color) {
  display.fillScreen(BLACK);
  for (int16_t x = 0; x < display.width() - 1; x += 6) {
    LED_run();
    LED_run();
    display.drawLine(0, 0, x, display.height() - 1, color); //drawline bugs?

  }
  for (int16_t y = 0; y < display.height() - 1; y += 6) {
    display.drawLine(0, 0, display.width() - 1, y, color);
  }

  display.fillScreen(BLACK);
  for (int16_t x = 0; x < display.width() - 1; x += 6) {
    display.drawLine(display.width() - 1, 0, x, display.height() - 1, color);
  }
  for (int16_t y = 0; y < display.height() - 1; y += 6) {
    display.drawLine(display.width() - 1, 0, 0, y, color);
  }

  // To avoid ESP8266 watchdog timer resets when not using the hardware SPI pins
  delay(0);

  display.fillScreen(BLACK);
  for (int16_t x = 0; x < display.width() - 1; x += 6) {
    display.drawLine(0, display.height() - 1, x, 0, color);
  }
  for (int16_t y = 0; y < display.height() - 1; y += 6) {
    display.drawLine(0, display.height() - 1, display.width() - 1, y, color);
  }

  display.fillScreen(BLACK);
  for (int16_t x = 0; x < display.width() - 1; x += 6) {
    display.drawLine(display.width() - 1, display.height() - 1, x, 0, color);
  }
  for (int16_t y = 0; y < display.height() - 1; y += 6) {
    display.drawLine(display.width() - 1, display.height() - 1, 0, y, color);
  }

}

void testdrawtext(char *text, uint16_t color) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
}

void testfastlines(uint16_t color1, uint16_t color2) {
  display.fillScreen(BLACK);
  for (int16_t y = 0; y < display.height() - 1; y += 5) {
    display.drawFastHLine(0, y, display.width() - 1, color1);
  }
  for (int16_t x = 0; x < display.width() - 1; x += 5) {
    display.drawFastVLine(x, 0, display.height() - 1, color2);
  }
}

void testdrawrects(uint16_t color) {
  display.fillScreen(BLACK);
  for (int16_t x = 0; x < display.height() - 1; x += 6) {
    display.drawRect((display.width() - 1) / 2 - x / 2, (display.height() - 1) / 2 - x / 2 , x, x, color);//bug!
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  display.fillScreen(BLACK);
  for (int16_t x = display.height() - 1; x > 6; x -= 6) {
    display.fillRect((display.width() - 1) / 2 - x / 2, (display.height() - 1) / 2 - x / 2 , x, x, color1);
    display.drawRect((display.width() - 1) / 2 - x / 2, (display.height() - 1) / 2 - x / 2 , x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (uint8_t x = radius; x < display.width() - 1; x += radius * 2) {
    for (uint8_t y = radius; y < display.height() - 1; y += radius * 2) {
      display.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = 0; x < display.width() - 1 + radius; x += radius * 2) {
    for (int16_t y = 0; y < display.height() - 1 + radius; y += radius * 2) {
      display.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  display.fillScreen(BLACK);
  int color = 0xF800;
  int t;
  int w = display.width() / 2;
  int x = display.height();
  int y = 0;
  int z = display.width();
  for (t = 0 ; t <= 15; t += 1) {
    display.drawTriangle(w, y, y, x, z, x, color);
    x -= 4;
    y += 4;
    z -= 4;
    color += 100;
  }
}

void testroundrects() {
  display.fillScreen(BLACK);
  int color = 100;
  int i;
  int t;
  for (t = 0 ; t <= 4; t += 1) {
    int x = 0;
    int y = 0;
    int w = display.width();
    int h = display.height();
    for (i = 0 ; i <= 8; i += 1) {
      display.drawRoundRect(x, y, w, h, 5, color);
      x += 2;
      y += 3;
      w -= 4;
      h -= 6;
      color += 1100;
    }
    color += 100;
  }
}

void tftPrintTest() {
  display.fillScreen(BLACK);
  //display.writeFillRect(0, 0, 95, 63, BLACK); //test ok!  writeFillRectPreclipped
  display.setCursor(0, 5);
  display.setTextColor(RED);
  display.setTextSize(1);
  display.println("Hello World!");
  display.setTextColor(YELLOW, GREEN);
  display.setTextSize(2);
  display.print("2020");
  display.setTextColor(BLUE);
  display.setTextSize(3);
  display.print(1234.567);
  delay(1500);
  display.setCursor(0, 5);
  display.fillScreen(BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(0);
  display.println("Hello World!");
  display.setTextSize(1);
  display.setTextColor(GREEN);
  display.print(p, 5);
  display.println(" Want pi?");
  display.print(8675309, HEX); // print 8,675,309 out in HEX!
  display.print(" Print HEX");
  display.setTextColor(WHITE);
  display.println("Sketch has been");
  display.println("running for: ");
  display.setTextColor(MAGENTA);
  display.print(millis() / 1000);
  display.setTextColor(WHITE);
  display.print(" seconds.");
}

void mediabuttons() {
  // play
  display.fillScreen(BLACK);
  display.fillRoundRect(25, 10, 78, 60, 8, WHITE);
  display.fillTriangle(42, 20, 42, 60, 90, 40, RED);
  delay(500);
  // pause
  display.fillRoundRect(25, 90, 78, 60, 8, WHITE);
  display.fillRoundRect(39, 98, 20, 45, 5, GREEN);
  display.fillRoundRect(69, 98, 20, 45, 5, GREEN);
  delay(500);
  // play color
  display.fillTriangle(42, 20, 42, 60, 90, 40, BLUE);
  delay(50);
  // pause color
  display.fillRoundRect(39, 98, 20, 45, 5, RED);
  display.fillRoundRect(69, 98, 20, 45, 5, RED);
  // play color
  display.fillTriangle(42, 20, 42, 60, 90, 40, GREEN);
}

/**************************************************************************/
/*!
    @brief  Renders a simple test pattern on the LCD
*/
/**************************************************************************/

//time used: 579 ms @2M HWSPI VSPI
void lcdTestPattern(void)
{
  uint8_t w, h;
  //display.setAddrWindow(0, 0, 96, 64); //actually write 0,0,95,63.  include startWrite() at the end of setAddrWindow;
  display.writePixel(0, 0, WHITE); //writePixels works!
  //display.endWrite(); //needed, in
  display.writePixel(2, 0, WHITE); //writePixels
  // display.endWrite();
  display.writePixel(4, 0, RED); //writePixels
  //display.endWrite();
  //display.startWrite();
  //display color bars
  for (h = 0; h < 64; h++) {
    for (w = 0; w < 96; w++) {
      if (w > 83) {
        //display.drawPixel(w, h, WWWW);//not working
        display.writePixel(w, h, MilkWhite);
        //display.endWrite();
      } else if (w > 71) {
        display.writePixel(w, h, BLUE);
        //display.endWrite();
      } else if (w > 59) {
        display.writePixel(w, h, GREEN);
        //display.endWrite();
      } else if (w > 47) {
        display.writePixel(w, h, CYAN);
        //display.endWrite();
      } else if (w > 35) {
        display.writePixel(w, h, RED);
        //display.endWrite();
      } else if (w > 23) {
        display.writePixel(w, h, MAGENTA);
        //display.endWrite();
      } else if (w > 11) {
        display.writePixel(w, h, YELLOW);
        //display.endWrite();
      } else {
        // Serial.println("fisrt bar Black");
        // display.drawPixel(w, h, WHITE);

        display.writePixel(w, h, WHITE); //signle bar cost 8.5ms @2M HWSPI
        //display.endWrite();
      }
    }
    //delay(1);
  }
  //display.writePixel(0, 0, WHITE); //writePixels works!
  //display.writePixel(1, 0, WHITE); //writePixels works!
  //display.endWrite();
}


//a single pixel
// Serial.printf("drawPixel GREEN at %d,%d \r\n", display.width() / 2, display.height() / 2);

//display.drawPixel(display.width() / 2, display.height() / 2, BLACK); //OK
//  delay(500);


//  Serial.println("writeFillRect black");
//  display.writeFillRect(0, 20, 40, 40, BLACK); //test ok!  writeFillRectPreclipped
//  //display.endWrite();
//  Serial.println("writeFillRect finish");
//display.fillScreen(BLACK);//not working for HW SPI
//Adafruit_GFX::fillRect-->writeFillRectPreclipped-->writeColor
//drawFastVLine-->writeLine(x, y, x, y+h-1, color)  writeLine not working
//writeLine--writePixel!!!
//Adafruit_SSD1331 OLED_095 = Adafruit_SSD1331(&hwspi, CS, DC, RST);


// 3st line draw test
//  Serial.println("testlines");
//
//  Serial.println("writeFastHLine");
//  display.writeFastHLine(0, 32, 10, RED);//ok
//  Serial.println("GFX writeLine1");
//  //OLED_095.drawPixel(display.width() / 2+2, display.height() / 2, BLACK); //OK
//  //display.startWrite();
//  display.writeLine(0, 0, 64, display.height() - 1, GREEN); //not working at writeLine(), drawPixel works!~!
//  Serial.println("GFX drawLine2");
//  display.drawLine(32, 63, 64, 0 , BLUE); //not working at writeLine()
/*Adafruit_GFX::writeLine
  #GFX::writePixel
  #Adafruit_SPITFT::writePixel

*/

//----run results
//init
//fillScreen blue
//start screen test now!
//lcdTestPattern() finish
//time used: 419 ms
//fillScreen black
//OLED Print text: RGB OLED driver test v0.9
//time used: 1459 ms
//tftPrintTest
//YELLOW_testlines
//time used: 6762 ms
//testfastlines
//testdrawrects
//time used: 7808 ms
//testfillrects
//testfillcircles
//testdrawcircles
//testroundrects
//testtriangles
//time used: 12264 ms
//done
//loop 1000,time cost: 16334 ms
//loop 2000,time cost: 17334 ms
