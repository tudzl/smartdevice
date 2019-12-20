
/*  Smart device Main PCB code for ESP32 wrover and 6 sensors
    this code is working as APP launcher compatibale 2019.12.17
    Problem list： To add WiFiMQTTManager.h( related esp8266 lib is problemmatic!
    TO DO TASK:  3. ESP32-->PIC UART communication! 4. Max30205 limit alert setting function
    Doing now: 3.ESP32-->PIC UART communication
    Version 8.5 MQTTT wifi config and store para is OK! wifi check and roll back to def 4 if failed is tested OK!
    Version 8.3 MQTT last will active！ need test, need add new wifi failed, then switch to def 4
    Version 8.2 MQTT wifi config and store para info is done！ need test and improve
    Version 8.1 MQTT Ta data source sellect from max30205, BME280, max30205  bug fixed!
    Version 8.0 MQTT Ta data source sellect from max30205, BME280, mlx90614 mode, max30205 have bugs for config regs
    Version 7.1 found bug with max44009, lux top at 2937.6 lux, bug fixed change to auto mode
    Version 7.0 added 4. wifi lost , reconnect seems ok!
    Version 6.9 RFID reset bug fixed! Final version/beta version
    Version 6.8 RFID is working! Final version/beta version
    Version 6.7 MLX90614 is working!
    Version 6.6 changed mqtt msg format to new api 2019.11.26
    Version 6.5 add ICM 20948 sensor function 2019.11.25
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


/***********************ICM20948 9-axis motion sensor*****************************************
   Example1_Basics.ino
   ICM 20948 Arduino Library Demo
   Use the default configuration to stream 9-axis IMU data
   Owen Lyke @ SparkFun Electronics
   Original Creation Date: April 17 2019

   This code is beerware; if you see me (or any other SparkFun employee) at the
   local, and you've found our code helpful, please buy us a round!

   Distributed as-is; no warranty is given.
 ***************************************************************/

//SPI flash system, need run format code first!
#include "FS.h"
#include "FFat.h"
#define FORMAT_FFAT false // You only need to format FFat the first time you run a test
// This file should be compiled with 'Partition Scheme' (in Tools menu)
// set to '16M Flash (3MB APP/9MB FATFS)'  for smart device 16M wrover module



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
#include "Protocentral_MAX30205.h" //body T sensor
//For #include "filename" the preprocessor searches first in the same directory as the file containing the directive, and then follows the search path used for the #include <filename> form. This method is normally used to include programmer-defined header files.
//#include "ICM_20948.h"  // old api ;  Click here to get the library: http://librarymanager/All#SparkFun_ICM20948_IMU
#include <Mahony_DPEng.h>
#include <Madgwick_DPEng.h>
#include <DPEng_ICM20948_AK09916.h>
//NCIR
#include <Adafruit_MLX90614.h>
//RFID
#include "MFRC522_I2C.h"


//device MAC: 24:6f:28:f3:a3:a4
//WIFI+MQTT
#define MQTT_KEEPALIVE 30
#include <WiFi.h>
#include <PubSubClient.h>  //MQTT
//#include <WiFiMQTTManager.h> //wifi config save, need ArduinoJson , WiFiManager
//mqtt.subscribe(str('stsmd/'+Device_ID+'/alert')
//test_ESP32_smartdevice

#define wifi_connectto_default false // use true for new device installation, if false ,will load config info from internal flash file system
const char* device_ID = "SmartDevice_zl";
const char* Combitac_ID_MA = "CT34.0090-P"; //PCB
const char* Combitac_ID_FE = "CT34.0090-S";

//ESP32---> PIC24 uart
uint16_t PIC_framehead = 0xAA24 ;
uint16_t PIC_frameend = 0xFF90 ;


const float BME280_Hum_offset = 11.2 ; // calibration with Fluke 971, 2019.12.4 for #5 PCB

#define SEALEVELPRESSURE_HPA (1013.25)
//bmp280 address:  0x76
//HP206C address:  0x76
//BME680 address:  0x76-->0x77:
unsigned char Bh1750_add = 0x23;
unsigned char BME280_add = 0x77;
unsigned char BME680_add = 0x77;
unsigned char hp206_add = 0x76;
unsigned char bmp280_add = 0x76;
unsigned char MAX44009_add = 0x4B; //light bot
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
MAX30205 MAX30205_sensor;
//ICM20948_I2C def
//ICM_20948_I2C ICM20948 ; //9-axis motion
Adafruit_MLX90614 MLX90614 = Adafruit_MLX90614();
// 0x28 is mfrc522 i2c address. Check your address with i2cscanner if not match.
MFRC522 mfrc522(0x28);   // Create MFRC522 instance.

POWER m5_power;
//global vars
float lux_BH1750 = 0;
float lux_max44009 = 0;





float pressure = 0;
float hum = 0;

float tmperature = 0;
unsigned char tmperature_source = 0; //0  for bme280, 1 for max30205
float T_MLX_obj = 0; //NCIR T
float T_MLX_self = 0;
float T_bmp = 0;
float Alt_bmp = 0;
double T_max30205 = 0;
unsigned int MAX30205_Range_H = 50;
unsigned int MAX30205_Range_L = 0;
float MAX30205_TOV = 27;
float MAX30205_HYST = 26;
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
bool MCP9808_ENA = false; // enable or disable MCP9808 sensor
bool ICM20948_ok = false;
bool MLX90614_ok = false;
bool mfrc522_ok = false;
bool MAX30205_ok = false;
//bool FFat
char TAG_NFC [9]; // 8 bytes for TAG UID

//--------------IMU----------------
const float Gravity_HZ = 9.7936 ;
const float Gravity_0 = 9.80665 ;
float Linear_ACC = 0 ;
float Linear_ACC_max = 0;
float Linear_ACC_max_last = 0; // for file writting
float Tilt_A = 0;
float ACC_X, ACC_Y, ACC_Z;
// Create sensor instance.
DPEng_ICM20948 ICM20948_dpEng = DPEng_ICM20948(0x948A, 0x948B, 0x948C);

bool yaw_compensation_EN = true ;
float yaw_offsets = 24.7 ; //added by zl ,offsets between ecompass heading and physical compass north
// Mag calibration values are calculated via ahrs_calibration example sketch results（MotionCal.exe）.
// These values must be determined for each baord/environment.
// See the image in this sketch folder for the values used
// below.

// Offsets applied to raw x/y/z mag values， obtained from MotionCal.exe
float mag_offsets[3]            = { -16.8F, -13.68F, 10.33F };  // for 1# PCB smart device v2 ICM-20948  2019.11.18

// Soft iron error compensation matrix ，  obtained from MotionCal.exe
float mag_softiron_matrix[3][3] = {
  {  1.000,  -0.002,  0.006 },
  {  -0.002,  1.014,  0.002 },
  {  0.006,   0.002,  0.996 }
}; // for 1# PCB smart device v2 ICM-20948  2019.11.18

//mag_field_strength seems not used in th program
float mag_field_strength        = 33.43F;  // for 1# PCB smart device v2 ICM-20948  2019.11.18

// Offsets applied to compensate for gyro zero-drift error for x/y/z
float gyro_zero_offsets[3]      = { 0.0F, 0.0F, 0.0F };

// Mahony is lighter weight as a filter and should be used
// on slower systems
Mahony_DPEng IMU_filter;
//Madgwick_DPEng IMU_filter;  //fast cpu







unsigned int HP206_DSR = 256; // down sampling rate


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
const float Gravity = 9.8; //The value 9.8 m/s² is valid for the object at the surface of earth (at sea level).
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
bool MQTT_active = true;  // default status after reboot
//------ Height meter vars
float height_base =  0;
float height_new =  0;
float height_diff =  0;

