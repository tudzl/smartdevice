//modified to fit for smart device v2 by ling zhou  2019.11.14
//tested works! 2019.11.14

#include <Wire.h>
#include <M5Stack.h>





//PCB related paras
const int ledPin = 2;
unsigned char blink_status = 1;
unsigned long run_cnt = 0;
const unsigned long Benchmark_count = 100 * 100000; //n* 10^6
long now = 0;
void setup()
{

  //M5.begin();
  Serial.begin(115200);

  // Wait for the Serial Monitor to open (comment out to run without Serial Monitor)
  // while(!Serial);

  Serial.println(F("smart device CPU benchmark demo"));
  Serial.println("");

  //Wire.begin();
  pinMode(ledPin, OUTPUT);

   now = millis();
}

void loop(void)
{

  blink_status = 1 - blink_status;
  digitalWrite(ledPin, blink_status);
  run_cnt++;

  //if (1 == run_cnt)  now = millis();
  if (Benchmark_count == run_cnt) {
    now = millis() - now;
    //float time = now/1000.0F;
    Serial.printf("------- System benchmark run count: %d  total time cost: %d ms  -------\r\n", run_cnt, now);
    Serial.printf("------- CPU score: %d IOPS ------\r\n", 1000 * run_cnt / now);
    

    Serial.println("");
  }
  //delay(50);
}
