/*===========================================================================
  Title: smart device I2C Scanner for smart device based on ESP32 Wrover module
    - New Repository: https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
    - Original Repository: https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
  Version: 9
  Date: 2019 / 10 / 31
  Author: Ling zhou
  Email: tudzl@hotmail.de

  
  Version: 8
  Date: 2018 / 02 / 17
  Author: Austin St. Aubin
  Email: AustinSaintAubin@gmail.com
  
  Description: 
    ESP32 Scans for i2c Devices  
  
  Version History:
    Version 8
      Added preprocessor statments for display 
    Version 6
      Added i2c Ports
      Added OLED output
    Version 5, March 28, 2013
      As version 4, but address scans now to 127.
      A sensor seems to use address 120.
    Version 4, March 3, 2013, Using Arduino 1.0.3
      by Arduino.cc user Krodal.
      Changes by louarnold removed.
      Scanning addresses changed from 0...127 to 1...119,
      according to the i2c scanner by Nick Gammon
      http://www.gammon.com.au/forum/?id=10896
    Version 3, Feb 26  2013
      V3 by louarnold
    Version 2, Juni 2012, Using Arduino 1.0.1
       Adapted to be as simple as possible by Arduino.cc user Krodal
    Version 1
      This program (or code that looks like it)
      can be found in many places.
      For example on the Arduino.cc forum.
      The original author is not known.
      
  Notes:
  
    This sketch tests the standard 7-bit addresses
    Devices with higher bit address might not be seen properly.
    
  ============================================================================= */

// [# Preprocessor Statements #]
#define OLED_DISPLAY false    // Enable OLED Display
//#define OLED_DISPLAY_SSD1306  // OLED Display Type: SSD1306(OLED_DISPLAY_SSD1306) / SH1106(OLED_DISPLAY_SH1106), comment this line out to disable oled

// [ Included Library's ]
#include <Wire.h>

// Initialize the OLED display using I2C
#ifdef OLED_DISPLAY_SSD1306
  //#include "SSD1306.h"   // alias for `#include "SSD1306Wire.h"  ori
  #include "Adafruit_SSD1306.h"  //by zl
#elif defined OLED_DISPLAY_SH1106
  #include "SH1106.h"  // alias for `#include "SH1106Wire.h"`
#elif !OLED_DISPLAY
  #warning "OLED Display Disabled"
#else
  #error "Undefined OLED Display Type"
#endif
/*For a connection via I2C using brzo_i2c (must be installed) include
    #include <brzo_i2c.h>  Only needed for Arduino 1.6.5 and earlier
    #include "SSD1306Brzo.h"
    #include "SH1106Brzo.h"
  For a connection via SPI include
    #include <SPI.h>  Only needed for Arduino 1.6.5 and earlier
    #include "SSD1306Spi.h"
    #include "SH1106SPi.h" 
*/

// [ Global Pin Constants / Varables ]
/*Initialize the OLED display using SPI
  D5 -> CLK
  D7 -> MOSI (DOUT)
  D0 -> RES
  D2 -> DC
  D8 -> CS */
//const uint8_t I2C_SDA_PIN = D1; //SDA;  // i2c SDA Pin
//const uint8_t I2C_SCL_PIN = D2; //SCL;  // i2c SCL Pin

// [ Global Classes ]
#ifdef OLED_DISPLAY_SSD1306
  SSD1306  display(0x3c, I2C_SDA_PIN, I2C_SCL_PIN);
#elif defined OLED_DISPLAY_SH1106
  SH1106 display(0x3c, I2C_SDA_PIN, I2C_SCL_PIN);
#endif

void setup()
{
   //Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
   Wire.begin();
  // Serial
  Serial.begin(115200);
  Serial.println("\nSmart device I2C Scanner");

  // OLED
  #if OLED_DISPLAY
    display.init();
    display.flipScreenVertically();
    display.setContrast(255);
    display.setFont(ArialMT_Plain_10);
    display.setColor(WHITE);
    // - - - - - - - - - - - - - -
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(DISPLAY_WIDTH, 0, "v8");
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "I2C Scanner w/ OLED");
    // - - - - - - - - - - - - - -
    display.drawLine(0, 12, DISPLAY_WIDTH, 12);
    // - - - - - - - - - - - - - -
    display.display();
  #endif
}


void loop()
{
  byte error, address;
  int nDevices;

  Serial.println("Sensor Scanning...");

  #if OLED_DISPLAY
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 14, "Scanning...");
    display.display();
  #endif
  
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      nDevices++;
      
      Serial.print("Sensor found @ 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
      
      #if OLED_DISPLAY
        display.drawString(((nDevices -1) % 4) * 32, 23 + (round((nDevices -1) /4) *10), (String)nDevices + ":x" + String(address, HEX));
        display.display();
      #endif
    }
    else if (error==4) 
    {
      Serial.print("Unknow error @ 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);

      #if OLED_DISPLAY
        display.drawString(((nDevices -1) % 4) * 32, 23 + (round((nDevices -1) /4) *10), "!:x" + String(address, HEX));
        display.display();
      #endif
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
    
    #if OLED_DISPLAY
        display.drawString(0, 23 , "No I2C devices found");
        display.display();
    #endif
  } else {
    Serial.println("done\n");
    
    #if OLED_DISPLAY
        display.drawString(((nDevices -0) % 4) * 32, 23 + (round((nDevices -0) /4) *10), "done");
        display.display();
    #endif
  }

// DEBUGING
//  for(address = 40; address < 52; address++ ) {
//    nDevices++;
//    display.drawString(((nDevices -1) % 4) * 32, 23 + (round((nDevices -1) /4) *10), (String)nDevices + ":x" + String(address, HEX));
//    Serial.println(address,HEX);
//  }
  
  delay(5000);           // wait 5 seconds for next scan

  #if OLED_DISPLAY
    // Clear Bottom of Display
    display.setColor(BLACK);
    display.fillRect(0, 13, DISPLAY_WIDTH, DISPLAY_HEIGHT - 13);
    display.setColor(WHITE);
  #endif
}