//----------PCB related paras-----------
// Blink ledPin refers to smart device ESP32 GPIO 2
const int ledPin = 2;
unsigned char blink_status = 1;

//********************MQTT****************************
// Replace the next variables with your SSID/Password combination
bool wifi_ok = false;
const char* ssid = "TC";
const char* password = "sthz@2020";
const char* ssid2 = "WH10";
const char* password2 = "smartdevice";
const char* ssid3 = "HUAWEI"; //test only
const char* password3 = "smartdevice";
const char* ssid4 = "Staubli_SD"; //for ASWIL test only
const char* password4 = "smartdevice";
char* ssid_pre = "null";
char*  password_pre = "null";
char ssid_new [16];
char password_new [16];
const char wifi_config_num = 4; //1 to 4 for pre defigned configs, 0 for previous SPI flash stored config, which is received by MQTT
char wifi_config = 4 ;
// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "st.devmolv.com";
const char* mqtt_lastwillTopic = "stsmd/CT34.0090-P/info";
const char* mqtt_lastwillMsg = "offline";
//server back control related
const char* mqtt_server_CMD_topic = "Smartdevice_server_CMD/+"; //subscribe to this topic
const char* mqtt_server_CMD_ssid = "Smartdevice_server_CMD/config_ssid";
const char* mqtt_server_CMD_password = "Smartdevice_server_CMD/config_password";
const char* mqtt_server_CMD_status = "Smartdevice_server_CMD/status";
//device talk to server
const char* MQTT_Info_head =  "stsmd/CT34.0090-P/info"; //for new server api
const char* MQTT_data_head =  "stsmd/CT34.0090-P/data";
const char* MQTT_SensorMsg_head = "stsmd/CT34.0090-P/local"; //for old api
const char* MQTT_GlobalMsg_head = "stsmd/CT34.0090-P/global"; //for old api
char MQTT_payload[128] ;
const char  Data_ava_flag = B1001111 ; //ori 1000111
//Strings are actually one-dimensional array of characters terminated by a null character '\0'.


WiFiClient SmartDevice_zl;
PubSubClient client(SmartDevice_zl); //using name from above wificlient

long lastMsg = 0;
char msg[32];
char tmp_string[32];
//int value = 0;
long now = 0;

//Timer1 for wifi check
long TMR1_start = 0;
long TMR1_stop = 0;
long TMR1_interval = 10;  //10s


//short press Btn to reset IMU offsets
void buttonA_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  Serial.println(" !--> buttonA/S1 was Pressed");
  //Serial.println("##reset motion sensor offset now！");
  //Gas_EN = !Gas_EN;
  //  if ( ICM20948.dataReady() ) {
  //    ICM20948.getAGMT();
  //    reset_ACC_offset(ICM20948.agmt);
  //  }
  //
  //  Serial.printf("ACC ofsets X:%6.1f mG; Y:%6.1f mG; Z:%6.1f mG\r\n", ACC_X_offset, ACC_Y_offset, ACC_Z_offset);
  Serial.println("##reset Recorded Linear Acc Max to Zero！");
  Linear_ACC_max_last = 0;
  Serial.println("##Recording new max Linear Acc value...");
  Record_max_G();

}


//long press Btn > 1s to Disconnect wifi
void buttonA_longPressed2(void) {
  //M5.Speaker.beep();  // too laud
  Serial.println("");
  Serial.println(" --^^^--- buttonA/S1 is longPressed over 1s !!!");
  Serial.println("");
  Serial.println(" --^^^--- Disconnect wifi now to save power !!!");
  WiFi.disconnect(true);
  Serial.println(WiFi.localIP());
  delay(500);
}

//long press Btn 2s to connect wifi
void buttonA_longPressed3(void) {
  //M5.Speaker.beep();  // too laud
  Serial.println("");
  Serial.println(" --^^^--- buttonA/S1 is longPressed over 2s !!!");
  Serial.println("");
  Serial.println(" --^^^--- connect wifi now  !!!");
  WiFi.disconnect(false);
  setup_wifi();
  delay(50);
  Serial.println(WiFi.localIP());
  delay(500);
}
//long press Btn > 500ms to enable/disable MQTT , send online/offline info, need improve
void buttonA_longPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  //M5.lcd.setBrightness(30);

  Serial.println("");
  Serial.println(" --^^^--- buttonA/S1 is longPressed over 0.5s !!!");
  Serial.println("");
  Serial.printf(" ## MQTT_active: %d", MQTT_active);
  if (true == MQTT_active) {
    Serial.printf("-->:Smart device MQTT turn OFF now...\r\n");
    client.publish(MQTT_Info_head, "Device MQTT is turning off...");
    delay(50);
    client.publish(MQTT_Info_head, "offline");
    delay(50);
    //client.publish("strtd/CT34.0090-P/online", "");
  }
  else {// need improve
    Serial.printf("-->:Smart device MQTT go Online now!\r\n");
    client.publish(MQTT_Info_head, "online");
    delay(50);
    //client.publish("strtd/CT34.0090-P/online", "now online!");
  }
  MQTT_active = 1 - MQTT_active;
  Serial.println("");
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
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Wifi functions%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//void connect_wifi() is to connect and verify the wifi ssid PW ok or not,  called in loop(), MQTT back control function ralted.
void connect_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Wifi try Connecting to ");



  Serial.println(ssid_new);
  WiFi.begin(ssid_new, password_new);


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
    wifi_config = 0 ;
    Serial.println("WiFi config set to 0 (new received configeration)!");
    writeFile(FFat, "/Wifi_config_Nr.txt", 0);
    Serial.println("WiFi config save to Wifi_config_Nr.txt !");


  }
  else  {
    wifi_ok = false;
    WiFi.disconnect(true);
    Serial.println("#connectting wifi failed");
    //char * tmpmsg= wifi_config_num;
    writeFile(FFat, "/Wifi_config_Nr.txt", &wifi_config_num);
    Serial.printf("WiFi config restored to default setting %d !\r\n", wifi_config_num);
  }

  Zeit = float(millis() - Zeit_anfang) / 1000.0f;
  //Zeit = Zeit/1000 ; // to S

  Serial.printf("#Setting up wifi time passed: %.2f seconds", Zeit);
  Serial.println("");
}

//void setup_wifi() is to connect to wifi AP for the first time after system booting, called first in the end of setup();
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
  { Serial.println(ssid2);
    WiFi.begin(ssid2, password2);
  }
  else if (3 == wifi_config)
  { Serial.println(ssid3);
    WiFi.begin(ssid3, password3);
  }
  else if (4 == wifi_config)
  { Serial.println(ssid4);
    WiFi.begin(ssid4, password4);
  }
  else if (0 == wifi_config) // for connection to new applied/ stored wifi configurations
  { //read back last time wifi config ssid_pre  bugs!
    readFilestring(FFat, "/Wifi_ssid.txt", ssid_new);
    //ssid_pre = readFile(FFat, "/Wifi_ssid.txt");;
    //password_pre = readFile(FFat, "/Wifi_password.txt");;
    readFilestring(FFat, "/Wifi_password.txt", password_new);
    Serial.print("Wifi try Connecting to ");
    Serial.println(ssid_new);
    WiFi.begin(ssid_new, password_new);
  }
  else
  { Serial.println(ssid4);
    WiFi.begin(ssid4, password4);
  }

  long Zeit_anfang = millis();
  long now;
  float Zeit;
  Serial.printf("sys time Zeit_anfang: %d ms \r\n", Zeit_anfang);


  //waiting for connection establishment!
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    now = millis();
    //Zeit = now -Zeit_anfang;
    Serial.println("");
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
    //added 2019.12.20, in case  config 0 failed( after reloaction of the device?), switch back to def config 4
    wifi_config = wifi_config_num;
  }

  Zeit = float(millis() - Zeit_anfang) / 1000.0f;
  //Zeit = Zeit/1000 ; // to S

  Serial.printf("#Setting up wifi time passed: %.2f seconds", Zeit);
  Serial.println("");
}

