#ifndef MAX44009_h
#define MAX44009_h
#include <Arduino.h>
#include <Wire.h>

#define MAX_ADDR 0x4B

/*
Lux = (2(exponent) x mantissa) x 0.045

The IC’s output (registers 0x03 and 0x04) comprises a
12-bit result that represents the ambient light expressed
in units of lux.

*/
class MAX44009 {
    public:
		MAX44009();
		
		int begin();
		bool begin2(uint8_t addr);        
		float get_lux(void);
		
};



#endif
