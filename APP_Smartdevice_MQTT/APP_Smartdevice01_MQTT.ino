
/*  Smart device Main PCB code for ESP32 wrover and 6 sensors
    this code is working as APP launcher compatibale
    Version 6.4 add MCP9808 sensor function contain value convertion bugs 2019.11.11
    Version 6.3 config Wifi ssid through MQTT 2019.11.11 not tested
    Version 6.2 Wifi config switch through S2 btn  2019.11.8
    Version 6.1 MQTT working!  2019.11.8
    Version 6.0 Light , BME280 working!  2019.11.1
    Version 5.0  Add altitude base , offset diff measure function
    Version 4.0  HP206C bug fixed!  altitude: ulong changed to long
    Version 3.1  Zfilter and kalman filter works!
    Version 2.2  HP206C works!
    need to improve BME680 T reading accuracy!
    m5stack fire arduino device test app for ENV unit and BME680 module +HP206C  module
    Author ling zhou, 16.8.2019
    &&&Loop routine:
    1. read and process sensor values
    2. MQTT check, compose msg payload and send msgs
    3. Btn scan, sys run info process

    ###buttonA_longPressed to swirch MQTT active
    ###device ID need change for different boaards!!!
    note: ONboard sensors: BME280+ICM-20948+MCP9808T-E/MS+BH1750+MAX44009EDT+analog Mic ICS-40181

    note: need add library Adafruit_BME680 from library manage

    Sketch uses 866978 bytes (27%) of program storage space. Maximum is 3145728 bytes.
    Global variables use 42080 bytes (12%) of dynamic memory, leaving 285600 bytes for local variables. Maximum is 327680 bytes.

*/


/***********************ICM_20948 9-axis motion sensor*****************************************
   Example1_Basics.ino
   ICM 20948 Arduino Library Demo
   Use the default configuration to stream 9-axis IMU data
   Owen Lyke @ SparkFun Electronics
   Original Creation Date: April 17 2019

   This code is beerware; if you see me (or any other SparkFun employee) at the
   local, and you've found our code helpful, please buy us a round!

   Distributed as-is; no warranty is given.
 ***************************************************************/


//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
//#include <M5TreeView.h> // https://github.com/lovyan03/M5Stack_TreeView
//#include <M5OnScreenKeyboard.h> // https://github.com/lovyan03/M5Stack_OnScreenKeyboard/

#include <M5Stack.h>
//#include "utility/Power.h"

#include "DHT12.h"

#include <Wire.h> //The DHT12 uses I2C comunication.

#include "Adafruit_Sensor.h"
#include "Adafruit_BME680.h"
#include "Adafruit_BMP280.h"
#include "Adafruit_BME280.h"
#include <BH1750FVI.h>
#include <HP206C_Zlib.h>
#include <ZFilter.h>
#include <MAX44009.h>  //Wide 0.045 Lux to 188,000 Lux Range
#include "Adafruit_MCP9808.h"
//For #include "filename" the preprocessor searches first in the same directory as the file containing the directive, and then follows the search path used for the #include <filename> form. This method is normally used to include programmer-defined header files.
#include "ICM_20948.h"  // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU


//WIFI+MQTT
#define MQTT_KEEPALIVE 30
#include <WiFi.h>
#include <PubSubClient.h>  //MQTT
//mqtt.subscribe(str('stsmd/'+Device_ID+'/alert')
//test_ESP32_smartdevice


#define SEALEVELPRESSURE_HPA (1013.25)
//bmp280 address:  0x76
//HP206C address:  0x76
//BME680 address:  0x76-->0x77:
unsigned char Bh1750_add = 0x23;
unsigned char BME280_add = 0x77;
unsigned char BME680_add = 0x77;
unsigned char hp206_add = 0x76;
unsigned char bmp280_add = 0x76;
unsigned char MAX44009_add = 0x4B;
unsigned char MCP9808_add = 0x19;
unsigned char ICM20948_add = 0x68;
// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 MCP9808 = Adafruit_MCP9808();
//DHT address:  0x5C
MAX44009 MAX44009_light;  //bot light sensor
DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
BH1750FVI::eDeviceMode_t BH_DEVICEMODE = BH1750FVI::k_DevModeContHighRes2;
BH1750FVI Top_LightSensor(BH_DEVICEMODE);
Adafruit_BME680 bme680; // I2C  0x76
Adafruit_BMP280 bmp280;  // I2C  0x76
Adafruit_BME280 bme280;  // I2C  0x77
HP20x_dev HP206;  // I2C  0x76
//ICM_20948_I2C def
ICM_20948_I2C ICM20948 ; //9-axis motion

POWER m5_power;
//global vars
float lux_BH1750 = 0;
float lux_max44009 = 0;
float pressure = 0;
float T_bmp = 0;
float Alt_bmp = 0;
double Altitude = 0; // in meter @ 25 degree
double Altitude_offset = 0;   // in meter
bool top_light_ok = false;  //BH
bool bot_light_ok = false; //max
bool dht12_ok = false;
bool BME280_ok = false;
bool bmp280_ok = false;
bool bme680_ok =  false;
bool HP206_ok = false;
bool MCP9808_ok = false;
bool ICM20948_ok = false;

unsigned int HP206_DSR = 256; // down sampling rate
float hum = 0;
float tmperature = 0;