void MQTT_RX_callback(char* topic, byte* message, unsigned int length) {
  Serial.println("");
  Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message content: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println("");

  // Feel free to add more if statements to control more GPIOs with MQTT
  //-----wifi config via MQTT CMD
  if (String(topic) == mqtt_server_CMD_ssid) {//"Smartdevice_server_CMD/config_ssid"
    client.publish(MQTT_Info_head, "new SSID Msg received by device");
    Serial.printf("Changing wifi ssid to: %s \r\n", String(messageTemp));
    sprintf(ssid_new, "%s", messageTemp);
    //ssid_new = messageTemp;
    //--------------store in spi flash
    writeFile(FFat, "/Wifi_ssid.txt", ssid_new);
  }
  if (String(topic) == mqtt_server_CMD_password) {//"Smartdevice_server_CMD/config_password"
    client.publish(MQTT_Info_head, "new PW Msg received by device");
    Serial.printf("Changing wifi password to %s", String(messageTemp));
    sprintf(password_new, "%s", messageTemp);
    Serial.println("");
    //password_new = messageTemp;
    writeFile(FFat, "/Wifi_password.txt", password_new);
    //disconnect first
    Serial.println("Device will now disconnect current WiFi and try to connect to new WiFi...");
    WiFi.disconnect(true);

    delay (100);
    connect_wifi(); //if ok, will set wifi_config to 0
  }



  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == mqtt_server_CMD_status) {
    Serial.print("Changing status to ");
    client.publish(MQTT_Info_head, "new status Msg received by device");
    if (messageTemp == "on") {
      Serial.println("on");
      //digitalWrite(ledPin, HIGH);
      //talk to pic
      Serial.print(PIC_framehead);
      Serial.print(1);
      Serial.print(PIC_frameend);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      //digitalWrite(ledPin, LOW);
      Serial.print(PIC_framehead);
      Serial.print(2);
      Serial.print(PIC_frameend);
    }

    Serial.println("");
    Serial.println("#########-ESP32 talk to pic24 finish---$$$$$$$$$$$$");
  }
  Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
  Serial.println("");
  delay (1000); // for better terminal display
}


