#ifndef MAX44009_h
#define MAX44009_h
#include <Arduino.h>
#include <Wire.h>

#define MAX_ADDR 0x4B

//Device Reg add
#define MAX44009_CONFIGURATION 0x02

//Device Reg values

#define MAX44009_Autolmode 0x00
#define MAX44009_manualmode 0x40
#define MAX44009_CDR 0x08  // 1/8 current ratio
#define MAX44009_TIM 0x03  // preferred mode for high brightness mode
/*
Lux = (2(exponent) x mantissa) x 0.045

The IC’s output (registers 0x03 and 0x04) comprises a
12-bit result that represents the ambient light expressed
in units of lux.

*/
class MAX44009 {
    public:
		MAX44009();
		bool manual_mode = false; // true for 0.045 LUX high resolution low intensity mode, MAX lux is limited to 2938 LUX
		int begin();
		bool begin2(uint8_t addr);       //by zl  
		float get_lux(void);
		
};



#endif
