/*************************************************************
  Author: Ling ZELL,
  Release date: 2020.02.10
  Version: 1.3,
  M5stack launch APPArduinoJson 6!https://arduinojson.org/v6/assistant/
  data source: https://github.com/BlankerL/DXY-2019-nCoV-Data
  https://github.com/BlankerL/DXY-2019-nCoV-Data
  Original old program Author: Ironpanda(铁熊，陈众贤）
  E-mail：xyxybot@gmail.com
  WeChat：chen_zhongxian
  Release date: 2020.02.07
  Version: 0.1 (ArduinoJson 5)
  Description: Using Arduino HandBit or ESP32 to read the nCoV data

 *************************************************************
  Download latest Mixly with portable Arduino IDE here:
    https://mixly.readthedocs.io/zh_CN/latest/basic/02Installation-update.html

  Follow my WeChat Official Accounts: ironpando
  Scan the QR Code here:
    https://ae01.alicdn.com/kf/Hc425e6dfae404d8c832d2f4f752cba876.jpg

 *************************************************************/
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
//#include <U8g2lib.h>  //OLED
#include <Wire.h>
#include <SimpleTimer.h>//https://github.com/jfturcot/SimpleTimer/
//for use as bin app lovyan03
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
SimpleTimer timer;
SimpleTimer timer2;

// 根据选用的显示屏，注释相应的程序
// 1.3寸OLED12864显示屏
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// 0.96寸OLED12864显示屏
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

const char *ssid = "Staubli_SD";
const char *password = "smartdevice";
String apiUrl = "http://lab.isaaclin.cn/nCoV/api/area?latest=1&province=浙江省";
String apiUrl_all = "http://lab.isaaclin.cn/nCoV/api/overall";
//String apiUrl = "https://lab.isaaclin.cn/nCoV/api/area?latest=1&province=浙江省";
//return msg: {"results": [{"country": "中国", "provinceName": "浙江省", "provinceShortName": "浙江", "confirmedCount": 1075, "suspectedCount": 0, "curedCount": 185, "deadCount": 0, "cities": [{"cityName": "温州", "confirmedCount": 448, "suspectedCount": 0, "curedCount": 70, "deadCount": 0, "locationId": 330300}, {"cityName": "杭州", "confirmedCount": 165, "suspectedCount": 0, "curedCount": 41, "deadCount": 0, "locationId": 330100}, {"cityName": "宁波", "confirmedCount": 146, "suspectedCount": 0, "curedCount": 13, "deadCount": 0, "locationId": 330200}, {"cityName": "台州", "confirmedCount": 138, "suspectedCount": 0, "curedCount": 25, "deadCount": 0, "locationId": 331000}, {"cityName": "金华", "confirmedCount": 53, "suspectedCount": 0, "curedCount": 13, "deadCount": 0, "locationId": 330700}, {"cityName": "嘉兴", "confirmedCount": 38, "suspectedCount": 0, "curedCount": 1, "deadCount": 0, "locationId": 330400}, {"cityName": "绍兴", "confirmedCount": 38, "suspectedCount": 0, "curedCount": 8, "deadCount": 0, "locationId": 330600}, {"cityName": "丽水", "confirmedCount": 17, "suspectedCount": 0, "curedCount": 5, "deadCount": 0, "locationId": 331100}, {"cityName": "衢州", "confirmedCount": 15, "suspectedCount": 0, "curedCount": 3, "deadCount": 0, "locationId": 330800}, {"cityName": "湖州", "confirmedCount": 10, "suspectedCount": 0, "curedCount": 1, "deadCount": 0, "locationId": 330500}, {"cityName": "舟山", "confirmedCount": 7, "suspectedCount": 0, "curedCount": 5, "deadCount": 0, "locationId": 330900}], "comment": "", "updateTime": 1581217050180, "createTime": null, "modifyTime": null}], "success": true}
//country-->provinceName-->cities[cityName,confirmedCount,suspectedCount,curedCount,deadCount...]
//全国疫情数据变量
int CN_confirmedCount;
int CN_suspectedCount;
int CN_curedCount;
int CN_deadCount;
float Death_rate = 0;
float Cured_rate = 0;

