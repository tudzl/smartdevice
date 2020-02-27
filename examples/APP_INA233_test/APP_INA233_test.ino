/* this code is working as APP launcher compatible
    INA233 sensor basic test app
    V2.1 add offset calibration 2020.2.25
    Version 2.0  iic read LSB+MSB bugs fixed!
    Version 1.2 rewrite sensor iic api, test adc_config, bugs!
    Version 1.1  improve sensor GUI ,  working with m5stack +INA233， test OK!
    m5stack fire arduino device test app for zl ina233 board
    Author ling zhou, 18.02.2020
    note: need real device test， 16 possible addresses for INA233!!!
*/
#include <Wire.h>
#include <INA233.h>
//for use as bin app lovyan03 launcher
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
//#include <M5TreeView.h> // https://github.com/lovyan03/M5Stack_TreeView
//#include <M5OnScreenKeyboard.h> // https://github.com/lovyan03/M5Stack_OnScreenKeyboard/

#include <M5Stack.h>
//#include "utility/Power.h"
//#include "Free_Fonts.h" // Include the header file attached to this sketch
/* Example code for LCD library */
// M5.Lcd.setFreeFont(FF18);
// M5.Lcd.drawString(TEXT, 160, 120, GFXFF);// Print the string name of the font
//M5.Lcd.drawString("M5Stack has been connected", 160, 120, 2); //size 2
//Show all 48 fonts in centre of screen ( x,y coordinate 160,120)

uint8_t ina233_addr = 0x40;//@A1, A0 is GND GND
INA233 Sensor_INA233(ina233_addr);  // IC2 address 0x40 1000100
bool INA_sensor_ok = false;

extern const unsigned char m5stack_startup_music[];

POWER m5_power;
unsigned long run_cnt = 0;
bool Cont_Read_EN = true;

uint16_t R_pre, G_pre, B_pre, C_pre, colorTemp_pre, lux_pre; // store previous values
uint16_t r, g, b, c, colorTemp, colorTemp_uni, lux;
uint16_t low_light_thre = 3000 ;
uint16_t mid_light_thre = 35000 ;
int lux_range = 1; //def 1 for low light; 2, 3
// to store current value to previous vars
#define CURRENT_400MA  (0x01 << 2)
#define NOTE_D0 -1
#define NOTE_D1 294
#define NOTE_D2 330
#define NOTE_D3 350
#define NOTE_D4 393
#define NOTE_D5 441
#define NOTE_D6 495
#define NOTE_D7 556


uint16_t loop_interval = 400 ;
bool PC_OSC_monitor_mode = false ;
float flow_rate =0 ; 
float flow_current_ratio =1; //flow_rate = current * flow_current_ratio

//------INA233--Calibration---------
float R_shunt = 2; //Ohm, call RS1 on the board
float C_LSB = 1.25; // uA/Lsb
float I_max = 0.03; //A
const uint8_t INA233_conf_ADC_H = 0b01000101; //0x43   bit 9 to 11: Average = 4
const uint8_t INA233_conf_ADC_L = 0b00111111; //0x3f   Shunt and bus, continuous, 8.244 ms (def 1 0 0: 1.1ms) for both convertion time, bit 0 to 2: mode, def 111
//1.1 ms, 4X average , v shunt continuous
const uint8_t INA233_conf_higespeed_ADC_H = 0b01000011; //0x  bit 9 to 11: Average = 4
const uint8_t INA233_conf_higespeed_ADC_L = 0b00100101; //0x  1.1 ms (def 1 0 0: 1.1ms) for both convertion time, bit 0 to 2: mode, def 111

//------INA233--vars---------
int16_t raw_vbus = 0;
int16_t raw_vshunt = 0;
int16_t raw_current = 0;
int16_t raw_power = 0;
float Vbus = 0;
float V_shunt_mV = 0;
float Current_mA = 0;
double Current_mA_C = 0;
float INA_Power = 0;
float INA233_current_offset = 0 ;