//MQTT server reconnect,In the  MQTT_reconnect() function, you can subscribe to MQTT topics. In this case, the ESP32 is only subscribed to the test_ESP32_smartdevice_server:
void MQTT_reconnect() {
  // Loop until we're reconnected
  unsigned int cnt = 0 ;
  while (!client.connected()) {
    Serial.print("-->:Attempting MQTT connection...");

    if ( cnt > 4)
      break; //time out quite


    //const char* mqtt_lastwillTopic = "stsmd/CT34.0090-P/info";
    //const char* mqtt_lastwillMsg = "offline";
    // Attempt to connect，
    // if (client.connect(device_ID)) { //oir code without last will
    //MQTT connect to server with last will, but not retained!
    //boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    if (client.connect(device_ID, mqtt_lastwillTopic, MQTTQOS2, false, mqtt_lastwillMsg) ) {
      Serial.println("connected");
      //send online status
      client.publish(MQTT_Info_head, "online");
      delay(20);
      // Subscribe
      client.subscribe(mqtt_server_CMD_topic); //need change?
      //client.subscribe("config_ssid"); //need change
      //client.subscribe("config_password"); //need change
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      cnt++;
    }
  }
}
void setup() {


  //void M5Stack::begin(bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable)
  M5.begin();

  Wire.begin();
  pinMode(ledPin, OUTPUT);
  wifi_config = wifi_config_num;

  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load Wifi config_1, default is config_2");
    wifi_config = 1;
    //Serial.println("Will Load menu binary");
    //updateFromFS(SD);
    //ESP.restart();
  }
  dev_cnt = 0;





  Serial.println(F("<<<Smart device V2 sensor MQTT test>>>"));
  Serial.println(F("<<<Firmware Version 7.0, ling zhou, 10.12.2019>>>"));
  Serial.printf("<<<----Device ID: %s----->>>\r\n", device_ID);
  delay(50);
  if (Top_LightSensor.begin2(Bh1750_add))
  {
    dev_cnt++;
    top_light_ok = true;
    Serial.println("* Top light sensor BH1750 is connected!");
    Serial.println("  BH1750 DeviceMode: H-resolution mode2(0.5lux resolution)!");
    Serial.println("  Range: 0.5 - 65535 lux, 120ms responsive time!");
    delay(50);
    Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
  }

  if (MLX90614.begin2()) {
    //MLX90614 chip ID readout: 0  bugs?

    MLX90614_ok = true;
    dev_cnt++;
    //Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
    Serial.println("* NCIR MLX90614 sensor is connected!");
    Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
  }
  else {
    Serial.println("## NCIR sensor is not connected!");
  }


  if (MCP9808.begin(MCP9808_add)) {
    MCP9808_ok = true;
    dev_cnt++;
    //MCP9808.shutdown();
    //MCP9808.wakeup();   // wake up, delay 250ms; ready to read!
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
    Serial.println("  Max44009 spec:  Wide 0.045 Lux to 188,000 Lux Range");
    Serial.println("  4,000,000 to 1 dynamic range");
    Serial.println("  -40°C to +85°C Temperature Range");
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
    Serial.printf("BME280 Humidity calibrated to FLUKE 971 with offset: %.1f !\r\n", BME280_Hum_offset);


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
    if (!ICM20948_dpEng.begin(ICM20948_ACCELRANGE_4G, GYRO_RANGE_250DPS, ICM20948_ACCELLOWPASS_50_4_HZ))
    {
      /* There was a problem detecting the ICM20948 ... check your connections */
      Serial.println("ICM20948/AK09916 init error, no chip detected ... Check your wiring!");
      delay(50);
    }
    else {
      Serial.print( F("-- Initialization of the motion sensor returned: ") );
      /* Display some basic information on this sensor */
      displaySensorDetails();
      ICM_initialized = true;
      ICM20948_ok = true;
      dev_cnt++;
      Serial.println( "* 9 axis motion sensor ICM-20948 is initialized!" );
    }
  }
  IMU_filter.begin();

  //old api
  //  while ( !ICM_initialized ) {
  //    //0 use ICM20948_I2C_ADDR_AD0  0x68
  //    ICM20948.begin( Wire, 0 );
  //
  //
  //    Serial.print( F("-- Initialization of the motion sensor returned: ") );
  //    Serial.println( ICM20948.statusString() );
  //    if ( ICM20948.status != ICM_20948_Stat_Ok ) {
  //      Serial.println( "Trying init. again..." );
  //      delay(500);
  //    } else {
  //      ICM_initialized = true;
  //      uint8_t ICM20948_ID =  ICM20948.getWhoAmI();
  //      Serial.print("ICM20948 chip ID check: ");
  //      Serial.println(ICM20948_ID, HEX);
  //
  //      //Serial.println(ICM20948.statusString(ICM20948_ID)); //#define ICM_20948_WHOAMI    0xEA
  //      Serial.println( "* 9 axis motion sensor ICM-20948 is initialized!" );
  //      ICM20948_ok;
  //      dev_cnt++;
  //
  //    }
  //  }
  //  ICM20948_config();

  //*******************RFID*********************
  Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
  Serial.println("^^ Now doing RFID Reader module init....");
  if (mfrc522.PCD_Init2() ) {

    Serial.println( "* RFID Reader module mfrc522 is initialized!" );
    mfrc522_ok = true;
    dev_cnt++;
    ShowMFRC522Details();
  }
  else {
    Serial.println( "* RFID Reader module is not connected!" );
    mfrc522_ok = false;
  }


  if (MAX30205_sensor.begin2())
  {
    Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
    dev_cnt++;
    MAX30205_ok = true;
    delay(50);
    Serial.println("* MAX30205 high precision body temperature sensor is connected!");
    Serial.println("  16-Bit (0.00390625°C) Temperature Resolution!");
    Serial.println("  0.1°C Accuracy (37°C to 39°C)!");
    Serial.println("  Operating range: 0°C to 50°C !");
    Serial.println("  configuration of Thyst and TOS finished");
    Serial.println("--->Regs settings readout:");
    MAX30205_sensor.printRegisters();
    Serial.println("##: Auto mode, Thyst is 29 degree, TOS is 30 degree!");
    delay(50);

  }
  else  Serial.println("Couldn't find body temperature sensor Max30205! Check your PCB module connections and verify the IIC address 0x49 is correct.");


  Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");

  //----------sensor all init. above----------------
  Serial.printf("# Sensor initialization finished, total sensor connected %d !\r\n", dev_cnt);


  //--------mount spi flash filesystem
  if (FORMAT_FFAT) FFat.format();//run only once
  if (!FFat.begin()) {
    Serial.println("FFat Mount Failed");
    //return;
  }
  else {
    Serial.println("-->:Internal flash FFat file system is initilized!");
  }
  unsigned mem_space = FFat.totalBytes();
  Serial.printf("Total space: %10u bytes (%2u MB)\n", mem_space, mem_space / 1024 / 1024);
  mem_space = FFat.freeBytes();
  Serial.printf("Free space: %10u bytes (%2u MB)\n", mem_space, mem_space / 1024 / 1024);
  listDir(FFat, "/", 0);
  Serial.println("Now read wifi settings from Flash file system...");
  //read    wifi_config
  //read spi flash file , pass content to msg, max 31 bytes
  //void readFilestring(fs::FS & fs, const char * path, char * msg )
  //char* tmpfilecontent=wifi_config_num;
  char tmpfilecontent[1];
  readFilestring(FFat, "/Wifi_config_Nr.txt", tmpfilecontent); //bugs?!
  readFilestring(FFat, "/Wifi_ssid.txt", ssid_new); //bugs?
  readFilestring(FFat, "/Wifi_password.txt", password_new); //bugs?
  // atoi();
  Serial.println("Now pass the parameters to the current system...");
  //wifi_connectto_default
  if (wifi_connectto_default)
    wifi_config = wifi_config_num; // default settings,  need improve
  else wifi_config = atoi(tmpfilecontent);
  Serial.printf("wifi_config : %d \r\n", wifi_config, wifi_config);
  //wifi_config = (char) *tmpfilecontent;

  //----------config wifi and connect----------------
  Serial.println("Setting up Wifi and MQTT now...");
  setup_wifi();
  if (wifi_ok) {
    Serial.printf("-->Now setting MQTT server: %s port 1883\r\n", mqtt_server);
  }
  client.setServer(mqtt_server, 1883);
  delay(50);
  client.setCallback(MQTT_RX_callback);
  delay(150);
  if (wifi_ok) {
    if (!client.connected()) {
      MQTT_reconnect();
    }
  }
  //client.loop();
  Serial.printf("MQTT status: %d (0 means MQTT_CONNECTED )\r\n", client.state());
  delay(100);
  client.publish(MQTT_Info_head, "online");
  delay(20);
  //client.publish("strtd/CT34.0090-P/online", "device will go online now"); //old api
  // m5mqtt.publish(str('strtd/'+Device_ID+'/online'), "Smart device is online!")
  Serial.printf("MQTT sent: %s:online", MQTT_Info_head);
  Serial.println("");





  Serial.println("#######################################");
  Serial.println("-->  ^^^ Main loop running now... ^^^");

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
    Serial.printf("Bot light =  %.2f Lux\r\n", lux_max44009 );
  }

  if (MAX30205_ok) {
    T_max30205 = MAX30205_sensor.getTemperature();
    Serial.printf("-->: MAX30205 body T =  %.3f °C\r\n", T_max30205 );
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
    hum += BME280_Hum_offset;
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
  if (MCP9808_ok && MCP9808_ENA) {
    MCP9808_T = MCP9808.readTemperature();
    Serial.printf("MCP9808 PCB Temperature =  %.2f °C\r\n", MCP9808_T);
    Serial.printf("%%%MCP9808 Temperature raw =  %d  \r\n", MCP9808.readTempRaw());
    //uint16_t regval = MCP9808.read16(MCP9808_REG_CONFIG);
    Serial.printf("  MCP9808 Config REG: %d \r\n", MCP9808.read16(MCP9808_REG_CONFIG));
    //
    //MCP9808.shutdown();
    //tests only
    //    MCP9808_T = MCP9808.readTempC();
    //    Serial.printf("MCP9808.readTempC() =  %5.2f °C\r\n", MCP9808_T);
    //    MCP9808_T = MCP9808.readTemperature2();
    //    Serial.printf("MCP9808.readTemperature2() =  %5.2f °C\r\n", MCP9808_T);

  }

  //-------------NCIR test!------------
  if (MLX90614_ok) {

    T_MLX_obj = MLX90614.readObjectTempC();
    Serial.printf("--> MLX90614 Obj Temperature =  %.2f °C\r\n", T_MLX_obj);
    T_MLX_self = MLX90614.readAmbientTempC();
    Serial.printf("--> MLX90614 self Temperature =  %.2f °C\r\n", T_MLX_self);


  }




  if (ICM20948_ok) {
    //now = millis();
    sensors_event_t accel_event;
    sensors_event_t gyro_event;
    sensors_event_t mag_event;

    // Get new data samples
    ICM20948_dpEng.getEvent(&accel_event, &gyro_event, &mag_event);

    // Apply mag offset compensation (base values in uTesla)
    float x = mag_event.magnetic.x - mag_offsets[0];
    float y = mag_event.magnetic.y - mag_offsets[1];
    float z = mag_event.magnetic.z - mag_offsets[2];

    // Apply mag soft iron error compensation
    float mx = x * mag_softiron_matrix[0][0] + y * mag_softiron_matrix[0][1] + z * mag_softiron_matrix[0][2];
    float my = x * mag_softiron_matrix[1][0] + y * mag_softiron_matrix[1][1] + z * mag_softiron_matrix[1][2];
    float mz = x * mag_softiron_matrix[2][0] + y * mag_softiron_matrix[2][1] + z * mag_softiron_matrix[2][2];

    // Apply gyro zero-rate error compensation
    float gx = gyro_event.gyro.x + gyro_zero_offsets[0];
    float gy = gyro_event.gyro.y + gyro_zero_offsets[1];
    float gz = gyro_event.gyro.z + gyro_zero_offsets[2];

    // Update the filter
    IMU_filter.update(gx, gy, gz,
                      accel_event.acceleration.x, accel_event.acceleration.y, accel_event.acceleration.z,
                      mx, my, mz);

    // Print the orientation filter output
    // Note: To avoid gimbal lock you should read quaternions not Euler
    // angles, but Euler angles are used here since they are easier to
    // understand looking at the raw values. See the ble fusion sketch for
    // and example of working with quaternion data.
    float roll = IMU_filter.getRoll();
    float pitch = IMU_filter.getPitch();
    float heading = IMU_filter.getYaw(); //mag poles
    if (yaw_compensation_EN)
      heading = heading - yaw_offsets ;//!!!! 2019.11.18 only for PCB #1 !!!!
    //now = millis() - now;
    //Serial.print(millis());
    Serial.print("--> Orientation(Yaw,Pitch,Row): ");
    Serial.print(heading);
    Serial.print(" ");
    Serial.print(pitch);
    Serial.print(" ");
    Serial.println(roll);
    Tilt_A = Calc_tilt(ICM20948_dpEng.accel_raw.x, ICM20948_dpEng.accel_raw.y, ICM20948_dpEng.accel_raw.z);
    if (pitch < 0)
      Tilt_A = -Tilt_A;
    Serial.printf("--> PCB Tilt in steady state: %.1f degree\r\n", Tilt_A);
    //Show_raw_values();
    show_sci_values();
    Linear_ACC = Calc_linearACC (ACC_X, ACC_Y, ACC_Z) ; // m/s2
    Vibration = Linear_ACC;
    Linear_ACC = Linear_ACC / Gravity_0; // to G
    Linear_ACC_max = max(Linear_ACC_max, Linear_ACC);
    Serial.printf("--> current Linear Acc: %.2f G; Max value: %.2f ; Recorded Max: %.2f G\r\n", Linear_ACC, Linear_ACC_max, Linear_ACC_max_last);
    Serial.println("");


  }


  M5.update(); // This function reads The State of Button A and B and C.

  //toggle gas reading
  if (M5.BtnA.wasPressed() ) {
    buttonA_wasPressed();
    //Serial.printf("BtnA was Pressed\r\n");
  }

  //M5.BtnA/B/C. pressedFor(uint32_t ms); If the button is pressed more than the set time – always returns true
  //set LCD brightness
  if (M5.BtnA.pressedFor(500) ) {
    buttonA_longPressed();
    Serial.printf("BtnA was long Pressed\r\n");
  }


  if (M5.BtnA.pressedFor(950) ) {
    buttonA_longPressed2();
    Serial.printf("!!--!! BtnA was long Pressed over 750ms \r\n");
  }

  if (M5.BtnA.pressedFor(1250) ) {
    buttonA_longPressed3();
    Serial.printf("!!--!! BtnA was long Pressed over 1250ms \r\n");
  }


  if (Wrover_module)
    delay(time_interval); // total 500ms loop
  //MCP9808.wakeup();   // wake up, delay 250ms; ready to read!
  run_cnt++;
  //delay(100); //100ms


  //---------timer 1 check and call back-----------
  TMR1_stop = millis();
  float Zeit = (TMR1_stop - TMR1_start) / 1000.0; //to seconds
  if (Zeit >= TMR1_interval) {

    if ( WiFi.status() == WL_CONNECTED) {
      wifi_ok = true;
    }
    else {
      Serial.println("WiFi connection is lost!");
      wifi_ok = false;
      //MQTT_active = false;
      WiFi.reconnect();
      setup_wifi(); //reconnect wifi
    }
    //reset timer1
    TMR1_start = millis();
    Zeit = 0;
  }


  if (run_cnt % 10 == 0) {




    if (MQTT_active && wifi_ok  ) {
      //MQTT routine
      if (!client.connected()) {
        MQTT_reconnect();
      }
      client.loop();
    }
  }
  //  //get motion sensor offsets
  //  if (2 == run_cnt)
  //  {
  //    if ( ICM20948.dataReady() ) {
  //      ICM20948.getAGMT();
  //      reset_ACC_offset(ICM20948.agmt);
  //    }

  //    Serial.printf("ACC ofsets X:%6.1f mG; Y:%6.1f mG; Z:%6.1f mG\r\n", ACC_X_offset, ACC_Y_offset, ACC_Z_offset);
  //  }

  //  if (run_cnt % 5 == 0)
  //  {
  //-------------MCP9808 bugs!------------
  //    if (MCP9808_ok) {
  //      MCP9808_T = MCP9808.readTemperature();
  //      Serial.printf("MCP9808 PCB Temperature =  %.2f °C\r\n", MCP9808_T);
  //      Serial.printf("%%%MCP9808 Temperature raw =  %d  \r\n", MCP9808.readTempRaw());
  //      //
  //       MCP9808.shutdown();
  //
  //    }
  //-------------motion sensor test  old api------------
  //    if ( ICM20948.dataReady() ) {
  //      ICM20948.getAGMT();                // The values are only updated when you call 'getAGMT'
  //      //    printRawAGMT( ICM20948.agmt );     // Uncomment this to see the raw values, taken directly from the agmt structure
  //      printScaledAGMT( ICM20948.agmt);   // This function takes into account the sclae settings from when the measurement was made to calculate the values with units
  //      Vibration = calc_vibration(ICM20948.agmt);
  //      Serial.printf("Device Vibration:%6.1f mG\r\n", Vibration);
  //
  //      //delay(30);
  //    }

  //  }
  //one time
  if (6 == run_cnt) {
    if (MQTT_active && wifi_ok  ) {

      Serial.println("-->MQTT try sending online info now...\r\n");
      client.publish("MQTT_info_head", "online");
      Serial.println("-->online msg sent!\r\n");
    }
  }

  if (run_cnt % 10 == 0) {

    RFID_READ_TAG();
    Serial.printf("MQTT status: %d (0 means MQTT_CONNECTED )\r\n", client.state());

    if (MQTT_active && wifi_ok  ) {
      //-------tmperature sellection----------
      if (MAX30205_ok) {
        if ( (T_max30205 > MAX30205_Range_L) && (T_max30205 < MAX30205_Range_H) ) {
          //switch to high resolution high presion source
          tmperature = T_max30205 ;
          tmperature_source = 1;
        }
        else tmperature_source = 0;
      }
      else tmperature_source = 0;



      Serial.println("MQTT try sending data now...\r\n");
      //    char buf1[10] = "string1";
      //    char buf2[10] = "string2";
      //    char * dtostrf( double __val, signed char __width, unsigned char __prec, char * __s)
      //    The minimum field width of the output string (including the possible '.' and the possible sign for negative values) is given in width, and prec determines the number of digits after the decimal sign
      //    strcpy(myNewCombinedArray, buf1);
      //    strcat(myNewCombinedArray, ";");
      //    strcat(myNewCombinedArray, buf2);
      //^^^^^^^^^^^^^^^^^^^^New server APi^^^^^^^^^^^^^^^^^^^^^
      //Message1：Ta=32.44,To=44.32,AP=1023.34,Hum=56,AT= 100.1,LT=678.3,LB=23.56
      //Message2：Vib=0.23,Tilt=30.1,LA=1.01,ACC_X=0.213,ACC_Y=0.233,ACC_Z=9.834
      //Message3：CYC=3444, TAG = 9d324d5t
      //Message4：Head=30.1, Pitch = 20.4，roll=10.1
      //unsigned int len = 0;
      memset(MQTT_payload, 0, sizeof(MQTT_payload)); //no bugs
      //Ta not stored in MQTT_payload? bug fixed!
      if (tmperature_source)
        dtostrf(tmperature, 3, 3, msg);
      else  dtostrf(tmperature, 3, 2, msg);
      sprintf(MQTT_payload, "Ta="); //end with null?
      strcat(MQTT_payload, msg);
      //len=sprintf(MQTT_payload, "Ta="); //end with null?
      //MQTT_payload[len] = '';




      //T obj  NCIR
      strcat(MQTT_payload, ",To=");
      dtostrf(T_MLX_obj, 3, 2, msg);
      strcat(MQTT_payload, msg);


      //H
      strcat(MQTT_payload, ",Hum=");
      dtostrf(hum, 2, 2, msg);
      strcat(MQTT_payload, msg);

      //A
      strcat(MQTT_payload, ",AP=");
      dtostrf(pressure, 4, 2, msg);
      strcat(MQTT_payload, msg);


      //light top+bot
      strcat(MQTT_payload, ",LT=");

      dtostrf(lux_BH1750, 6, 1, msg);
      strcat(MQTT_payload, msg); //light
      strcat(MQTT_payload, ",LB=");
      dtostrf(lux_max44009, 6, 1, msg);
      strcat(MQTT_payload, msg); //light

      Serial.printf("MQTT Msg1: %s: %s \r\n", MQTT_data_head, MQTT_payload);
      client.publish(MQTT_data_head, MQTT_payload);
      delay(10);

      //$$$$$$$$$$$$---------Message2：Vib=0.23,Tilt=30.1,LA=1.01(G),ACC_X=0.213,ACC_Y=0.233,ACC_Z=9.834
      memset(MQTT_payload, 0, sizeof(MQTT_payload)); //no bugs
      //vibration
      sprintf(MQTT_payload, "Vib=");
      dtostrf(Vibration, 3, 2, msg);
      strcat(MQTT_payload, msg);

      //Tilt
      strcat(MQTT_payload, ",Tilt=");
      dtostrf(Tilt_A, 3, 2, msg);
      strcat(MQTT_payload, msg);

      //LA
      strcat(MQTT_payload, ",LA=");
      dtostrf(Linear_ACC, 2, 2, msg);
      strcat(MQTT_payload, msg);

      //ACC_X
      strcat(MQTT_payload, ",ACC_X=");
      dtostrf(ACC_X, 3, 3, msg);
      strcat(MQTT_payload, msg);

      //ACC_Y
      strcat(MQTT_payload, ",ACC_Y=");
      dtostrf(ACC_Y, 3, 3, msg);
      strcat(MQTT_payload, msg);

      //ACC_Z
      strcat(MQTT_payload, ",ACC_Z=");
      dtostrf(ACC_Z, 3, 3, msg);
      strcat(MQTT_payload, msg);

      Serial.printf("MQTT Msg2: %s: %s \r\n", MQTT_data_head, MQTT_payload);
      client.publish(MQTT_data_head, MQTT_payload);
      delay(10);


      //$$$$--------------Message3：CYC=3444, TAG = 9d324d5t

      memset(MQTT_payload, 0, sizeof(MQTT_payload));
      //Cycle,Combitac_ID_FE
      sprintf(MQTT_payload, "CYC=%d,TAG=%s", run_cnt, String(TAG_NFC) );  // for real tag test 2019.12.2 TAG_NFC[i]
      //sprintf(MQTT_payload, "CYC=%d,TAG=%s", run_cnt, Combitac_ID_FE);  // for sim test 2019.12.2
      //sprintf(MQTT_payload, "CYC=%d,TAG=%s", run_cnt, String(run_cnt)); //test only
      //sprintf(MQTT_payload, "CYC=%d,TAG=%c", run_cnt, run_cnt); //seems working
      //      dtostrf(run_cnt, 2, 0, msg);
      //      strcat(MQTT_payload, msg);

      //TAG
      //      strcat(MQTT_payload, ",TAG=");
      //      dtostrf(run_cnt, 3, 0, msg);
      //      strcat(MQTT_payload, msg);
      Serial.printf("MQTT Msg3: %s: %s \r\n", MQTT_data_head, MQTT_payload);
      client.publish(MQTT_data_head, MQTT_payload);
      delay(10);
      //^^^^^^^^^^^^^^^^^^^^new MQTT APi end^^^^^^^^^^^^^^^^^^^^^




      //^^^^^^^^^^^^^^^^^^^^Old APi^^^^^^^^^^^^^^^^^^^^^
      // Convert the value to a char array
      //char humString[8];
      //char *dtostrf(double val, signed char width, unsigned char prec, char *s)
      //      dtostrf(lux_max44009, 6, 1, msg);
      //      Serial.printf("MQTT raw Msg 1:%s\r\n", msg);
      //      client.publish("stsmd/CT34.0090-P/light_B", msg);
      //      Serial.printf("MQTT Msg 1 sent: stsmd/CT34.0090-P/light: %.1f Lux\r\n", lux_max44009);

      //      dtostrf(MCP9808_T, 6, 2, msg);
      //      Serial.printf("MQTT raw Msg 2:%s\r\n", msg);
      //      client.publish("stsmd/CT34.0090-P/T_PCB", msg);
      //Serial.println(String("")+"Your Height="+height +   ", and Weight=" + weight);
      //data_payload = Data_ava_flag+str("[")+str(T)+str(",")+str(H)+str(",")+str(A)+str(",")+str(Vibration)+str(",")+str(BH_data)+str("] ")
      /*
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
        client.publish("strtd/CT34.0090-P/cycle", tmp_string);
      */
      //^^^^^^^^^^^^^^^^^^^^Old APi^^^^^^^^^^^^^^^^^^^^^

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

  //  if (run_cnt % 20 == 0) {
  //
  //    //Serial.printf("MQTT status: %d (0 means MQTT_CONNECTED; -1 means MQTT_DISCONNECTED)\r\n", client.state());
  //
  //    if (MQTT_active  ) {
  //      //Serial.printf("MQTT try sending global data now...\r\n");
  //      Serial.printf("&&&MQTT global Msg: %s: %s \r\n", MQTT_GlobalMsg_head, MQTT_payload);
  //      client.publish(MQTT_GlobalMsg_head, MQTT_payload);
  //    }
  //  }//end run_cnt/20 MQTT global
  long now = millis();
  Serial.printf("------- System run count: %d  Loop time cost: %d ms -------\r\n", run_cnt, now - lastMsg);
  Serial.println("");

  //if (now - lastMsg > 5000) {
  //}
  lastMsg = now;

  if (run_cnt % 1000 == 0) {
    Record_max_G();
    //    Serial.println("Recording max linear ACC value now:");
    //    if (Linear_ACC_max > Linear_ACC_max_last) {
    //      Serial.println("Current Linear_ACC_max is larger than previous one, writting files now...");
    //      dtostrf(Linear_ACC_max, 2, 2, tmp_string);
    //      //toFloat()
    //      //sprintf(tmp_string, "%d", run_cnt);
    //      writeFile(FFat, "/Linear_ACC_max.txt", tmp_string);
    //      readFile(FFat, "/Linear_ACC_max.txt");
    //    }

  }

}// end loop

//^^^^^^^^^^^^^^^^^^^^^%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//^^^^^^^^^^^^^^^^^^^^^%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//^^^^^^^^^^^^^^^^^^^^^%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//^^^^^^^^^^^^^^^^^^^^^%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


//^^^^^^^^^^^^^^^^^^^^^ RFID API ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void RFID_READ_TAG (void) {

  // Look for new cards, and select one if present
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }

  // Now a card is selected. The UID and SAK is in mfrc522.uid.
  Serial.print("--> RFID TAG is present with UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);

    //TAG_NFC[i]=mfrc522.uid.uidByte[i]; //for test, cause "CYC=90,TAG=\��"

  }
  sprintf(TAG_NFC, "%2X%2X%2X%2X", mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2], mfrc522.uid.uidByte[3]);
  //sprintf(TAG_NFC[2*i+1],"%1X",0x0F&(mfrc522.uid.uidByte[i]) );
  Serial.println();
}




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

