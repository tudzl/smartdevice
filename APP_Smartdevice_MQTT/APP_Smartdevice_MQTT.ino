
/*  Smart device Main PCB code for ESP32 wrover and 6 sensors
    this code is working as APP launcher compatibale
    Version 6.1 MQTT working!  2019.11.8
    Version 6.0 Light , BME280 working!  2019.11.1
    Version 5.0  Add altitude base , offset diff measure function
    Version 4.0  HP206C bug fixed!  altitude: ulong changed to long
    Version 3.1  Zfilter and kalman filter works!
    Version 2.2  HP206C works!
    need to improve BME680 T reading accuracy!
    m5stack fire arduino device test app for ENV unit and BME680 module +HP206C  module
    Author ling zhou, 16.8.2019
    note: ONboard sensors: BME280+ICM-20948+MCP9808T-E/MS+BH1750+MAX44009EDT+analog Mic ICS-40181

    note: need add library Adafruit_BME680 from library manage

*/


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
//For #include "filename" the preprocessor searches first in the same directory as the file containing the directive, and then follows the search path used for the #include <filename> form. This method is normally used to include programmer-defined header files.

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
//DHT address:  0x5C
MAX44009 MAX44009_light;  //bot light sensor
DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
BH1750FVI::eDeviceMode_t BH_DEVICEMODE = BH1750FVI::k_DevModeContHighRes2;
BH1750FVI Top_LightSensor(BH_DEVICEMODE);
Adafruit_BME680 bme680; // I2C  0x76
Adafruit_BMP280 bmp280;  // I2C  0x76
Adafruit_BME280 bme280;  // I2C  0x77
HP20x_dev HP206;  // I2C  0x76
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

unsigned int HP206_DSR = 256; // down sampling rate
float hum = 0;
float tmperature = 0;
float bme680_T_offset = 5;
unsigned int Gas = 0;
bool Gas_EN = true;
float Hum_BME680 = 0;
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
bool Wrover_module = true  ;
unsigned long run_cnt = 0;
unsigned int time_interval = 200; //cycle time
//---sys run vars---
unsigned char dev_cnt = 0;

//------ Height meter vars
float height_base =  0;
float height_new =  0;
float height_diff =  0;


// Blink ledPin refers to smart device ESP32 GPIO 2
const int ledPin = 2;
unsigned char blink_status = 1;




//for bme680 only, toggle gas measurrement
void buttonA_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  Serial.println("buttonA/S1 was Pressed");
  //Gas_EN = !Gas_EN;

}

//set the lcd Brightness
void buttonA_longPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  //M5.lcd.setBrightness(30);

  Serial.println("buttonA/S1 is longPressed");
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

//********************MQTT****************************
// Replace the next variables with your SSID/Password combination
const char* ssid = "TC";
const char* password = "sthz@2020";
const char* ssid2 = "WH10";
const char* password2 = "Zell9090";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "st.devmolv.com";
const char* device_ID = "SmartDevice_01";
//Strings are actually one-dimensional array of characters terminated by a null character '\0'.
const char* MQTT_SensorMsg_head = "stsmd/SmartDevice_01/global";
char* MQTT_payload ;
const char  Data_ava_flag = B1001111 ;

WiFiClient Smartdevice_01;
PubSubClient client(Smartdevice_01);

long lastMsg = 0;
char msg[64];
char tmp_string[32];
int value = 0;


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Wifi try Connecting to ");
  Serial.println(ssid2);

  //WiFi.begin(ssid, password);
  WiFi.begin(ssid2, password2);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi is now connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
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
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  dev_cnt = 0;





  Serial.println(F("<<<Smart device V2 sensor test>>>"));

  if (Top_LightSensor.begin2(Bh1750_add))
  {
    dev_cnt++;
    top_light_ok = true;
    Serial.println("* Top light sensor BH1750 is connected!");
    delay(50);
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
  }

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

  }


  Serial.println("Setting up Wifi and MQTT now...");
  setup_wifi();
  Serial.printf("Wifi ok! setting MQTT server: %s port 1883\r\n", mqtt_server);
  client.setServer(mqtt_server, 1883);
  client.setCallback(MQTT_RX_callback);
  client.publish("stsmd/Smartdevice_01/info", "device online now");
  Serial.println("MQTT sent:stsmd/Smartdevice_01/info:device online now");
  Serial.println("Main loop running now...");


}



void loop() {


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
    Serial.printf("BME280 Temp. = %2.2f*C ; Humidity= %0.2f %%  ;pressure= %0.2f hpa\r\n", tmperature, hum, pressure);
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
  //    Serial.printf("DHT12 T =  %2.2f*C Humidity= %0.2f %%\r\n", tmperature, hum);
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

  M5.update(); // This function reads The State of Button A and B and C.

  //toggle gas reading
  if (M5.BtnA.wasPressed() ) {
    buttonA_wasPressed();
    Serial.printf("BtnA was Pressed\r\n");
  }

  //M5.BtnA/B/C. pressedFor(uint32_t ms); If the button is pressed more than the set time – always returns true
  //set LCD brightness
  if (M5.BtnA.pressedFor(200) ) {
    buttonA_longPressed();
    Serial.printf("BtnA was long Pressed\r\n");
  }


  if (Wrover_module)
    delay(200); //100ms

  run_cnt++;
  //delay(100); //100ms
  Serial.printf("System run count: %d\r\n", run_cnt);
  Serial.println("---------------------------");
  //MQTT routine
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (run_cnt % 5 == 0) {

    
    Serial.printf("MQTT status: %d\r\n",client.state());
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
    client.publish("stsmd/Smartdevice_01/local", msg);
    Serial.printf("MQTT Msg 1 sent: stsmd/Smartdevice_01/local: %.1f Lux\r\n", lux_max44009);

    //-----------
    /*
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

      //vib need add real vals later
      //dtostrf(pressure, 4, 2, msg);
      //strcat(MQTT_payload, msg);
      //strcat(MQTT_payload, ",");
      strcat(MQTT_payload, "0,");
      //light
      dtostrf(lux_max44009, 6, 1, msg);
      strcat(MQTT_payload, msg); //light
      strcat(MQTT_payload, "]");
    */
    //Serial.printf("MQTT Msg: %s: %s \r\n", MQTT_SensorMsg_head, MQTT_payload);
    //client.publish(MQTT_SensorMsg_head, MQTT_payload);
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


}