void play_startup_music() {

  uint32_t length = strlen((char*)m5stack_startup_music);
  //logo music
  for (int i = 0; i < length; i++) {
    dacWrite(SPEAKER_PIN, m5stack_startup_music[i] >> 2);
    delayMicroseconds(40);

  }
}

//change sampling mode  high speed or low speed with 4x average
void buttonA_wasPressed(void) {

  uint16_t tmp_date;
  if ( loop_interval > 0 ) {
    loop_interval = 0;
    tmp_date = ((uint16_t)INA233_conf_higespeed_ADC_H ) << 8 ;
    tmp_date += INA233_conf_higespeed_ADC_L;
    Serial.println("Config device to high speed mode(1.1ms +4X)!");


  }
  else {
    tmp_date = ((uint16_t)INA233_conf_ADC_H ) << 8 ;
    tmp_date += INA233_conf_ADC_L;
    Serial.println("Config device to high precesion mode(8.244ms+16X)!");

    loop_interval = 400;
  }
  //M5.Speaker.beep();  // too laud

  Serial.printf("#:Re-Write tmp ADC config value to CHIP: 0b");
  Serial.println(tmp_date, BIN);
  tmp_date = Sensor_INA233.ADC_CONFIG(tmp_date);
  Serial.printf("->:INA233.ADC_CONFIG chip value readback:0b");
  Serial.println(tmp_date, BIN);
  play_startup_music();
}




//pause
void buttonB_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  Cont_Read_EN = !Cont_Read_EN;
  //M5.Speaker.tone(NOTE_D2, 200); //frequency 3000, with a duration of 200ms
  play_startup_music();
}



//offset calibration
void buttonC_wasPressed(void) {
  //M5.Speaker.beep();  // too laud
  //M5.Speaker.tone(800, 20);
  //Cont_Read_EN = !Cont_Read_EN;
  //M5.Speaker.tone(NOTE_D2, 200); //frequency 3000, with a duration of 200ms


  Sensor_INA233.update_value();
  Current_mA_C = Sensor_INA233.CalCurrent_mA(R_shunt);// calc by host


  INA233_current_offset = Current_mA_C ;
  play_startup_music();
}





