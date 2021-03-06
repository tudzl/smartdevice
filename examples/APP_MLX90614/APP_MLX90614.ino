/* this code is working as APP launcher compatible
 *  Version 6.2  improve max30205 sensor GUI  ,  working with m5stack， 2019.12.25
    Version 6.1  improve MLX90614 smartdevice sensor GUIMZ ,  working with m5stack， 2019.12.14
    Version 6.0  Add MLX90614 smartdevice sensor ,  working with m5stack， 2019.12.10
    Version 5.0  Add altitude base , offset diff measure function
    Version 4.0  HP206C bug fixed!  altitude: ulong changed to long
    Version 3.1  Zfilter and kalman filter works!
    Version 2.2  HP206C works!
    need to improve BME680 T reading accuracy!
    m5stack fire arduino device test app for ENV unit and BME680 module +HP206C  module
    Author ling zhou, 16.8.2019
    note: need add library Adafruit_BMP280 from library manage
    note: need add library Adafruit_BME680 from library manage
*/


//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
//#include <M5TreeView.h> // https://github.com/lovyan03/M5Stack_TreeView
//#include <M5OnScreenKeyboard.h> // https://github.com/lovyan03/M5Stack_OnScreenKeyboard/

#include <M5Stack.h>
//#include "utility/Power.h"


#include <Wire.h> //The DHT12 uses I2C comunication.

//NCIR
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 MLX90614 = Adafruit_MLX90614();
//sensors
#include "DHT12.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BME680.h"
#include "Adafruit_BMP280.h"
#include <HP206C_Zlib.h>
#include "Protocentral_MAX30205.h" //body T sensor
#include <ZFilter.h>
//For #include "filename" the preprocessor searches first in the same directory as the file containing the directive, and then follows the search path used for the #include <filename> form. This method is normally used to include programmer-defined header files.
//bmp280 address:  0x76
//HP206C address:  0x76
//BME680 address:  0x76-->0x77:
unsigned char BME680_add = 0x77;
unsigned char hp206_add = 0x76;
unsigned char bmp280_add = 0x76;
//DHT address:  0x5C
DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
Adafruit_BME680 bme680; // I2C  0x76
Adafruit_BMP280 bmp280;  // I2C  0x76
HP20x_dev HP206;  // I2C  0x76
MAX30205 MAX30205_sensor;

POWER m5_power;
//global vars
float pressure = 0;
float T_bmp = 0;
float Alt_bmp = 0;
double Altitude = 0; // in meter @ 25 degree
double Altitude_offset = 0;   // in meter
float T_MLX_obj = 0; //NCIR T
float T_MLX_self = 0;

float T_max, T_min = 0; // for object
float TA_max, TA_min = 0; // for ambient
//max30205
bool MAX30205_ok = false;
double T_max30205 = 0;
unsigned int MAX30205_Range_H = 50;
unsigned int MAX30205_Range_L = 0;
float MAX30205_TOV = 27;
float MAX30205_HYST = 26;

//status vars
bool dht12_ok = false;
bool bmp280_ok = false;
bool bme680_ok =  false;
bool HP206_ok = false;
bool MLX90614_ok = false;

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
bool Wrover_module = false  ;
unsigned long run_cnt = 0;
unsigned int time_interval = 200; //cycle time

//------ Height meter vars
float height_base =  0;
float height_new =  0;
float height_diff =  0;

//for bme680 only, toggle gas measurrement
void buttonA_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);

  Gas_EN = !Gas_EN;

}

//set the lcd Brightness
void buttonA_longPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  M5.lcd.setBrightness(30);
}

void buttonB_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  if (HP206_ok)
    height_base = A_Kfilter;
  else if (bme680_ok)
    height_base = Altitude;
}
//set the altitude offset to current altitude
void buttonB_longPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  Altitude_offset = Altitude;

}

void buttonC_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);


  if (HP206_ok)
    height_new = A_Kfilter;
  else if (bme680_ok)
    height_new = Alt_bmp;
  height_diff = height_new - height_base;

}