float MCP9808_T = 0;
int16_t T_limit_H = 40;
int16_t T_limit_L = 5;
int16_t T_limit_Critical = 50;

float bme680_T_offset = 5;
unsigned int Gas = 0;
bool Gas_EN = true;
float Hum_BME680 = 0;

//Motion sensor vars
float ACC_X_offset = 0;
float ACC_Y_offset = 0;
float ACC_Z_offset = 0;
float Vibration = 0;
/* KalmanFilter Instance */
bool filter_on = true;
ZFilter t_filter;    //temperature filter
ZFilter p_filter;    //pressure filter
ZFilter a_filter;    //altitude filter
KalmanFilter t_kalman;    //temperature filter
KalmanFilter p_kalman;    //pressure filter
KalmanFilter a_kalman;    //altitude filter
float T_filter = 0;
float P_filter = 0;
float A_filter = 0;
float T_Kfilter = 0;
float P_Kfilter = 0;
float A_Kfilter = 0;
#define Filter_len 5
float filter_buf[Filter_len];
float filter_val = 0; //average value
unsigned int idx = 0;
//POWER.ShutdownTime SHDT；

//*********global control para
//SHDT = ShutdownTime.SHUTDOWN_64S; // SHDT is assigned a particular enum value
const unsigned int wifi_Timeout = 12000; //12s
bool Wrover_module = true  ;
unsigned long run_cnt = 0;
unsigned int time_interval = 485; //cycle time
//---sys run vars---
unsigned char dev_cnt = 0;
bool MQTT_active = false;
//------ Height meter vars
float height_base =  0;
float height_new =  0;
float height_diff =  0;


// Blink ledPin refers to smart device ESP32 GPIO 2
const int ledPin = 2;
unsigned char blink_status = 1;

//********************MQTT****************************
// Replace the next variables with your SSID/Password combination
bool wifi_ok = false;
const char* ssid = "TC";
const char* password = "sthz@2020";
const char* ssid2 = "WH10";
const char* password2 = "Zell9090";
const char* ssid3 = "HUAWEI_XM"; //test only
const char* password3 = "Zell9090";
char ssid_new [12];
char password_new [12];
unsigned char wifi_config = 2 ;
// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "st.devmolv.com";
const char* device_ID = "SmartDevice_01";
//Strings are actually one-dimensional array of characters terminated by a null character '\0'.
const char* MQTT_SensorMsg_head = "stsmd/SmartDevice_01/local";
const char* MQTT_GlobalMsg_head = "stsmd/SmartDevice_01/global";
char MQTT_payload[100] ;
const char  Data_ava_flag = B1001111 ; //ori 1000111

WiFiClient SmartDevice_01;
PubSubClient client(SmartDevice_01);

long lastMsg = 0;
char msg[64];
char tmp_string[32];
//int value = 0;



//for bme680 only, toggle gas measurrement
void buttonA_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  Serial.println(" ！！！buttonA/S1 was Pressed");
  Serial.println("##reset motion sensor offset now！");
  //Gas_EN = !Gas_EN;
  if ( ICM20948.dataReady() ) {
    ICM20948.getAGMT();
    reset_ACC_offset(ICM20948.agmt);
  }

  Serial.printf("ACC ofsets X:%6.1f mG; Y:%6.1f mG; Z:%6.1f mG\r\n", ACC_X_offset, ACC_Y_offset, ACC_Z_offset);

}

//set the lcd Brightness
void buttonA_longPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  //M5.lcd.setBrightness(30);
  MQTT_active = 1 - MQTT_active;
  Serial.println(" ！！！buttonA/S1 is longPressed");
  Serial.printf("##MQTT_active: %d", MQTT_active);
  if (false == MQTT_active) {
    Serial.printf("Smart device MQTT OFF now...\r\n");
    client.publish("strtd/SmartDevice_01/online", "");
  }
  else {
    Serial.printf("Smart device MQTT go Online now!\r\n");
    client.publish("strtd/SmartDevice_01/online", "now online!");
  }
}

//void buttonB_wasPressed(void) {
//  //M5.Speaker.beep();  // too laud
//  //M5.Speaker.tone(800, 20);
//  if (HP206_ok)
//    height_base = A_Kfilter;
//  else if (bme680_ok)
//    height_base = Altitude;
//}
////set the altitude offset to current altitude
//void buttonB_longPressed(void) {
//  //M5.Speaker.beep();  // too laud
//  //M5.Speaker.tone(800, 20);
//  Altitude_offset = Altitude;
//
//}
//
//void buttonC_wasPressed(void) {
//  //M5.Speaker.beep();  // too laud
//  //M5.Speaker.tone(800, 20);
//
//
//  if (HP206_ok)
//    height_new = A_Kfilter;
//  else if (bme680_ok)
//    height_new = Alt_bmp;
//  height_diff = height_new - height_base;
//
//}