/* old api
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
  Serial.print(" ], Mag micro Tesla(uT) [ ");
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
  //Serial.printf("$$$ Vib calc1: %f \r\n",Vibration);
  Vibration = sqrt(Vibration);
  //Serial.printf("$$$ Vib calc2: %f \r\n",Vibration);
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

*/
/* old api
  void ICM20948_config(void) {

  // The next few configuration functions accept a bit-mask of sensors for which the settings should be applied.

  // Set Gyro and Accelerometer to a particular sample mode
  // options: ICM20948_Sample_Mode_Continuous
  //          ICM20948_Sample_Mode_Cycled
  Serial.println("--set Sample Mode to Continuous now...");
  ICM20948.setSampleMode( (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), ICM_20948_Sample_Mode_Continuous );
  if ( ICM20948.status != ICM_20948_Stat_Ok) {
    Serial.print(F("setSampleMode Continuous returned: "));
    Serial.println(ICM20948.statusString());
  }
  // Set full scale ranges for both acc and gyr
  ICM_20948_fss_t myFSS;  // This uses a "Full Scale Settings" structure that can contain values for all configurable sensors

  myFSS.a = gpm4;         // (ICM20948_ACCEL_CONFIG_FS_SEL_e)
  // gpm2
  // gpm4
  // gpm8
  // gpm16

  myFSS.g = dps500;       // (ICM20948_GYRO_CONFIG_1_FS_SEL_e)
  // dps250
  // dps500
  // dps1000
  // dps2000
  Serial.println("--set Acc FullScale to 4g, GYRO to 500dps now...");
  ICM20948.setFullScale( (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS );
  if ( ICM20948.status != ICM_20948_Stat_Ok) {
    Serial.print(F("set Acc FullScale to 4g, GYRO to 500dps, returned: "));
    Serial.println(ICM20948.statusString());
  }


  // Set up Digital Low-Pass Filter configuration
  ICM_20948_dlpcfg_t myDLPcfg;            // Similar to FSS, this uses a configuration structure for the desired sensors
  myDLPcfg.a = acc_d473bw_n499bw;         // (ICM_20948_ACCEL_CONFIG_DLPCFG_e)
  // acc_d246bw_n265bw      - means 3db bandwidth is 246 hz and nyquist bandwidth is 265 hz
  // acc_d111bw4_n136bw
  // acc_d50bw4_n68bw8
  // acc_d23bw9_n34bw4
  // acc_d11bw5_n17bw
  // acc_d5bw7_n8bw3        - means 3 db bandwidth is 5.7 hz and nyquist bandwidth is 8.3 hz
  // acc_d473bw_n499bw

  myDLPcfg.g = gyr_d361bw4_n376bw5;       // (ICM_20948_GYRO_CONFIG_1_DLPCFG_e)
  // gyr_d196bw6_n229bw8
  // gyr_d151bw8_n187bw6
  // gyr_d119bw5_n154bw3
  // gyr_d51bw2_n73bw3
  // gyr_d23bw9_n35bw9
  // gyr_d11bw6_n17bw8
  // gyr_d5bw7_n8bw9
  // gyr_d361bw4_n376bw5
  Serial.println("--setup Digital Low-Pass Filter parameters now...");
  ICM20948.setDLPFcfg( (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myDLPcfg );
  if ( ICM20948.status != ICM_20948_Stat_Ok) {
    Serial.print(F("Set up Digital Low-Pass Filter configuration returned: "));
    Serial.println(ICM20948.statusString());
  }



  // Choose whether or not to use DLPF
  // Here we're also showing another way to access the status values, and that it is OK to supply individual sensor masks to these functions
  ICM_20948_Status_e accDLPEnableStat = ICM20948.enableDLPF( ICM_20948_Internal_Acc, false );
  ICM_20948_Status_e gyrDLPEnableStat = ICM20948.enableDLPF( ICM_20948_Internal_Gyr, false );
  //ICM_20948_Status_e accDLPEnableStat = ICM20948.enableDLPF( ICM_20948_Internal_Acc, true );
  //ICM_20948_Status_e gyrDLPEnableStat = ICM20948.enableDLPF( ICM_20948_Internal_Gyr, true );
  Serial.print(F("Enable DLPF for Accelerometer returned: "));
  Serial.println(ICM20948.statusString(accDLPEnableStat));
  Serial.print(F("Enable DLPF for Gyroscope returned: "));
  Serial.println(ICM20948.statusString(gyrDLPEnableStat));
  // Now we're going to set up interrupts. There are a lot of options, but for this test we're just configuring the interrupt pin and enabling interrupts to tell us when new data is ready
  /*
      ICM_20948_Status_e  cfgIntActiveLow         ( bool active_low );
      ICM_20948_Status_e  cfgIntOpenDrain         ( bool open_drain );
      ICM_20948_Status_e  cfgIntLatch             ( bool latching );                          // If not latching then the interrupt is a 50 us pulse

      ICM_20948_Status_e  cfgIntAnyReadToClear    ( bool enabled );                           // If enabled, *ANY* read will clear the INT_STATUS register. So if you have multiple interrupt sources enabled be sure to read INT_STATUS first

      ICM_20948_Status_e  cfgFsyncActiveLow       ( bool active_low );
      ICM_20948_Status_e  cfgFsyncIntMode         ( bool interrupt_mode );                    // Can ue FSYNC as an interrupt input that sets the I2C Master Status register's PASS_THROUGH bit

      ICM_20948_Status_e  intEnableI2C            ( bool enable );
      ICM_20948_Status_e  intEnableDMP            ( bool enable );
      ICM_20948_Status_e  intEnablePLL            ( bool enable );
      ICM_20948_Status_e  intEnableWOM            ( bool enable );
      ICM_20948_Status_e  intEnableWOF            ( bool enable );
      ICM_20948_Status_e  intEnableRawDataReady   ( bool enable );
      ICM_20948_Status_e  intEnableOverflowFIFO   ( uint8_t bm_enable );
      ICM_20948_Status_e  intEnableWatermarkFIFO  ( uint8_t bm_enable );
*/
/*
  ICM20948.cfgIntActiveLow(true);                      // Active low to be compatible with the breakout board's pullup resistor
  ICM20948.cfgIntOpenDrain(false);                     // Push-pull, though open-drain would also work thanks to the pull-up resistors on the breakout
  ICM20948.cfgIntLatch(true);                          // Latch the interrupt until cleared
  Serial.print(F("cfgIntLatch returned: "));
  Serial.println(myICM.statusString());

  ICM20948.intEnableRawDataReady(true);                // enable interrupts on raw data ready
  Serial.print(F("intEnableRawDataReady returned: "));
  Serial.println(myICM.statusString());
*//*
  Serial.println();
  Serial.println(F("§ 9 Axis Motion sensor Configuration complete!"));
  }

*/