// 浙江疫情数据变量
int zj_confirmedCount;
int zj_suspectedCount;
int zj_curedCount;
int zj_deadCount;

// 杭州疫情数据变量
int hangzhou_confirmedCount;
int hangzhou_suspectedCount;
int hangzhou_curedCount;
int hangzhou_deadCount;

// 宁波疫情数据变量
int ningbo_confirmedCount;
int ningbo_suspectedCount;
int ningbo_curedCount;
int ningbo_deadCount;

//台州
int taizhou_confirmedCount;
int taizhou_suspectedCount;
int taizhou_curedCount;
int taizhou_deadCount;

// 温州疫情数据变量
int wenzhou_confirmedCount;
int wenzhou_suspectedCount;
int wenzhou_curedCount;
int wenzhou_deadCount;


unsigned long run_cnt = 0;
const unsigned int wifi_Timeout = 15000; //12s

void setup() {
  M5.begin();
  //Serial.begin(115200);
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("##-->:Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

  Serial.println(F("<<<M5stack Novel coronavirus pneumonia analyzer>>>"));
  Serial.println(F("<<<Firmware Version 1.2, ling zhou, 09.2.2020>>>"));

  //m5_power.begin();
  M5.Lcd.setBrightness(30);
  M5.Lcd.clear(BLACK);

  M5.Lcd.drawJpgFile(SD, "/NCoV_BG.jpg");
  delay(300);
  M5.Lcd.setCursor(0, 200);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.println("  M5stack NCP analyzer");
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.println("  Firmware V1.1, Zell");


  // M5.Lcd.drawJpgFile(SD, "/NCoV_BG.jpg", 20, 30, 40, 50);

  /*
    // 初始化OLED显示屏
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
    u8g2.setFontPosTop();
    u8g2.clearDisplay();
  */
  //delay(200);
  Serial.println("Start WiFi now!");
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.print("  WiFi try to connect:");
  M5.Lcd.print(ssid);
  Serial.print("Wifi try Connecting to ");
  Serial.println(ssid);


  // 连接网络
  WiFi.begin(ssid, password);
  long Zeit_anfang = millis();
  long now;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    M5.Lcd.print(".");

    now = millis();
    //Zeit = now -Zeit_anfang;

    //time out break of while
    if ( (now - Zeit_anfang) > wifi_Timeout) {
      Serial.printf("wifi_Timeout: %d ms \r\n", now - Zeit_anfang);
      break;
    }
  }
  Serial.printf("time used so far: %d ms \r\n", now - Zeit_anfang);
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Local IP: ");
  M5.Lcd.print(WiFi.localIP());
  M5.Lcd.println("    ");
  //M5.Lcd.fillRect(0, 19, 320, 40, BLACK);


  // 设置定时器  10s
  //timer.setInterval(10000L, timerCallback);

  // 设置定时器  30s
  timer2.setInterval(30000L, timerCallback);

  Serial.println("Main code start running now! Data Source:");
  Serial.println(apiUrl);
  delay(200);
  M5.Lcd.drawJpgFile(SD, "/virus_BG.jpg");
  delay(300);
  timerCallback();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(0xEEEE, BLACK);
  M5.Lcd.println("M5stack NCP(n-Cov) demo");
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.println("  Firmware V1.3, Zell");
}

void loop() {
  timer2.run();
  //timer2.run();
  //  if (run_cnt == 1) {
  //    //increase timer interval
  //    timer.setInterval(20000L, timerCallback);
  //  }
}

