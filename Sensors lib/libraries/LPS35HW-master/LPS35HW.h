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

#ifndef LPS35HW_H
#define LPS35HW_H

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#include <Wire.h>

#define LPS35HW_ADDRESS 0x5d  //for smart device ncir pcb
#define LPS35HW_DEFAULT_CTRL_REG1 0b00100010  // continuous@10Hz, no low pass filter, BDU on
#define LPS35HW_DEFAULT_CTRL_REG2 0b00010000
#define LPS35HW_ID 0b10110001  // same as LPS22HB, LPS33W, LPS33HW

class LPS35HW {
  public:
    enum OutputRate {
        OutputRate_OneShot = 0b000,
        OutputRate_1Hz     = 0b001,
        OutputRate_10Hz    = 0b010,
        OutputRate_25Hz    = 0b011,
        OutputRate_50Hz    = 0b100,
        OutputRate_75Hz    = 0b101
    };

    enum LowPassFilter {
        LowPassFilter_Off   = 0b00,
        LowPassFilter_ODR9  = 0b10,
        LowPassFilter_ODR20 = 0b011,
    };

    enum Registers {
        LPS35HW_INTERRUPT_CFG = 0x0B,
        LPS35HW_THS_P_L       = 0x0C,
        LPS35HW_THS_P_H       = 0x0D,
        LPS35HW_WHO_AM_I      = 0x0F,
        LPS35HW_CTRL_REG1     = 0x10,
        LPS35HW_CTRL_REG2     = 0x11,
        LPS35HW_CTRL_REG3     = 0x12,
        LPS35HW_FIFO_CTRL     = 0x14,
        LPS35HW_REF_P_XL      = 0x15,
        LPS35HW_REF_P_L       = 0x16,
        LPS35HW_REF_P_H       = 0x17,
        LPS35HW_RPDS_L        = 0x18,
        LPS35HW_RPDS_H        = 0x19,
        LPS35HW_RES_CONF      = 0x1A,
        LPS35HW_NT_SOURCE     = 0x25,
        LPS35HW_FIFO_STATUS   = 0x26,
        LPS35HW_STATUS        = 0x27,
        LPS35HW_PRESS_OUT_XL  = 0x28,
        LPS35HW_PRESS_OUT_L   = 0x29,
        LPS35HW_PRESS_OUT_H   = 0x2A,
        LPS35HW_TEMP_OUT_L    = 0x2B,
        LPS35HW_TEMP_OUT_H    = 0x2C,
        LPS35HW_LPFP_RES      = 0x33
    };

    LPS35HW(uint8_t addr = LPS35HW_ADDRESS);
    bool begin();
    bool begin(TwoWire *theWire);
    void setOutputRate(OutputRate rate);
    void setLowPassFilter(LowPassFilter filter);
    void setLowPower(bool on);
    void requestOneShot();
    void reset();
    float readPressure();
    float readTemp();

  private:
    TwoWire *_wire;
    const uint8_t _addr;
    uint8_t _config;

    bool init();
    void writeRegister(Registers reg, uint8_t value);
    uint8_t readRegister(Registers reg);
};

#endif  // LPS35HW_H