# LPS35HW pressure sensor library (I2C)
[![Build Status](https://travis-ci.org/pilotak/LPS35HW.svg?branch=master)](https://travis-ci.org/pilotak/LPS35HW)
[![Framework Badge Arduino](https://img.shields.io/badge/framework-arduino-00979C.svg)](https://arduino.cc)

## Fully compatible with following barometers
- LPS22HB
- LPS33W
- LPS33HW

This library works with metric units, if you need imperial please consider using [MeteoFunctions](https://github.com/pilotak/MeteoFunctions) library, please see example below.

```cpp
#include <Wire.h>
#include "LPS35HW.h"
#include "MeteoFunctions.h"

LPS35HW lps;
MeteoFunctions calc;

const float above_sea = 1000;  // ft

void setup() {
    Serial.begin(9600);
    Serial.println("LPS barometer test");

    if (!lps.begin()) {
        Serial.println("Could not find a LPS barometer, check wiring!");

        while (1) {}
    }
}

void loop() {
    float pressure = lps.readPressure();
    float temp = calc.c_f(lps.readTemp());

    Serial.print("Pressure: ");
    Serial.print(calc.relativePressure_f(pressure, above_sea, temp));
    Serial.print("inHg\ttemperature: ");
    Serial.print(temp);
    Serial.println("*F\n");

    delay(1000);
}
```