void displaySensorDetails(void)
{
  sensor_t accel, gyro, mag;
  ICM20948_dpEng.getSensor(&accel, &gyro, &mag);
  Serial.println("------------------------------------");
  Serial.println("ACCELEROMETER");
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(accel.name);
  Serial.print  ("Driver Ver:   "); Serial.println(accel.version);
  Serial.print  ("Unique ID:    0x"); Serial.println(accel.sensor_id, HEX);
  Serial.print  ("Min Delay:    "); Serial.print(accel.min_delay); Serial.println(" s");
  Serial.print  ("Max Value:    "); Serial.print(accel.max_value, 4); Serial.println(" m/s^2");
  Serial.print  ("Min Value:    "); Serial.print(accel.min_value, 4); Serial.println(" m/s^2");
  Serial.print  ("Resolution:   "); Serial.print(accel.resolution, 8); Serial.println(" m/s^2");
  Serial.println("------------------------------------");
  Serial.println("");
  Serial.println("------------------------------------");
  Serial.println("GYROSCOPE");
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(gyro.name);
  Serial.print  ("Driver Ver:   "); Serial.println(gyro.version);
  Serial.print  ("Unique ID:    0x"); Serial.println(gyro.sensor_id, HEX);
  Serial.print  ("Min Delay:    "); Serial.print(accel.min_delay); Serial.println(" s");
  Serial.print  ("Max Value:    "); Serial.print(gyro.max_value); Serial.println(" g");
  Serial.print  ("Min Value:    "); Serial.print(gyro.min_value); Serial.println(" g");
  Serial.print  ("Resolution:   "); Serial.print(gyro.resolution); Serial.println(" g");
  Serial.println("------------------------------------");
  Serial.println("");
  Serial.println("------------------------------------");
  Serial.println("MAGNETOMETER");
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(mag.name);
  Serial.print  ("Driver Ver:   "); Serial.println(mag.version);
  Serial.print  ("Unique ID:    0x"); Serial.println(mag.sensor_id, HEX);
  Serial.print  ("Min Delay:    "); Serial.print(accel.min_delay); Serial.println(" s");
  Serial.print  ("Max Value:    "); Serial.print(mag.max_value); Serial.println(" uTesla");
  Serial.print  ("Min Value:    "); Serial.print(mag.min_value); Serial.println(" uTesla");
  Serial.print  ("Resolution:   "); Serial.print(mag.resolution); Serial.println(" uTesla");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}