void setup(void) {
  //Serial.begin(9600);


  M5.begin();
  M5.Lcd.setBrightness(50);  //define BLK_PWM_CHANNEL 7  PWM


  m5_power.begin();//M5.powerOFF(); soft power off
  m5_power.setVinMaxCurrent(CURRENT_400MA );
  //for app flash back
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }



  //M5.Lcd.drawJpgFile(fs::FS &fs, const char *path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, uint16_t offX, uint16_t offY, jpeg_div_t scale);
  M5.Lcd.drawJpgFile(SD, "/APP_flow_meter.jpg");
  delay(50);
  play_startup_music();
  delay(1000);
  M5.Lcd.fillScreen(BLACK);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.println("Flow monitor APP V2.0");

  //variables to catch the outputs from set_Calibration()
  uint16_t CAL = 0;
  int16_t m_c = 0;
  int16_t m_p = 0;
  int8_t R_c = 0;
  int8_t R_p = 0;
  uint8_t Set_ERROR = 0;
  float Current_LSB = 0;
  float Power_LSB = 0;


  //variable to check the loaded calibration
  uint16_t Read_CAL = 0;



  if (Sensor_INA233.begin2(ina233_addr)) {
    Serial.println("Found INA233 sensor");
    Serial.printf("Chip ID: %x\r\n", Sensor_INA233.Read_ID()); //Chip ID: 4954, should be 5449h(TI), return value lsb first!?
    INA_sensor_ok = true;
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setCursor(0, 200);
    M5.Lcd.println("INA Sensor OK!");
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.println("INA233 calibration now...");
    //-------------------------MFR_ADC_CONFIG  bugs!!----------------------
    //uint16_t tmp_date = (uint16_t)INA233_conf_ADC_H << 8 + INA233_conf_ADC_L ; //bug!!!
    uint16_t tmp_date = (uint16_t)INA233_conf_ADC_H << 8 ;
    tmp_date += INA233_conf_ADC_L;
    Serial.printf("Config value Lo: %X, Hi: %X, Word: %X\r\n", INA233_conf_ADC_L, INA233_conf_ADC_H, tmp_date);
    Serial.printf("First write tmp ADC config value to CHIP: 0b");
    Serial.println(tmp_date, BIN);
    tmp_date = Sensor_INA233.ADC_CONFIG(tmp_date);
    Serial.printf("INA233.ADC_CONFIG value readback: %b ");
    Serial.println(tmp_date, BIN);

    //-------------------------MFR_ADC_CONFIG  bugs!!----------------------
    CAL = Sensor_INA233.setCalibration(R_shunt, I_max, &Current_LSB, &Power_LSB, &m_c, &R_c, &m_p, &R_p, &Set_ERROR);
    Serial.printf("INA233.setCalibration value: %d\r\n", CAL);

    Serial.print("-----INA233 calibration results: ------");
    Serial.print("Current LSB :   "); Serial.print(Current_LSB); Serial.println(" uA");
    Serial.print("CAL :   "); Serial.println(CAL);
    Serial.print("m_c :   "); Serial.println(m_c);
    Serial.print("R_c :   "); Serial.println(R_c);
    Serial.print("m_p :   "); Serial.println(m_p);
    Serial.print("R_p :   "); Serial.println(R_p);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf("CAL:%d, C_LSB:%.2f uA", CAL, Current_LSB);
    //M5.Lcd.println("");
    M5.Lcd.setCursor(0, 170);
    M5.Lcd.setTextColor(ORANGE, BLACK);
    M5.Lcd.printf("R= %1.1f Ohms", R_shunt);
    //M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 40);
    if (Set_ERROR == 1)
    {
      Serial.println("CALIBRATION OUT of RANGE, choose a different R_shunt or I_max");
    }
    else //no error
    {
      Sensor_INA233.wireReadWord(MFR_CALIBRATION, &Read_CAL);
      Serial.print("Calibration value read from INA233:   "); Serial.println(Read_CAL);
      if (Read_CAL != CAL) {
        Serial.println("CALIBRATION value not valid!");

        M5.Lcd.setCursor(0, 200);
        M5.Lcd.println("CALIBRATION unvalid!");

        //        Sensor_INA233.RegSetCalibration(CAL);
        //        delay(10);
        //        Sensor_INA233.wireReadWord(MFR_CALIBRATION, &Read_CAL);
        //        if (Read_CAL != CAL) {
        //          Serial.println("CALIBRATION failed!");
        //          M5.Lcd.setCursor(0, 200);
        //          M5.Lcd.println("CALIBRATION unvalid!");
        //        }
        //        else {
        //          Serial.println("CALIBRATION OK");
        //          delay(200);
        //          M5.Lcd.setCursor(0, 200);
        //          M5.Lcd.println("INA CALIBRATIONM OK!");
        //        }
      }
      else {
        Wire.endTransmission();
        Serial.println("CALIBRATION OK");
        delay(200);
        M5.Lcd.setCursor(0, 200);
        M5.Lcd.println("INA CALIBRATION OK!");
      }
    }



  } else {
    Serial.println("No INA found ... check your connections");
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.println("INA Sensor N.A.!");
    INA_sensor_ok = false;
    delay(3000);
    M5.Lcd.println("restart now！");
    delay(500);
    ESP.restart();
    //while (1);
  }

  // Now we're ready to get readings!
  M5.Lcd.drawJpgFile(SD, "/flow_icon.jpg", 216, 134);
  delay(300);
}