// 定时器回调函数
void timerCallback() {
  run_cnt++;

  M5.Lcd.setCursor(0, 220);
  Serial.println("#:Timer callback: updating data now!");
  Serial.printf("#:Run:%d\r\n",  run_cnt);

  M5.Lcd.setCursor(0, 220);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(0xfbe4, BLACK); //ORANGE 0xfbe4
  M5.Lcd.printf("  Run: %d        ",  run_cnt);

  HTTPClient http;
  http.begin(apiUrl);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    M5.Lcd.setCursor(200, 220);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.println("Zell      ");
    Serial.println("#:http.GET() return OK");
    String results = http.getString();
    //Serial.println(results); // 网页内容 test ok
    //arduinojson v5
    /*
      DynamicJsonBuffer jsonBuffer(512);
      //des
      JsonObject& resultsJson = jsonBuffer.parseObject(results);

      if (!resultsJson.success()) {
      Serial.println("parseObject() failed");
      return;
      }
    */

    DynamicJsonDocument  resultsJson(3084); //jsonBuffer  2451,,Input length: 1581


    DeserializationError Deserialization_error = deserializeJson(resultsJson, results);//deserializeJson(doc, input);
    if (Deserialization_error) {
      Serial.println("Json deserializeJson() failed, with code: ");
      Serial.println(Deserialization_error.c_str()); //NoMemory if DynamicJsonDocument size not enough
      return;
    }

    // -------- 浙江省数据 -----------
    JsonObject provinces = resultsJson["results"][0];  //v5:JsonObject& provinces

    const char* country = provinces["country"]; // "中国"
    const char* provinceName = provinces["provinceName"]; // "浙江省"
    const char* provinceShortName = provinces["provinceShortName"]; // "浙江"
    zj_confirmedCount = provinces["confirmedCount"];
    zj_suspectedCount = provinces["suspectedCount"];
    zj_curedCount = provinces["curedCount"];
    zj_deadCount = provinces["deadCount"];

    // -------- cities -----------
    JsonArray cities = provinces["cities"];


    //    JsonObject results_0_cities_0 = results_0_cities[0];
    //    const char* results_0_cities_0_cityName = results_0_cities_0["cityName"]; // "温州"
    //    int results_0_cities_0_confirmedCount = results_0_cities_0["confirmedCount"]; // 448
    //    int results_0_cities_0_suspectedCount = results_0_cities_0["suspectedCount"]; // 0
    //    int results_0_cities_0_curedCount = results_0_cities_0["curedCount"]; // 70
    //    int results_0_cities_0_deadCount = results_0_cities_0["deadCount"]; // 0
    //    long results_0_cities_0_locationId = results_0_cities_0["locationId"]; // 330300


    // -------- 温州数据 -----------
    JsonObject wenzhou = cities[0];
    const char* wenzhou_cityName = wenzhou["cityName"]; // "宁波"
    wenzhou_confirmedCount = wenzhou["confirmedCount"];
    wenzhou_suspectedCount = wenzhou["suspectedCount"];
    wenzhou_curedCount = wenzhou["curedCount"];
    wenzhou_deadCount = wenzhou["deadCount"];


    // -------- 杭州数据 -----------
    JsonObject hangzhou = cities[1];
    const char* hangzhou_cityName = hangzhou["cityName"]; // "杭州"
    hangzhou_confirmedCount = hangzhou["confirmedCount"];
    hangzhou_suspectedCount = hangzhou["suspectedCount"];
    hangzhou_curedCount = hangzhou["curedCount"];
    hangzhou_deadCount = hangzhou["deadCount"];

    // -------- 宁波数据 -----------
    JsonObject ningbo = cities[2];
    const char* ningbo_cityName = ningbo["cityName"]; // "宁波"
    ningbo_confirmedCount = ningbo["confirmedCount"];
    ningbo_suspectedCount = ningbo["suspectedCount"];
    ningbo_curedCount = ningbo["curedCount"];
    ningbo_deadCount = ningbo["deadCount"];


    //overall data
    http.begin(apiUrl_all);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String results2 = http.getString();
      DeserializationError Deserialization_error2 = deserializeJson(resultsJson, results2);//deserializeJson(doc, input);
      if (Deserialization_error2) {
        Serial.println("second Json deserializeJson() failed, with code: ");
        Serial.println(Deserialization_error2.c_str()); //NoMemory if DynamicJsonDocument size not enough
        return;
      }
      JsonObject Nation = resultsJson["results"][0];
      CN_confirmedCount = Nation["confirmedCount"];
      CN_suspectedCount = Nation["suspectedCount"];
      CN_curedCount = Nation["curedCount"];
      CN_deadCount = Nation["deadCount"];


    }
    else {
      Serial.println("Http GET 2 Error.");
    }




    //M5 LCD GUI
    M5_LCD_display_data();

    // -------- 串口打印实时疫情信息 -----------
    Serial.println("浙江省新型肺炎疫情实时数据");

    Serial.println("-----------------------------------------");
    Serial.print(provinceName);
    Serial.print("：\t确诊：");
    Serial.print(zj_confirmedCount);
    Serial.print("\t\t治愈：");
    Serial.print(zj_curedCount);
    Serial.print("\t\t死亡：");
    Serial.println(zj_deadCount);

    Serial.print(hangzhou_cityName);
    Serial.print("：\t确诊：");
    Serial.print(hangzhou_confirmedCount);
    Serial.print("\t\t治愈：");
    Serial.print(hangzhou_curedCount);
    Serial.print("\t\t死亡：");
    Serial.println(hangzhou_deadCount);

    Serial.print(wenzhou_cityName);
    Serial.print("：\t确诊：");
    Serial.print(ningbo_confirmedCount);
    Serial.print("\t\t治愈：");
    Serial.print(ningbo_curedCount);
    Serial.print("\t\t死亡：");
    Serial.println(ningbo_deadCount);

    Serial.print(ningbo_cityName);
    Serial.print("：\t确诊：");
    Serial.print(ningbo_confirmedCount);
    Serial.print("\t\t治愈：");
    Serial.print(ningbo_curedCount);
    Serial.print("\t\t死亡：");
    Serial.println(ningbo_deadCount);
    Serial.println("-----------------------------------------");
    Serial.println("全国新型肺炎疫情实时数据：");
    Serial.print("\t确诊：");
    Serial.print(CN_confirmedCount);
    Serial.print("\t\t治愈：");
    Serial.print(CN_curedCount);
    Serial.print("\t\t死亡：");
    Serial.println(CN_deadCount);
    Serial.print("\t\t治愈率：");
    Serial.print(Cured_rate);
    Serial.print("\t\t死亡率：");
    Serial.println(Death_rate);
    Serial.println();


  } else {
    Serial.println("Http GET Error.");
    M5.Lcd.setCursor(140, 220);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(PINK, BLACK); //ORANGE 0xfbe4
    M5.Lcd.print("Http GET Error.");
  }

  http.end();
  /*
    // OLEDdisplay
    u8g2.firstPage();
    do
    {
    displayData();
    }
    while (u8g2.nextPage());
  */

}