void show_sci_values (void) {


  sensors_event_t aevent, gevent, mevent;

  /* Get a new sensor event */
  ICM20948_dpEng.getEvent(&aevent, &gevent, &mevent);
  ACC_X = aevent.acceleration.x;
  ACC_Y = aevent.acceleration.y;
  ACC_Z = aevent.acceleration.z;

  /* Display the accel results (acceleration is measured in m/s^2) */
  Serial.print("A ");
  Serial.print("X: "); Serial.print(aevent.acceleration.x, 4); Serial.print("  ");
  Serial.print("Y: "); Serial.print(aevent.acceleration.y, 4); Serial.print("  ");
  Serial.print("Z: "); Serial.print(aevent.acceleration.z, 4); Serial.print("  ");
  Serial.println("m/s^2");

  /* Display the gyro results (gyro data is in g) */
  Serial.print("G ");
  Serial.print("X: "); Serial.print(gevent.gyro.x, 1); Serial.print("  ");
  Serial.print("Y: "); Serial.print(gevent.gyro.y, 1); Serial.print("  ");
  Serial.print("Z: "); Serial.print(gevent.gyro.z, 1); Serial.print("  ");
  Serial.println("g");

  /* Display the mag results (mag data is in uTesla) */
  Serial.print("M ");
  Serial.print("X: "); Serial.print(mevent.magnetic.x, 1); Serial.print("  ");
  Serial.print("Y: "); Serial.print(mevent.magnetic.y, 1); Serial.print("  ");
  Serial.print("Z: "); Serial.print(mevent.magnetic.z, 1); Serial.print("  ");
  Serial.println("uT");

  Serial.println("");

}
void Show_raw_values( void) {

  sensors_event_t accel_event;
  sensors_event_t gyro_event;
  sensors_event_t mag_event; // Need to read raw data, which is stored at the same time

  // Get new data samples
  ICM20948_dpEng.getEvent(&accel_event, &gyro_event, &mag_event);
  Serial.print(" - Raw ACC_GRYO_Mag:");

  Serial.print(ICM20948_dpEng.accel_raw.x);
  Serial.print(',');
  Serial.print(ICM20948_dpEng.accel_raw.y);
  Serial.print(',');
  Serial.print(ICM20948_dpEng.accel_raw.z);
  Serial.print(',');

  Serial.print(ICM20948_dpEng.gyro_raw.x);
  Serial.print(',');
  Serial.print(ICM20948_dpEng.gyro_raw.y);
  Serial.print(',');
  Serial.print(ICM20948_dpEng.gyro_raw.z);
  Serial.print(',');

  Serial.print(ICM20948_dpEng.mag_raw.x);
  Serial.print(',');
  Serial.print(ICM20948_dpEng.mag_raw.y);
  Serial.print(',');
  Serial.print(ICM20948_dpEng.mag_raw.z);
  Serial.println();



}