void setup() {
  //void M5Stack::begin(bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable)
  M5.begin();

  Wire.begin();

  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

  Serial.println(F("<<<M5stack Smart sensor test>>>"));
  Serial.println(F("<<<Firmware Version 6.0, ling zhou, 12.12.2019>>>"));

  m5_power.begin();
  //POWER::ShutdownTime SHDT; // SHDT is declared as the enum
  //SHDT = POWER::ShutdownTimeSHUTDOWN_64S;
  m5_power.setLowPowerShutdownTime(POWER::ShutdownTime::SHUTDOWN_32S);
  //m5_power.setLightLoadShutdownTime(); //SHUTDOWN_64S
  //m5_power.setLowPowerShutdownTime(POWER::ShutdownTime::SHUTDOWN_64S);
  m5_power.setPowerBoostSet(true); //短按开关boost, 1:enable
  m5_power.setPowerVin(true);//VIN拔出后，是否开启 Boost
  //m5_power.setPowerBoostOnOff(true)

  bool ip5306status = m5_power.isCharging();
  Serial.println( "ip5306status:");
  Serial.println(  ip5306status);

  M5.Lcd.setBrightness(50);  //define BLK_PWM_CHANNEL 7  PWM
  M5.Lcd.print( "ip5306status:");
  M5.Lcd.println(  ip5306status);


  Serial.println(F("ENV Unit(DHT12 and BMP280) test..."));

  if (dht12.begin(0x5c))
    //if (bmp280.begin(0x76))
  {
    dht12_ok  = true ;
    bmp280.begin(bmp280_add);
    bmp280_ok = true ;
    Serial.println(" dht12 sensor is connected");
    M5.Lcd.println("dht12 sensor is connected");
    delay(400);

  }
  else if (bme680.begin(BME680_add))
  {
    bme680_ok = true ;
    Serial.println(" BME680 sensor is connected");
    M5.Lcd.println("BME680 sensor is connected, init...");
    // Set up oversampling and filter initialization

    bme680.setTemperatureOversampling(BME680_OS_8X);

    bme680.setHumidityOversampling(BME680_OS_2X);

    bme680.setPressureOversampling(BME680_OS_4X);

    bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);

    bme680.setGasHeater(320, 150); // 320*C for 150 ms
    Serial.println(" BME680 setIIRFilterSize = 3");
    delay(400);
  }
  else if (HP206.begin2(hp206_add)) {

    HP206_ok = true ;
    //HP206.HP20X_DisableCompensate();
    Serial.println(" HP206 sensor is connected");
    M5.Lcd.println(" HP206 sensor is connected");
    delay(400);
  }
  else if (MLX90614.begin2()) {
    //MLX90614 chip ID readout: 0  bugs?

    MLX90614_ok = true;
    //dev_cnt++;
    //Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Melexis Infra Red Thermometer MLX90614 sensor is connected!");

    Serial.println("* Melexis Infra Red Thermometer MLX90614 sensor is connected!");
    Serial.println("-40°C…+125˚C for sensor temperature ");
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.println("-40°C…+125˚C for sensor temperature ");
    delay(400);
    Serial.println("-70°C…+380˚C for object temperature ");
    M5.Lcd.println("-70°C…+380˚C for object temperature ");
    delay(400);
    float Emissivity = MLX90614.readEmissivity();
    Serial.printf("Default Emissivity =  %.2f \r\n", Emissivity);
    M5.Lcd.printf("Default Emissivity =  %.2f \r\n", Emissivity);

    Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
    delay(600);

    T_max =   MLX90614.readObjectTempC();
    T_min = T_max;
    TA_max =   MLX90614.readAmbientTempC();
    TA_min = TA_max;
    if (MAX30205_sensor.begin2())
    {
      Serial.println("^^^^^^^^^^^^$$$$$$$$$$$$^^^^^^^^^^^^^^^");
     
      MAX30205_ok = true;
      delay(50);
      Serial.println("* MAX30205 high precision body temperature sensor is connected!");
      M5.Lcd.println("* MAX30205 high precision T sensor is connected!");
      Serial.println("  16-Bit (0.00390625°C) Temperature Resolution!");
      Serial.println("  0.1°C Accuracy (37°C to 39°C)!");
      Serial.println("  Operating range: 0°C to 50°C !");
      Serial.println("  configuration of Thyst and TOS finished");
      Serial.println("--->Regs settings readout:");
      MAX30205_sensor.printRegisters();
      Serial.println("##: Auto mode, Thyst is 29 degree, TOS is 30 degree!");
      delay(100);

    }

  }
  else {
    while (!bmp280.begin(0x76)) {

      Serial.println("Could not find a valid BMP280 sensor, check wiring!");

      M5.Lcd.println("Could not find a valid BMP280 sensor, check wiring!");
      delay(100);


    }
  }

  M5.Lcd.clear(BLACK);

  M5.Lcd.setCursor(0, 220);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(GREEN, BLACK);
  if (dht12_ok)
    M5.Lcd.println("ENV DHT12+BMP280 test V2.1");
  else if (HP206_ok)
    M5.Lcd.println("ENV HP206 test V2.0");
  else  M5.Lcd.println("ENV BME680 test V1.5");

  //lcd.font(lcd.FONT_Default) #改变字体

  //M5.Speaker.beep(); //bug!! cause system restart!!
  //M5.Speaker.begin();
}



