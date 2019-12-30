#include <Wire.h>
#include "LPS35HW.h"

const uint8_t address = 0x5D;  // select different address, default is 0x5C

LPS35HW lps(address);

void setup() {
    Serial.begin(9600);
    Serial.println("LPS barometer low power test");

    if (!lps.begin()) {
        Serial.println("Could not find a LPS barometer, check wiring!");
        while (1) {}
    }

    lps.setLowPower(true);
    lps.setOutputRate(LPS35HW::OutputRate_OneShot);  // get results on demand
    // lps.setLowPassFilter(LPS35HW::LowPassFilter_ODR9);  // default is off
}

void loop() {
    lps.requestOneShot();  // important to request new data before reading

    float pressure = lps.readPressure();  // hPa
    float temp = lps.readTemp();  // °C

    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.print("hPa\ttemperature: ");
    Serial.print(temp);
    Serial.println("*C\n");

    delay(5000);
}