float Calc_linearACC ( float acc_x, float acc_y, float acc_z) {

  float Linear_ACC = 0 ;
  float R =  sq(acc_x ) + sq(acc_y) + sq(acc_z)  ;
  if (R > Gravity_HZ)
    Linear_ACC = sqrt(R - sq(Gravity_HZ)); // <0 bugs found @ 2019.11.19
  else Linear_ACC = 0;

  return Linear_ACC;
}

float Calc_tilt ( float acc_x, float acc_y, float acc_z) {

  float tilt_angle = 0 ;
  float unit_g =  sq(acc_x ) + sq(acc_y) + sq(acc_z)  ;
  unit_g = sqrt(unit_g);
  //cos(tilt_angle)= acc_z /unit_g ;
  tilt_angle = acos(acc_z / unit_g); //radians
  tilt_angle = tilt_angle * RAD_TO_DEG ; //convert to degree,#define RAD_TO_DEG 57.295779513082320876798154814105
  return tilt_angle;
}

void Record_max_G(void) {

  if (Linear_ACC_max > Linear_ACC_max_last) {
    Serial.println("Current Linear_ACC_max is larger than previous one, writting files now...");
    dtostrf(Linear_ACC_max, 2, 2, tmp_string);
    //toFloat()
    //sprintf(tmp_string, "%d", run_cnt);
    writeFile(FFat, "/Linear_ACC_max.txt", tmp_string);
    G_readFile(FFat, "/Linear_ACC_max.txt");
  }
}



//-----RFID MFRC522-----------------
void ShowMFRC522Details() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}


//read spi flash file , pass content to msg, max 31 bytes
void readFilestring(fs::FS &fs, const char * path, char * msg ) {

  char buf[32];
  unsigned int i = 0 ;
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while (file.available()) {
    buf[i] = file.read();
    msg[i] = buf[i]; //bug?
    Serial.write(buf[i]);
    i++;
  }
  Serial.println("");
  Serial.println("copy contents to vars!");
  //apply read value to Linear_ACC_max_last
  // msg = char*(buf);
  // return tmp_string;
  //Linear_ACC_max_last = tmp_string.toFloat();
  //Serial.printf("Apply %.2f to Linear_ACC_max_last!", Linear_ACC_max_last);
  //Serial.println("");
  Serial.println("File reading ends here!");
}

//read spi flash file , Serial print
void readFile(fs::FS &fs, const char * path) {

  char buf[32];
  unsigned int i = 0 ;
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while (file.available()) {
    buf[i] = file.read();
    Serial.write(buf[i]);
    i++;
  }
  Serial.println("");
  Serial.println("File reading ends here!");

  //apply read value to Linear_ACC_max_last
  //String tmp_string = String(buf);
  // return tmp_string;
  //Linear_ACC_max_last = tmp_string.toFloat();
  //Serial.printf("Apply %.2f to Linear_ACC_max_last!", Linear_ACC_max_last);
  //Serial.println("");
}






// only for Linear_ACC_max_last
void G_readFile(fs::FS &fs, const char * path) {

  char buf[32];
  unsigned int i = 0 ;
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while (file.available()) {
    buf[i] = file.read();
    Serial.write(buf[i]);
    i++;
  }
  Serial.println("");
  Serial.println("File reading ends here!");

  //apply read value to Linear_ACC_max_last
  String tmp_string = String(buf);
  Linear_ACC_max_last = tmp_string.toFloat();
  Serial.printf("Apply %.2f to Linear_ACC_max_last!", Linear_ACC_max_last);
  Serial.println("");
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}


void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message: 'World!' appended");
  } else {
    Serial.println("- append failed");
  }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("- file renamed");
  } else {
    Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}