void setup_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Wifi try Connecting to ");


  if (1 == wifi_config)
  { Serial.println(ssid);
    WiFi.begin(ssid, password);
  }
  else if (2 == wifi_config)
  { Serial.println(ssid);
    WiFi.begin(ssid2, password2);
  }

  long Zeit_anfang = millis();
  long now;
  float Zeit;
  Serial.printf("sys time Zeit_anfang: %d ms \r\n", Zeit_anfang);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    now = millis();
    //Zeit = now -Zeit_anfang;
    Serial.printf("time used so far: %d ms \r\n", now - Zeit_anfang);
    //time out break of while
    if ( (now - Zeit_anfang) > wifi_Timeout)
      break;
  }





  Serial.println("");


  if ( WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi is now connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    wifi_ok = true;
  }
  else  {
    wifi_ok = false;
    WiFi.disconnect(true);
    Serial.println("#connectting wifi failed");
  }

  Zeit = float(millis() - Zeit_anfang) / 1000.0f;
  //Zeit = Zeit/1000 ; // to S

  Serial.printf("#Setting up wifi time passed: %.2f seconds", Zeit);
  Serial.println("");
}

void MQTT_RX_callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  if (String(topic) == "config_ssid") {
    Serial.printf("Changing wifi ssid to: %s", messageTemp);
    sprintf(ssid_new, "%s", messageTemp);
    //ssid_new = messageTemp;
  }
  if (String(topic) == "config_password") {
    Serial.printf("Changing wifi password to %s", messageTemp);
    sprintf(password_new, "%s", messageTemp);
    //ssid_new = messageTemp;
  }





  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "test_ESP32_smartdevice_server") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      //digitalWrite(ledPin, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      //digitalWrite(ledPin, LOW);
    }
  }
}
//MQTT server reconnect,In the reconnect() function, you can subscribe to MQTT topics. In this case, the ESP32 is only subscribed to the test_ESP32_smartdevice_server:
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(device_ID)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("test_ESP32_smartdevice_server"); //need change
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup() {


  //void M5Stack::begin(bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable)
  M5.begin();

  Wire.begin();
  pinMode(ledPin, OUTPUT);
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load Wifi config_1, default is config_2");
    wifi_config = 1;
    //Serial.println("Will Load menu binary");
    //updateFromFS(SD);
    //ESP.restart();
  }
  dev_cnt = 0;





  Serial.println(F("<<<Smart device V2 sensor MQTT test>>>"));
  Serial.println(F("<<<Firmware Version 6.4, ling zhou, 11.11.2019>>>"));
  Serial.printf("<<<----Device ID: %s----->>>\r\n", device_ID);
  if (Top_LightSensor.begin2(Bh1750_add))
  {
    dev_cnt++;
    top_light_ok = true;
    Serial.println("* Top light sensor BH1750 is connected!");
    delay(50);
    Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
  }


  if (MCP9808.begin(MCP9808_add)) {
    MCP9808_ok = true;
    dev_cnt++;
    MCP9808.shutdown();
    MCP9808.wakeup();   // wake up, delay 250ms; ready to read!
    Serial.println("* MCP9808 -40°C to +125°C ±0.5°C Digital Temperature Sensor is connected!");
    Serial.printf("  MCP9808 Chip Rev.: %d \r\n", MCP9808.getRevision(MCP9808_add));

    Serial.println("-- MCP9808 default Resolution setting: 0.0625°C");
    Serial.println("-- MCP9808 default Sampling Time:  250 ms");
    Serial.println("-- MCP9808 configuring now...");

    MCP9808.setResolution(3); // sets the resolution mode of reading, the modes are defined in the table bellow:
    // Mode Resolution SampleTime
    //  0    0.5°C       30 ms
    //  1    0.25°C      65 ms
    //  2    0.125°C     130 ms
    //  3    0.0625°C    250 ms

    MCP9808.write16(MCP9808_REG_CONFIG, MCP9808_REG_CONFIG_ALERTCTRL);
    delay(1);
    //config thresholds
    uint16_t regval = T_limit_H * 4; //0.25 C step
    regval = regval << 2; //shit left 2 bits
    //Serial.printf("  regval: %d \r\n", regval);
    MCP9808.write16(MCP9808_REG_UPPER_TEMP, regval);
    regval = T_limit_L * 4; //0.25 C step
    regval = regval << 2; //shit left 2 bits
    MCP9808.write16(MCP9808_REG_LOWER_TEMP, regval);
    regval = T_limit_Critical * 4; //0.25 C step
    regval = regval << 2; //shit left 2 bits
    MCP9808.write16(MCP9808_REG_CRIT_TEMP, regval);

    regval = MCP9808.read16(MCP9808_REG_CONFIG);
    Serial.printf("  MCP9808 Config REG: %d \r\n", regval);
    regval = MCP9808.read16(MCP9808_REG_UPPER_TEMP);
    Serial.printf("  MCP9808 hight temp. alert reg: %d \r\n", regval);
    uint8_t resolution = MCP9808.getResolution();
    Serial.printf("  MCP9808 Resolution: %d (0 to 3, 3 means 0.0625°C)\r\n", resolution);
    Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
    delay(50);
  }
  else {
    Serial.println("Couldn't find MCP9808! Check your connections and verify the IIC address 0x19 is correct.");
  }


  if (MAX44009_light.begin2(MAX44009_add))
  {
    dev_cnt++;
    bot_light_ok = true;
    Serial.println("* Bot light sensor Max44009 is connected!");
    delay(50);

    //Two key features of the IC analog design are its ultra-low
    //current consumption (typically 0.65μA) and an extremely
    //wide dynamic light range that extends from 0.045 lux to
    //188,000 lux—more than a 4,000,000 to 1 range.
    Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
  }
  else  Serial.println("Couldn't find Bot light sensor Max44009! Check your connections and verify the IIC address 0x4B is correct.");

  if (bme280.begin(BME280_add))
  {
    BME280_ok  = true ;
    dev_cnt++;
    Serial.println("* Bosch BME280 sensor is connected!");
    Serial.print("Bosch SensorID is: 0x");
    Serial.println(bme280.sensorID(), 16);
    Serial.println("-- BME280 Default setting --");
    Serial.println("normal mode, 16x oversampling for all, filter off,");
    Serial.println("0.5ms standby period");
    Serial.println("Zfilter and Kalman filter enabled!");


    delay(50);
    Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
    //        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    //        Serial.print("        ID of 0x60 represents a BME 280.\n");
    //        Serial.print("        ID of 0x61 represents a BME 680.\n");
    /*
      // indoor navigation
      Serial.println("-- Indoor Navigation Scenario --");
      Serial.println("normal mode, 16x pressure / 2x temperature / 1x humidity oversampling,");
      Serial.println("0.5ms standby period, filter 16x");
      bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X2,  // temperature
                    Adafruit_BME280::SAMPLING_X16, // pressure
                    Adafruit_BME280::SAMPLING_X1,  // humidity
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_0_5 );

      // suggested rate is 25Hz
      // 1 + (2 * T_ovs) + (2 * P_ovs + 0.5) + (2 * H_ovs + 0.5)
      // T_ovs = 2
      // P_ovs = 16
      // H_ovs = 1
      // = 40ms (25Hz)
      // with standby time that should really be 24.16913... Hz
      delayTime = 41;
    */

  }//bmp280


  //9 axis motion init.
  bool ICM_initialized = false;
  while ( !ICM_initialized ) {


    //0 use ICM_20948_I2C_ADDR_AD0  0x68
    ICM20948.begin( Wire, 0 );


    Serial.print( F("-- Initialization of the motion sensor returned: ") );
    Serial.println( ICM20948.statusString() );
    if ( ICM20948.status != ICM_20948_Stat_Ok ) {
      Serial.println( "Trying init. again..." );
      delay(500);
    } else {
      ICM_initialized = true;
      Serial.println( "* 9 axis motion sensor ICM-20948 is initialized!" );
      ICM20948_ok;
      dev_cnt++;
      Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
    }
  }

  Serial.printf("# Sensor initialization finished, total sensor connected %d !\r\n", dev_cnt);
  Serial.println("Setting up Wifi and MQTT now...");
  setup_wifi();
  if (wifi_ok) {
    Serial.printf("Wifi ok! setting MQTT server: %s port 1883\r\n", mqtt_server);
  }
  client.setServer(mqtt_server, 1883);
  client.setCallback(MQTT_RX_callback);
  client.publish("stsmd/SmartDevice_01/info", "device starts now");
  client.publish("strtd/SmartDevice_01/online", "device will go online now");
  // m5mqtt.publish(str('strtd/'+Device_ID+'/online'), "Smart device is online!")
  Serial.println("MQTT sent:strtd/SmartDevice_01/online:device online now");
  Serial.println("Main loop running now...");


}