void M5_LCD_display_data() {

  // 绘制表格
  //M5.Lcd.drawFrame(0, 16, 128, 48);
  //  M5.Lcd.drawLine(0, 32, 127, 32, WHITE);
  //  M5.Lcd.drawLine(0, 48, 127, 48, WHITE);
  //  M5.Lcd.drawLine(32, 16, 32, 63, WHITE);
  //  M5.Lcd.drawLine(64, 16, 64, 63, WHITE);
  //  M5.Lcd.drawLine(96, 16, 96, 63, WHITE);

  // 表格类别， head
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(RED, BLACK);
  M5.Lcd.setCursor(60, 25);
  M5.Lcd.print("Confirmed");
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setCursor(180, 25);
  M5.Lcd.print("Cured");
  //M5.Lcd.setTextColor(0x3a59, BLACK); //dark blue
  M5.Lcd.setTextColor(BLUE, BLACK); //
  M5.Lcd.setCursor(260, 25);
  M5.Lcd.print("Death");

  //font
  //M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(GREENYELLOW, BLACK);

  // 浙江情况
  M5.Lcd.setCursor(2, 42);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print("Zhejiang:");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(60, 60);
  M5.Lcd.print(zj_confirmedCount);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(180, 50);
  M5.Lcd.print(zj_curedCount);
  M5.Lcd.setCursor(260, 50);
  M5.Lcd.print(zj_deadCount);

  // 杭州情况
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(2, 80);
  M5.Lcd.print("Hangzhou:");
  M5.Lcd.setCursor(60, 100);
  M5.Lcd.print(hangzhou_confirmedCount);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(180, 90);
  M5.Lcd.print(hangzhou_curedCount);
  M5.Lcd.setCursor(260, 90);
  M5.Lcd.print(hangzhou_deadCount);
  // 温州情况
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(0xa254, BLACK);
  M5.Lcd.setCursor(2, 120);
  M5.Lcd.print("WenZhou:");
  M5.Lcd.setCursor(60, 140);
  M5.Lcd.print(wenzhou_confirmedCount);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(180, 130);
  M5.Lcd.print(wenzhou_curedCount);
  M5.Lcd.setCursor(260, 130);
  M5.Lcd.print(wenzhou_deadCount);
  // ningbo情况
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(0xEFEF, BLACK);
  M5.Lcd.setCursor(2, 160);
  M5.Lcd.print("China:");
  M5.Lcd.setCursor(60, 180);
  M5.Lcd.print(CN_confirmedCount);

  M5.Lcd.setCursor(180, 180);
  M5.Lcd.print(CN_curedCount);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(260, 170);
  M5.Lcd.print(CN_deadCount);
  Death_rate = float(CN_deadCount / CN_confirmedCount) * 100.0f;
  Cured_rate = float(CN_curedCount / CN_confirmedCount) * 100.0f;
  M5.Lcd.setCursor(180, 195);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.printf("%2.1f", Cured_rate);
  M5.Lcd.print("%");

  M5.Lcd.setCursor(260, 195);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(BLUE, BLACK);
  M5.Lcd.printf("%2.1f", Death_rate);
  M5.Lcd.print("%");
}