void loop(void) {

  M5.update(); // This function reads The State of Button A and B and C.
  int Akku_level = m5_power.getBatteryLevel();
  long Zeit_anfang = millis();
  if (Cont_Read_EN) {

    Sensor_INA233.update_value();
    raw_vshunt  = Sensor_INA233.raw_shunt;
    //raw_vshunt = Sensor_INA233.getShuntVoltage_raw();
    V_shunt_mV = Sensor_INA233.mV_shunt;
    Current_mA = Sensor_INA233.getCurrent_mA(); // calc by ina233
    Current_mA_C = Sensor_INA233.CalCurrent_mA(R_shunt);// calc by host
    Current_mA_C = Current_mA_C - INA233_current_offset; //offset correction
    flow_rate = Current_mA_C *flow_current_ratio; //L/min
  }
  long Zeit_process = millis();



  if (PC_OSC_monitor_mode) {
    //Guru Meditation Error: Core  1 panic'ed (IntegerDivideByZero)
    Serial.printf("%3.3f mA\r\n", Current_mA_C );

  }
  else {

    Serial.printf("Flow rate: %2.3f L/min ", flow_rate );
    Serial.printf("V_shunt: %3.3f mV ", V_shunt_mV );
    Serial.printf("Current: %3.3f mA ", Current_mA );
    //Current_mA_C
    Serial.printf("Calulated C: %3.3f mA ", Current_mA_C );
    Serial.print("raw_V_shunt: ");
    Serial.println(raw_vshunt, DEC);
    //Serial.println("");


    if (INA_sensor_ok) {


      M5.Lcd.setTextSize(3);
      M5.Lcd.setCursor(0, 40);
      M5.Lcd.setTextColor(BLUE, BLACK);
      M5.Lcd.printf("Raw shunt:%d   \r\n", raw_vshunt);
      M5.Lcd.setCursor(0, 70);
      M5.Lcd.setTextColor(YELLOW, BLACK);
      M5.Lcd.printf("V_shunt:%3.3f mV \r\n", V_shunt_mV);
      M5.Lcd.setCursor(0, 100);
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.printf("Cal.C:%3.3f mA ", Current_mA_C );
      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(0, 120);
      M5.Lcd.setTextColor(RED, BLACK);
      M5.Lcd.printf("Current:%3.3f mA \r\n", Current_mA );
      M5.Lcd.setTextColor(WHITE, BLACK);
      M5.Lcd.setTextSize(3);
      M5.Lcd.printf("Flow:%2.2f", flow_rate );
      M5.Lcd.setTextSize(1);
      M5.Lcd.println("L/min");

      //M5.Lcd.setTextColor(GREENYELLOW, BLACK);
      //M5.Lcd.printf("CCT(ref):%d K \r\n", colorTemp_uni);

      M5.Lcd.println("");
    }
    //history GUI

  }


  long now = millis();
  //SYS  GUI
  M5.Lcd.setCursor(0, 190);
  M5.Lcd.setTextSize(1); //size 2 to 8
  M5.Lcd.setTextColor(LIGHTGREY, BLACK);
  M5.Lcd.printf("T.read:%d ms, Loop:%d ms FPS: %3.1f\r\n", Zeit_process - Zeit_anfang, now - Zeit_anfang, 1000 / (now - Zeit_anfang) );

  //int Akku_level = m5_power.getBatteryLevel();
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.setTextSize(2); //size 2 to 8
  M5.Lcd.setTextColor(ORANGE, BLACK);
  M5.Lcd.printf("Akku: %3d%% Run:%d", Akku_level, run_cnt);



  //offset

  if (M5.BtnC.wasPressed() ) {
    buttonC_wasPressed();
    delay(100);
    Serial.println("BtnB was Pressed\r\n");
  }
  //PC_OSC_monitor_mode
  if (M5.BtnB.wasReleasefor(500)) {
    Serial.println("BtnB was long pressed over 0.5S\r\n");
    PC_OSC_monitor_mode = not PC_OSC_monitor_mode;
    Cont_Read_EN = true;
    Serial.printf("PC current monitor mode is: %d\r\n", PC_OSC_monitor_mode);

  }
  else if (M5.BtnB.wasPressed() ) {
    buttonB_wasPressed();
    delay(100);
    Serial.println("BtnB was Pressed\r\n");
  }

  //re-config chip ADC
  if (M5.BtnA.wasPressed() ) {
    buttonA_wasPressed();
    delay(100);
    Serial.println("BtnA was Pressed\r\n");
  }

  delay(loop_interval);
  if (Cont_Read_EN)
    run_cnt++;
  else {
    run_cnt = 1;
    M5.Lcd.printf("Akku: %3d%% Run:Pause", Akku_level, run_cnt);
  }
}
