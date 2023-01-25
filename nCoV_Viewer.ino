/*************************************************************
  Author: Ironpanda(铁熊，陈众贤）
  E-mail：xyxybot@gmail.com
  WeChat：chen_zhongxian
  Release date: 2020.02.07
  Version: 0.1
  Description: Using Arduino HandBit or ESP32 to read the nCoV data

 *************************************************************
  Download latest Mixly with portable Arduino IDE here:
    https://mixly.readthedocs.io/zh_CN/latest/basic/02Installation-update.html

  Follow my WeChat Official Accounts: ironpando
  Scan the QR Code here:
    https://ae01.alicdn.com/kf/Hc425e6dfae404d8c832d2f4f752cba876.jpg

 *************************************************************/
 
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SimpleTimer.h>

SimpleTimer timer;

// 根据选用的显示屏，注释相应的程序
// 1.3寸OLED12864显示屏
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// 0.96寸OLED12864显示屏
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

const char *ssid = "wifi-name";
const char *password = "wifi-password";
String apiUrl = "https://lab.isaaclin.cn/nCoV/api/area?latest=1&province=浙江省";

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

void setup() {
  // 初始化OLED显示屏
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
  u8g2.setFontPosTop();
  u8g2.clearDisplay();

  Serial.begin(115200);

  // 连接网络
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  // 设置定时器
  timer.setInterval(10000L, timerCallback);
}

void loop() {
  timer.run();
}

// 定时器回调函数
void timerCallback() {
  HTTPClient http;
  http.begin(apiUrl);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {

    String results = http.getString();
    //    Serial.println(results); // 网页内容

    DynamicJsonBuffer jsonBuffer(512);
    JsonObject& resultsJson = jsonBuffer.parseObject(results);

    if (!resultsJson.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    // -------- 浙江省数据 -----------
    JsonObject& provinces = resultsJson["results"][0];

    const char* country = provinces["country"]; // "中国"
    const char* provinceName = provinces["provinceName"]; // "浙江省"
    const char* provinceShortName = provinces["provinceShortName"]; // "浙江"
    zj_confirmedCount = provinces["confirmedCount"];
    zj_suspectedCount = provinces["suspectedCount"];
    zj_curedCount = provinces["curedCount"];
    zj_deadCount = provinces["deadCount"];

    // -------- cities -----------
    JsonArray& cities = provinces["cities"];

    // -------- 杭州数据 -----------
    JsonObject& hangzhou = cities[1];
    const char* hangzhou_cityName = hangzhou["cityName"]; // "杭州"
    hangzhou_confirmedCount = hangzhou["confirmedCount"];
    hangzhou_suspectedCount = hangzhou["suspectedCount"];
    hangzhou_curedCount = hangzhou["curedCount"];
    hangzhou_deadCount = hangzhou["deadCount"];

    // -------- 宁波数据 -----------
    JsonObject& ningbo = cities[2];
    const char* ningbo_cityName = ningbo["cityName"]; // "宁波"
    ningbo_confirmedCount = ningbo["confirmedCount"];
    ningbo_suspectedCount = ningbo["suspectedCount"];
    ningbo_curedCount = ningbo["curedCount"];
    ningbo_deadCount = ningbo["deadCount"];

    // -------- 串口打印实时疫情信息 -----------
    Serial.println("浙江省新型肺炎疫情实时数据");

    Serial.println("-----------------------------------------");

    Serial.print("浙江：\t确诊：");
    Serial.print(zj_confirmedCount);
    Serial.print("\t治愈：");
    Serial.print(zj_curedCount);
    Serial.print("\t\t死亡：");
    Serial.println(zj_deadCount);

    Serial.print("杭州：\t确诊：");
    Serial.print(hangzhou_confirmedCount);
    Serial.print("\t\t治愈：");
    Serial.print(hangzhou_curedCount);
    Serial.print("\t\t死亡：");
    Serial.println(hangzhou_deadCount);

    Serial.print("宁波：\t确诊：");
    Serial.print(ningbo_confirmedCount);
    Serial.print("\t\t治愈：");
    Serial.print(ningbo_curedCount);
    Serial.print("\t\t死亡：");
    Serial.println(ningbo_deadCount);

    Serial.println();

  } else {
    Serial.println("GET Error.");
  }

  http.end();

  u8g2.firstPage();
  do
  {
    displayData();
  }
  while (u8g2.nextPage());
}


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
