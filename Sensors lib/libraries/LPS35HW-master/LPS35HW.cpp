/*
MIT License
Copyright (c) 2019 Pavel S
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#ifdef __AVR_ATtiny85__
    #include "TinyWireM.h"
    #define Wire TinyWireM
#else
    #include <Wire.h>
#endif

#include "LPS35HW.h"

LPS35HW::LPS35HW(uint8_t addr):
    _addr(addr),
    _config(LPS35HW_DEFAULT_CTRL_REG1) {
}

bool LPS35HW::begin() {
    _wire = &Wire;
    Wire.begin();
    return init();
}


bool LPS35HW::begin(TwoWire *theWire) {
    if (theWire) {
        _wire = theWire;
        return init();
    }

    return false;
}

bool LPS35HW::init() {
    if (readRegister(LPS35HW_WHO_AM_I) == LPS35HW_ID) {
        writeRegister(LPS35HW_CTRL_REG2, LPS35HW_DEFAULT_CTRL_REG2 | 0b10000100);  // Reset and reboot
        writeRegister(LPS35HW_CTRL_REG1, _config);
        return true;
    }

    return false;
}

void LPS35HW::setOutputRate(OutputRate rate) {
    _config &= ~(0b1110000);
    _config |= ((uint8_t)rate << 4);

    writeRegister(LPS35HW_CTRL_REG1, _config);
}

void LPS35HW::setLowPassFilter(LowPassFilter filter) {
    _config &= ~(0b1100);
    _config |= ((uint8_t)filter << 2);

    writeRegister(LPS35HW_CTRL_REG1, _config);
}

void LPS35HW::setLowPower(bool on) {
    uint8_t lp = readRegister(LPS35HW_RES_CONF);
    lp ^= (-on ^ lp) & (1 << 0);
    writeRegister(LPS35HW_RES_CONF, lp);
}

void LPS35HW::requestOneShot() {
    if (((_config >> 4) & 0b111) == OutputRate_OneShot) {  // OneShot
        writeRegister(LPS35HW_CTRL_REG2, LPS35HW_DEFAULT_CTRL_REG2 | 0b1);  // New conversion
    }
}

void LPS35HW::reset() {
    writeRegister(LPS35HW_CTRL_REG2, LPS35HW_DEFAULT_CTRL_REG2 | 0b10000100);  // Reset and reboot
    writeRegister(LPS35HW_CTRL_REG1, _config);  // Load previous config
}

float LPS35HW::readPressure() {
    int32_t value = readRegister(LPS35HW_PRESS_OUT_XL);
    value |= (static_cast<int32_t>(readRegister(LPS35HW_PRESS_OUT_L)) << 8);
    value |= static_cast<int32_t>(readRegister(LPS35HW_PRESS_OUT_H)) << 16;  // Important to read as last for BDU to work

    if (value != 0xFFFFFF) {
        if (value & 0x800000) {
            value = (0xFF000000 | value);
        }

        return static_cast<float>(value) / 4096.0;
    }

    return NAN;
}

float LPS35HW::readTemp() {
    int16_t value = (readRegister(LPS35HW_TEMP_OUT_H) << 8);
    value |= readRegister(LPS35HW_TEMP_OUT_L);

    if (value != (int16_t)0xFFFF) {
        return static_cast<float>(value) / 100.0;
    }

    return NAN;
}

void LPS35HW::writeRegister(Registers reg, uint8_t value) {
    _wire->beginTransmission(_addr);
    _wire->write((uint8_t)reg);
    _wire->write(value);
    _wire->endTransmission();
}

uint8_t LPS35HW::readRegister(Registers reg) {
    uint8_t val = 0xFF;

    _wire->beginTransmission(_addr);
    _wire->write((uint8_t)reg);

    if (_wire->endTransmission() == 0) {
        _wire->requestFrom((uint8_t)_addr, (uint8_t)1);
        val = _wire->read();
    }

    return val;
}