void loop() {

  //lastMsg= millis();
  blink_status = 1 - blink_status;
  digitalWrite(ledPin, blink_status);

  //M5.update(); //moved to loop end

  if (top_light_ok) {
    lux_BH1750 = Top_LightSensor.GetLightLux();
    //lux_BH1750 = Top_LightSensor.GetLightIntensity();
    Serial.printf("Top light =  %.1f Lux\r\n", lux_BH1750 );
  }

  if (bot_light_ok) {
    lux_max44009 = MAX44009_light.get_lux();
    Serial.printf("Bot light =  %.1f Lux\r\n", lux_max44009 );
  }

  //  if (Gas_EN == 0) {
  //    //disable gas heater
  //    //setGasHeater(uint16_t heaterTemp, uint16_t heaterTime);
  //    bme680.setGasHeater(0, 0);
  //    /* Create a ramp heat waveform in 3 steps */
  //    //gas_sensor.gas_sett.heatr_temp = 320; /* degree Celsius */
  //    //gas_sensor.gas_sett.heatr_dur = 150; /* milliseconds */
  //  }
  //  else {
  //    bme680.setGasHeater(320, 150); // 320*C for 150 ms
  //  }
  //  //bme680_ok = true ;
  //  int Akku_level = m5_power.getBatteryLevel();


  //----BME280------------------


  if (BME280_ok) {
    tmperature = bme280.readTemperature();
    hum = bme280.readHumidity();
    pressure = (float)bme280.readPressure() / 100.0; // in hPA
    Altitude = (float)bme280.readAltitude(SEALEVELPRESSURE_HPA) ;
    Serial.printf("BME280 Temp. = %2.2f°C ; Humidity= %0.2f %%  ;pressure= %0.2f hpa\r\n", tmperature, hum, pressure);
    Serial.printf("Altitude= %6.2f M \r\n", Altitude);

    if (filter_on) {
      //ZFilter
      //T_filter = t_filter.Filter(tmperature);
      P_filter = p_filter.Filter(pressure);
      A_filter = a_filter.Filter(Altitude);
      // kalman filter
      //T_Kfilter = t_kalman.Filter(tmperature);
      P_Kfilter = p_kalman.Filter(pressure);
      A_Kfilter = a_kalman.Filter(Altitude);
      //renew filters
      for (idx = Filter_len - 1; idx > 0; idx--) {
        filter_buf[idx] = filter_buf[idx - 1];
      }
      filter_buf[0] = A_filter;
      //calc average values
      filter_val = 0;
      for (idx = 0; idx < Filter_len; idx++) {
        filter_val = filter_val + filter_buf[idx];
      }
      filter_val = filter_val / Filter_len;
      filter_buf[idx] = A_filter;
      //debug outputs
      //Serial.printf("ZFiltered pressure：%0.2f hpa，  Altitude: %0.2f M \r\n",  P_filter, A_filter);
      //Serial.printf("KalmanFiltered pressure：%0.2f hpa，  Altitude: %0.2f M \r\n",  P_Kfilter, A_Kfilter);
      Serial.printf("Filtered Altitude= %0.2f M \r\n", filter_val);
      //Serial.printf("%0.2f;%0.2f;%0.2f\r\n" ,pressure,P_filter,P_Kfilter); // for test plot
    }


  }



  //  if (dht12_ok) {
  //    tmperature = dht12.readTemperature(); // not very accurate
  //
  //    hum = dht12.readHumidity();
  //    Serial.printf("DHT12 T =  %2.2f°C Humidity= %0.2f %%\r\n", tmperature, hum);
  //  }









  //-----------HP206 ----------------

  if (HP206_ok) {
    //ulong data
    tmperature = (float)HP20x.ReadTemperature() / 100.0; // degree seems not very accurate
    pressure = (float)HP20x.ReadPressure() / 100; // in hPA
    Altitude = (double)HP20x.ReadAltitude2() / 100; // in Meter.Bug fixed!!! 42949636m if P > 1017 mBar  ; 42949648M @ 1016 Hpa
    Serial.printf("HP206: T =  %2.2f*C ，pressure：%0.2f hpa，  Altitude: %6.2f M \r\n", tmperature, pressure, Altitude);
    if (filter_on) {
      //ZFilter
      //T_filter = t_filter.Filter(tmperature);
      P_filter = p_filter.Filter(pressure);
      A_filter = a_filter.Filter(Altitude);
      // kalman filter
      //T_Kfilter = t_kalman.Filter(tmperature);
      P_Kfilter = p_kalman.Filter(pressure);
      A_Kfilter = a_kalman.Filter(Altitude);
      //renew Altitude filter buf
      for (idx = Filter_len - 1; idx > 0; idx--) {
        filter_buf[idx] = filter_buf[idx - 1];
      }
      filter_buf[0] = A_filter;
      //calc average values
      filter_val = 0;
      for (idx = 0; idx < Filter_len; idx++) {
        filter_val = filter_val + filter_buf[idx];
      }
      filter_val = filter_val / Filter_len;
      filter_buf[idx] = A_filter;
      Serial.printf("ZFiltered pressure：%0.2f hpa，  Altitude: %0.2f M \r\n",  P_filter, A_filter);
      Serial.printf("KalmanFiltered pressure：%0.2f hpa，  Altitude: %0.2f M \r\n",  P_Kfilter, A_Kfilter);
      Serial.printf("Filtered Altitude= %0.2f M \r\n", filter_val);
      Serial.printf("%0.2f;%0.2f;%0.2f\r\n" , pressure, P_filter, P_Kfilter); // for test plot
    }


  }


  if (bmp280_ok) {
    pressure = bmp280.readPressure() / 100;
    T_bmp = bmp280.readTemperature();
    Alt_bmp = bmp280.readAltitude();
    Altitude = 8.5 * (1013.25 - pressure); // in meter @ 25 degree
    Altitude = Altitude + (T_bmp - 22) * 0.2273; // compensation for temperature,
    Serial.printf("T_bmp =  %2.2f*C  BMP280 Altitude: %0.2f M  Altitude: %0.2f M\r\n", T_bmp, Alt_bmp, Altitude);

  }
  else if (bme680_ok) {
    pressure = bme680.readPressure() / 100;
    T_bmp = bme680.readTemperature();
    if (Gas_EN)
      Gas = bme680.readGas();
    else Gas = 0;
    Hum_BME680 = bme680.readHumidity();
    Alt_bmp = bme680.readAltitude(1013.25);
    Altitude = 8.5 * (1013.25 - pressure); // in meter @ 25 degree
    Altitude = Altitude + (T_bmp - 22) * 0.2273; // compensation for temperature,
    Serial.printf("T_bme680 =  %2.2f*C Humidity: %0.2f%% Gas: %d Kohms\r\nAir pressure:%0.2fhPa BME680 Altitude: %0.2f M  Altitude: %0.2f M\r\n", T_bmp, Hum_BME680, Gas / 1000, pressure, Alt_bmp, Altitude);
  }


  //Serial.printf("Temperatura: %2.2f*C  Humidity: %0.2f%%  A.Pres.: %0.2fPa\r\n", tmperature, hum, pressure);


  M5.Lcd.setCursor(0, 0);

  M5.Lcd.setTextColor(WHITE, BLACK);

  M5.Lcd.setTextSize(3);

  if  (dht12_ok)
  { Serial.printf("Temperatura: %2.2f*C  Humidity: %0.2f%%  A.Pres.: %0.2fPa\r\n", T_bmp, hum, pressure);
    M5.Lcd.printf("Temp: %2.1f C \r\nHumi: %3.1f%%  \r\nPressure:%0.2fPa\r\nAltitude:%0.1f M\r\n", tmperature, hum, pressure , Altitude);

    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.printf("BMP_T:%0.1f C\r\n", T_bmp);
    M5.Lcd.printf("BMP_Alt:%0.1f M\r\n", Alt_bmp);
  }

  if  (HP206_ok)
  {
    //M5.Lcd.setTextSize(3);
    M5.Lcd.printf("Temp: %2.2f C\r\nPress:%0.2fhPa\r\nAltitude:%.2f M \r\n", tmperature,  pressure , Altitude);

    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setTextSize(2); //size 2 to 8
    M5.Lcd.printf("T.f:%0.2f C\r\n", T_Kfilter);
    M5.Lcd.printf("P.f:%0.2f hpa\r\n", P_Kfilter);
    M5.Lcd.printf("Alt.f:%7.2f M\r\n", A_Kfilter);

    M5.Lcd.setTextSize(3); //size 2 to 8
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.printf("AveA:%0.2f M\r\n", filter_val);

  }
  if  (bme680_ok)
  {
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Temp.:%0.2f C\r\n", T_bmp - bme680_T_offset);
    M5.Lcd.printf("Humidity:%0.1f%% C\r\n", Hum_BME680);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.printf("Pressure:%0.2f Pa\r\n", pressure);
    M5.Lcd.printf("BME_Alt:%0.2f M\r\n", Alt_bmp);

    M5.Lcd.setTextColor(BLUE, BLACK);
    M5.Lcd.printf("Gas: %0.1d Kohms\r\n", Gas / 1000);
  }
  //meter display
  //  M5.Lcd.setTextSize(2); //size 2 to 8
  //  M5.Lcd.setTextColor(LIGHTGREY, BLACK);
  //  M5.Lcd.printf("Base:%0.1f M; Cur:%0.1f M; Dif:%0.1f M \r\n", height_base, height_new, height_diff);
  //
  //
  //
  //  M5.Lcd.setCursor(0, 220);
  //  M5.Lcd.setTextSize(2); //size 2 to 8
  //  M5.Lcd.setTextColor(ORANGE, BLACK);
  //  M5.Lcd.printf("Akku: %3d%%   Run:%d", Akku_level, run_cnt);

  //-------------MCP9808 TA bugs!------------
  if (MCP9808_ok) {
    MCP9808_T = MCP9808.readTemperature();
    Serial.printf("MCP9808 PCB Temperature =  %.2f °C\r\n", MCP9808_T);
    Serial.printf("%%%MCP9808 Temperature raw =  %d  \r\n", MCP9808.readTempRaw());
    //
    MCP9808.shutdown();

  }




  M5.update(); // This function reads The State of Button A and B and C.

  //toggle gas reading
  if (M5.BtnA.wasPressed() ) {
    buttonA_wasPressed();
    //Serial.printf("BtnA was Pressed\r\n");
  }

  //M5.BtnA/B/C. pressedFor(uint32_t ms); If the button is pressed more than the set time – always returns true
  //set LCD brightness
  if (M5.BtnA.pressedFor(250) ) {
    buttonA_longPressed();
    Serial.printf("BtnA was long Pressed\r\n");
  }


  if (Wrover_module)
    delay(time_interval); // total 500ms loop
  //MCP9808.wakeup();   // wake up, delay 250ms; ready to read!
  run_cnt++;
  //delay(100); //100ms
  long now = millis();
  //if (now - lastMsg > 5000) {
  //}
  lastMsg = now;



  if (MQTT_active  ) {
    //MQTT routine
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
  //get motion sensor offsets
  if (2 == run_cnt)
  {
    if ( ICM20948.dataReady() ) {
      ICM20948.getAGMT();
      reset_ACC_offset(ICM20948.agmt);
    }

    Serial.printf("ACC ofsets X:%6.1f mG; Y:%6.1f mG; Z:%6.1f mG\r\n", ACC_X_offset, ACC_Y_offset, ACC_Z_offset);
  }

  if (run_cnt % 5 == 0)
  {
    //-------------MCP9808 bugs!------------
    //    if (MCP9808_ok) {
    //      MCP9808_T = MCP9808.readTemperature();
    //      Serial.printf("MCP9808 PCB Temperature =  %.2f °C\r\n", MCP9808_T);
    //      Serial.printf("%%%MCP9808 Temperature raw =  %d  \r\n", MCP9808.readTempRaw());
    //      //
    //       MCP9808.shutdown();
    //
    //    }
    //-------------motion sensor test------------
    if ( ICM20948.dataReady() ) {
      ICM20948.getAGMT();                // The values are only updated when you call 'getAGMT'
      //    printRawAGMT( ICM20948.agmt );     // Uncomment this to see the raw values, taken directly from the agmt structure
      printScaledAGMT( ICM20948.agmt);   // This function takes into account the sclae settings from when the measurement was made to calculate the values with units
      Vibration = calc_vibration(ICM20948.agmt);
      Serial.printf("Device Vibration:%6.1f mG\r\n", Vibration);

      //delay(30);
    }

  }



  if (run_cnt % 10 == 0) {


    Serial.printf("MQTT status: %d (0 means MQTT_CONNECTED )\r\n", client.state());

    if (MQTT_active  ) {
      Serial.printf("MQTT try sending data now...\r\n");
      //    char buf1[10] = "string1";
      //    char buf2[10] = "string2";
      //
      //    strcpy(myNewCombinedArray, buf1);
      //    strcat(myNewCombinedArray, ";");
      //    strcat(myNewCombinedArray, buf2);


      // Convert the value to a char array
      //char humString[8];
      //char *dtostrf(double val, signed char width, unsigned char prec, char *s)
      dtostrf(lux_max44009, 6, 1, msg);
      Serial.printf("MQTT raw Msg 1:%s\r\n", msg);
      client.publish("stsmd/SmartDevice_01/light_B", msg);
      Serial.printf("MQTT Msg 1 sent: stsmd/SmartDevice_01/light: %.1f Lux\r\n", lux_max44009);

      //      dtostrf(MCP9808_T, 6, 2, msg);
      //      Serial.printf("MQTT raw Msg 2:%s\r\n", msg);
      //      client.publish("stsmd/SmartDevice_01/T_PCB", msg);

      //data_payload = Data_ava_flag+str("[")+str(T)+str(",")+str(H)+str(",")+str(A)+str(",")+str(Vibration)+str(",")+str(BH_data)+str("] ")
      sprintf(MQTT_payload, "%c", Data_ava_flag);
      //strcpy(MQTT_payload, Data_ava_flag);
      strcat(MQTT_payload, "[");
      //T
      dtostrf(tmperature, 3, 2, msg);
      strcat(MQTT_payload, msg);
      strcat(MQTT_payload, ",");
      //H
      dtostrf(hum, 2, 2, msg);
      strcat(MQTT_payload, msg);
      strcat(MQTT_payload, ",");
      //A
      dtostrf(pressure, 4, 2, msg);
      strcat(MQTT_payload, msg);
      strcat(MQTT_payload, ",");

      //vibration 
      dtostrf(Vibration, 6, 2, msg);
      strcat(MQTT_payload, msg);
      strcat(MQTT_payload, ",");
      //strcat(MQTT_payload, "0,"); //dummy data for test
      //light
      dtostrf(lux_max44009, 6, 1, msg);
      strcat(MQTT_payload, msg); //light
      strcat(MQTT_payload, "]");

      Serial.printf("MQTT Msg: %s: %s \r\n", MQTT_SensorMsg_head, MQTT_payload);
      client.publish(MQTT_SensorMsg_head, MQTT_payload);

      sprintf(tmp_string, "%d", run_cnt);
      client.publish("strtd/SmartDevice_01/cycle", tmp_string);
      //Serial.printf("MQTT Msg: %s: %s \r\n", MQTT_SensorMsg_head, MQTT_payload);
      //combine msg data
      //Data_ava_flag  0b1000111
      //data_payload = Data_ava_flag+str("[")+str(T)+str(",")+str(H)+str(",")+str(A)+str(",")+str(Vibration)+str(",")+str(BH_data)+str("] ")
      //printf.(msg);
      /*
          long now = millis();
          if (now - lastMsg > 2000) {
            lastMsg = now;
            ++value;
            snprintf (msg, 50, "hello world #%ld", value);
            Serial.print("Publish message: ");
            Serial.println(msg);
            client.publish("outTopic", msg);
      */
    }
  }//end run_cnt/10 MQTT

  if (run_cnt % 20 == 0) {

    //Serial.printf("MQTT status: %d (0 means MQTT_CONNECTED; -1 means MQTT_DISCONNECTED)\r\n", client.state());

    if (MQTT_active  ) {
      //Serial.printf("MQTT try sending global data now...\r\n");
      Serial.printf("&&&MQTT global Msg: %s: %s \r\n", MQTT_GlobalMsg_head, MQTT_payload);
      client.publish(MQTT_GlobalMsg_head, MQTT_payload);
    }
  }//end run_cnt/20 MQTT global
  Serial.printf("System run count: %d  Loop time cost: %d ms\r\n", run_cnt, now - lastMsg);

  Serial.println("---------------------------");



}// end loop









// Below here are some helper functions to print the data nicely!

void printPaddedInt16b( int16_t val ) {
  if (val > 0) {
    Serial.print(" ");
    if (val < 10000) {
      Serial.print("0");
    }
    if (val < 1000 ) {
      Serial.print("0");
    }
    if (val < 100  ) {
      Serial.print("0");
    }
    if (val < 10   ) {
      Serial.print("0");
    }
  } else {
    Serial.print("-");
    if (abs(val) < 10000) {
      Serial.print("0");
    }
    if (abs(val) < 1000 ) {
      Serial.print("0");
    }
    if (abs(val) < 100  ) {
      Serial.print("0");
    }
    if (abs(val) < 10   ) {
      Serial.print("0");
    }
  }
  Serial.print(abs(val));
}

void printRawAGMT( ICM_20948_AGMT_t agmt) {
  Serial.print("RAW. Acc [ ");
  printPaddedInt16b( agmt.acc.axes.x );
  Serial.print(", ");
  printPaddedInt16b( agmt.acc.axes.y );
  Serial.print(", ");
  printPaddedInt16b( agmt.acc.axes.z );
  Serial.print(" ], Gyr [ ");
  printPaddedInt16b( agmt.gyr.axes.x );
  Serial.print(", ");
  printPaddedInt16b( agmt.gyr.axes.y );
  Serial.print(", ");
  printPaddedInt16b( agmt.gyr.axes.z );
  Serial.print(" ], Mag [ ");
  printPaddedInt16b( agmt.mag.axes.x );
  Serial.print(", ");
  printPaddedInt16b( agmt.mag.axes.y );
  Serial.print(", ");
  printPaddedInt16b( agmt.mag.axes.z );
  Serial.print(" ], Tmp [ ");
  printPaddedInt16b( agmt.tmp.val );
  Serial.print(" ]");
  Serial.println();
}


void printFormattedFloat(float val, uint8_t leading, uint8_t decimals) {
  float aval = abs(val);
  if (val < 0) {
    Serial.print("-");
  } else {
    Serial.print(" ");
  }
  for ( uint8_t indi = 0; indi < leading; indi++ ) {
    uint32_t tenpow = 0;
    if ( indi < (leading - 1) ) {
      tenpow = 1;
    }
    for (uint8_t c = 0; c < (leading - 1 - indi); c++) {
      tenpow *= 10;
    }
    if ( aval < tenpow) {
      Serial.print("0");
    } else {
      break;
    }
  }
  if (val < 0) {
    Serial.print(-val, decimals);
  } else {
    Serial.print(val, decimals);
  }
}

void printScaledAGMT( ICM_20948_AGMT_t agmt) {
  Serial.print("Scaled. Acc (mg) [ ");
  printFormattedFloat( ICM20948.accX(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( ICM20948.accY(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( ICM20948.accZ(), 5, 2 );
  Serial.print(" ], Gyr (DPS) [ ");
  printFormattedFloat( ICM20948.gyrX(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( ICM20948.gyrY(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( ICM20948.gyrZ(), 5, 2 );
  Serial.print(" ], Mag (uT) [ ");
  printFormattedFloat( ICM20948.magX(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( ICM20948.magY(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( ICM20948.magZ(), 5, 2 );
  Serial.print(" ], Tmp (C) [ ");
  printFormattedFloat( ICM20948.temp(), 5, 2 );
  Serial.print(" ]");
  Serial.println();
}

void printScaledAcc( ICM_20948_AGMT_t agmt) {
  Serial.print("Scaled. Acc (mg) [ ");
  printFormattedFloat( ICM20948.accX(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( ICM20948.accY(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( ICM20948.accZ(), 5, 2 );
  Serial.print(" ]");
  //Serial.print(" ], Gyr (DPS) [ ");

  Serial.println();
}

float calc_vibration(ICM_20948_AGMT_t agmt) {

  //      ACC_X = imu0.acceleration[0]-ACC_X_offset
  //      ACC_Y = imu0.acceleration[1]-ACC_Y_offset
  //      ACC_Z = imu0.acceleration[2]
  //      ACC_total = (ACC_X**2+ACC_Y**2+ACC_Z**2)**0.5
  //      Vibration = (ACC_X**2+ACC_Y**2+(ACC_Z-ACC_Z_offset)**2)**0.5
  //      ACC_hor = (ACC_X**2+ACC_Y**2)**0.5
  //      ACC_X_offset = 0
  //      ACC_Y_offset = 0
  //      ACC_Z_offset = 0

  float ACC_X = ICM20948.accX();
  float ACC_Y = ICM20948.accY();
  float ACC_Z = ICM20948.accZ();
  //float ACC_total = (ACC_X ^ 2 + ACC_Y ^ 2 + ACC_Z ^ 2) ^ 0.5;
  float Vibration = (  sq(ACC_X - ACC_X_offset) + sq(ACC_Y - ACC_Y_offset) + sq(ACC_Z - ACC_Z_offset) ) ;
  Serial.printf("$$$ Vib calc1: %f \r\n",Vibration);
  Vibration = sqrt(Vibration);
  Serial.printf("$$$ Vib calc2: %f \r\n",Vibration);
  return Vibration;
}

void reset_ACC_offset(ICM_20948_AGMT_t agmt) {

  //      ACC_X = imu0.acceleration[0]-ACC_X_offset
  //      ACC_Y = imu0.acceleration[1]-ACC_Y_offset
  //      ACC_Z = imu0.acceleration[2]
  //      ACC_total = (ACC_X**2+ACC_Y**2+ACC_Z**2)**0.5
  //      Vibration = (ACC_X**2+ACC_Y**2+(ACC_Z-ACC_Z_offset)**2)**0.5
  //      ACC_hor = (ACC_X**2+ACC_Y**2)**0.5
  //      ACC_X_offset = 0
  //      ACC_Y_offset = 0
  //      ACC_Z_offset = 0

  ACC_X_offset = ICM20948.accX();
  ACC_Y_offset = ICM20948.accY();
  ACC_Z_offset = ICM20948.accZ();

}