/*
  // 在OLED上显示实时疫情数据
  void displayData() {
  u8g2.clearDisplay();

  // 绘制表格
  u8g2.drawFrame(0, 16, 128, 48);
  u8g2.drawLine(0, 32, 127, 32);
  u8g2.drawLine(0, 48, 127, 48);
  u8g2.drawLine(32, 16, 32, 63);
  u8g2.drawLine(64, 16, 64, 63);
  u8g2.drawLine(96, 16, 96, 63);

  // 标题
  u8g2.setCursor(18, 2);
  u8g2.print("浙江疫情实时信息");

  // 表格类别
  u8g2.setCursor(36, 20);
  u8g2.print("确诊");
  u8g2.setCursor(68, 20);
  u8g2.print("治愈");
  u8g2.setCursor(100, 20);
  u8g2.print("死亡");

  // 浙江情况
  u8g2.setCursor(4, 36);
  u8g2.print("浙江");
  u8g2.setCursor(38, 36);
  u8g2.print(zj_confirmedCount);
  u8g2.setCursor(72, 36);
  u8g2.print(zj_curedCount);
  u8g2.setCursor(104, 36);
  u8g2.print(zj_deadCount);

  // 杭州情况
  u8g2.setCursor(4, 52);
  u8g2.print("杭州");
  u8g2.setCursor(40, 52);
  u8g2.print(hangzhou_confirmedCount);
  u8g2.setCursor(72, 52);
  u8g2.print(hangzhou_curedCount);
  u8g2.setCursor(104, 52);
  u8g2.print(hangzhou_deadCount);
  }

*/