void loop() {

  //M5.update(); //moved to loop end

  if (Gas_EN == 0) {
    //disable gas heater
    //setGasHeater(uint16_t heaterTemp, uint16_t heaterTime);
    bme680.setGasHeater(0, 0);
    /* Create a ramp heat waveform in 3 steps */
    //gas_sensor.gas_sett.heatr_temp = 320; /* degree Celsius */
    //gas_sensor.gas_sett.heatr_dur = 150; /* milliseconds */
  }
  else {
    bme680.setGasHeater(320, 150); // 320*C for 150 ms
  }
  //bme680_ok = true ;
  int Akku_level = m5_power.getBatteryLevel();


  if (dht12_ok) {
    tmperature = dht12.readTemperature(); // not very accurate

    hum = dht12.readHumidity();
    Serial.printf("DHT12 T =  %2.2f*C Humidity= %0.2f %%\r\n", tmperature, hum);
  }


  if (MLX90614_ok) {
    T_MLX_obj = MLX90614.readObjectTempC();
    Serial.printf("--> MLX90614 Obj Temperature =  %.2f °C\r\n", T_MLX_obj);
    T_MLX_self = MLX90614.readAmbientTempC();
    Serial.printf("--> MLX90614 self Temperature =  %.2f °C\r\n", T_MLX_self);
    //calc max ,min
    T_max = max(T_max, T_MLX_obj);
    T_min = min(T_min, T_MLX_obj);
    TA_max = max(TA_max, T_MLX_self);
    TA_min = min(TA_min, T_MLX_self);
    Serial.printf("--: MLX90614 Max and min object Temperature =  %.2f °C, %.2f °C\r\n", T_max, T_min);
    Serial.printf("--: MLX90614 Max and min Ambient Temperature =  %.2f °C, %.2f °C\r\n", TA_max, TA_min);

    if (MAX30205_ok) {
      T_max30205 = MAX30205_sensor.getTemperature();
      Serial.printf("-->: MAX30205 body T =  %.3f °C\r\n", T_max30205 );
    }

  }

  //-----------HP206 ----------------

  if (HP206_ok) {
    //ulong data
    tmperature = (float)HP20x.ReadTemperature() / 100.0; // degree seems not very accurate
    pressure = (float)HP20x.ReadPressure() / 100; // in hPA
    Altitude = (double)HP20x.ReadAltitude2() / 100; // in Meter.Bug fixed!!! 42949636m if P > 1017 mBar  ; 42949648M @ 1016 Hpa
    Serial.printf("HP206: T =  %2.2f*C ，pressure：%0.2f hpa，  Altitude: %6.2f M \r\n", tmperature, pressure, Altitude);
    if (filter_on) {
      T_filter = t_filter.Filter(tmperature);
      P_filter = p_filter.Filter(pressure);
      A_filter = a_filter.Filter(Altitude);

      T_Kfilter = t_kalman.Filter(tmperature);
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
      Serial.printf("ZFiltered T =  %2.2f*C ，pressure：%0.2f hpa，  Altitude: %0.2f M \r\n", T_filter, P_filter, A_filter);
      Serial.printf("KalmanFiltered T =  %2.2f*C ，pressure：%0.2f hpa，  Altitude: %0.2f M \r\n", T_Kfilter, P_Kfilter, A_Kfilter);
      Serial.printf("Filtered Altitude= %0.2f M \r\n", filter_val);
      //Serial.printf("%0.2f;%0.2f;%0.2f\r\n" ,pressure,P_filter,P_Kfilter); // for test plot
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


  // $$$$$$$$$$$$$$$$$$$$$$$$$$ ----------- LCD GUI  process ----------------%%%%%%%%%%%%%%%%%%%%%
  M5.Lcd.setCursor(0, 0);

  M5.Lcd.setTextColor(WHITE, BLACK);

  M5.Lcd.setTextSize(3);


  if (MLX90614_ok) {

    M5.Lcd.setTextColor(YELLOW, BLACK);
    //T_MLX_obj ;
    M5.Lcd.printf("NCIR T: %.2f C\r\n", T_MLX_obj);
    M5.Lcd.println("");
    if (MAX30205_ok) {
      //T_max30205 = MAX30205_sensor.getTemperature();
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.printf("Body T = %.3f C\r\n", T_max30205 );
    }
    M5.Lcd.setTextColor(LIGHTGREY, BLACK);
    //T_MLX_self ;
    M5.Lcd.printf("AmbientT: %.2f C\r\n", T_MLX_self);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("");
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("Max & Min object:\r\n %.2f C,  %.2f °C\r\n", T_max, T_min);
    M5.Lcd.setTextColor(BLUE, BLACK);
    M5.Lcd.printf("Max & Min Ambient:\r\n %.2f C,  %.2f °C\r\n", TA_max, TA_min);
    delay(200);




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
    M5.Lcd.printf("Temp: %2.2f C\r\nPress:%0.2fhPa\r\nAltitude:%.2f M\r\n", tmperature,  pressure , Altitude);

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
    //height meter display
    M5.Lcd.setTextSize(2); //size 2 to 8
    M5.Lcd.setTextColor(LIGHTGREY, BLACK);
    M5.Lcd.printf("Base:%0.1f M; Cur:%0.1f M; Dif:%0.1f M \r\n", height_base, height_new, height_diff);
  }



  //-------sys status GUI-------------------------------------
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.setTextSize(2); //size 2 to 8
  M5.Lcd.setTextColor(ORANGE, BLACK);
  M5.Lcd.printf("Akku: %3d%%   Run:%d ", Akku_level, run_cnt);

  M5.update(); // This function reads The State of Button A and B and C.

  //toggle gas reading
  if (M5.BtnA.wasPressed() ) {
    buttonA_wasPressed();
    Serial.printf("BtnA.wasPressed\r\n");
  }

  //M5.BtnA/B/C. pressedFor(uint32_t ms); If the button is pressed more than the set time – always returns true
  //set LCD brightness
  if (M5.BtnA.pressedFor(200) ) {
    buttonA_longPressed();
    Serial.printf("BtnA.was long Pressed\r\n");
  }
  //reset the altitude offset to current altitude
  if (M5.BtnB.pressedFor(250) ) {
    buttonB_longPressed();
    Serial.printf("BtnB.wasPressed\r\n");
  }

  if (M5.BtnB.wasPressed() ) {
    buttonB_wasPressed();
    Serial.printf("BtnB.wasPressed\r\n");
  }
  if (M5.BtnC.wasPressed() ) {
    buttonC_wasPressed();
    Serial.printf("BtnC.wasPressed\r\n");
  }

  if (Wrover_module)
    delay(200); //100ms
  if (HP206_ok)
    delay(time_interval); //time_interval
  run_cnt++;
  //delay(10); //100ms

}
//_______________________________________end loop____________________________________________
