#BH1750 test and IIC scan debug code by l.zhou
#V1.0 works ok
from m5stack import *
from uiflow import *
from m5ui import *
import  machine
import i2c_bus
import time



"""

Micropython BH1750 ambient light sensor driver.
0-65535 lux


Night: 0.001--0.02;
moonlightnight: 0.02--0.3;
cloudy indoor:5--50;
cloudy outdoor:50--500;
Sunny indoor:100--1000;
Sunny outdoor:>10000;
under the sunlight in summer afternoon: about 10*6 power; 
reading books for intensity of illumination:50--60;
home video standard intensity of illumination:1400.







"""



from utime import sleep_ms




BH1750_add = 0x23
class BH1750():

    """Micropython BH1750 ambient light sensor driver."""
     


    PWR_OFF = 0x00

    PWR_ON = 0x01

    RESET = 0x07



    # modes

    CONT_LOWRES = 0x13

    CONT_HIRES_1 = 0x10

    CONT_HIRES_2 = 0x11

    ONCE_HIRES_1 = 0x20

    ONCE_HIRES_2 = 0x21

    ONCE_LOWRES = 0x23



    # default addr=0x23 if addr pin floating or pulled to ground

    # addr=0x5c if addr pin pulled high
    # private function start with __
    def __init__(self, bus, addr=0x23):

        self.bus = bus

        self.addr = addr

        self.off()

        self.reset()



    def off(self):

        """Turn sensor off."""

        self.set_mode(self.PWR_OFF)



    def on(self):

        """Turn sensor on."""

        self.set_mode(self.PWR_ON)



    def reset(self):

        """Reset sensor, turn on first if required."""

        self.on()

        self.set_mode(self.RESET)



    def set_mode(self, mode):

        """Set sensor mode."""

        self.mode = mode

        self.bus.writeto(self.addr, bytes([self.mode]))



    def luminance(self, mode):

        """Sample luminance (in lux), using specified sensor mode."""

        # continuous modes

        if mode & 0x10 and mode != self.mode:

            self.set_mode(mode)

        # one shot modes

        if mode & 0x20:

            self.set_mode(mode)  #iic write occurs

        # earlier measurements return previous reading

        sleep_ms(24 if mode in (0x13, 0x23) else 180)

        data = self.bus.readfrom(self.addr, 2)

        factor = 2.0 if mode in (0x11, 0x21) else 1.0

        return (data[0]<<8 | data[1]) / (1.2 * factor)







#----------- main script---------------
#IIC  init
a = i2c_bus.get(i2c_bus.PORTA)

BH =BH1750(a)

    
    try:   

           #a.writeto(BH1750_add, bytes([0x20]))        # send BH1750_add 0x20, working!
           #i2c.writeto(0x42, 'hello')          # write 5 bytes to slave with address 0x42

           #time.sleep_ms(125)
           #data=a.readfrom(BH1750_add, 2)
           
           data=BH.luminance(BH1750.ONCE_HIRES_1)
           #label_iic.setText("lumi.: "+str("%.1f" % data)+"lux")  
    except:
           #label_iic.setText("IIC read: N.A. ")  
           pass

    
    
    
    




