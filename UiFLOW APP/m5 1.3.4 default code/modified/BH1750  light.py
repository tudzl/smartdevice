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








def buttonA_wasPressed():
  # global params
  global LCD_EN
  speaker.tone(446, 120, 1)

  if LCD_EN > 0:
     LCD_EN = 0
  else:
     LCD_EN = 10 
  wait(0.1)
  pass

#Btn B: DISPLAY BK light control
def buttonB_wasPressed():
  # global params
  global LCD_EN
  speaker.tone(1046, 120, 1)
  LCD_EN = LCD_EN-1
  if LCD_EN <= 0:
     LCD_EN = 20
  wait(0.1)
  pass

#Btn C: Sleep ON OFF
def buttonC_wasPressed():
  # global params
  global Sleep_EN
  global Scan_EN
  Scan_EN = not Scan_EN
  speaker.tone(646, 120, 1)
  Sleep_EN = 1-Sleep_EN
  wait(0.1)
  pass


#----------- main script---------------
BH1750_add = 0x23
lcd.setBrightness(25)
Sleep_EN = 0
LCD_EN = 10
setScreenColor(0x111111)
lcd.image(lcd.CENTER, lcd.CENTER, 'res/ghost_in_the_shell.jpg')
time.sleep(0.6) 
setScreenColor(0x111111)
title = M5Title(title="  ESP32 IIC debug, list iic addresses", x=3 , fgcolor=0xff99aa, bgcolor=0x1F1F1F)
#IIC  init
a = i2c_bus.get(i2c_bus.PORTA)
label1 = M5TextBox(10, 40, "Text", lcd.FONT_DejaVu24,0xFFFFAA, rotate=0)
label_iic = M5TextBox(10, 140,  "IIC:", lcd.FONT_Comic,0xEFEFDF, rotate=0)
lcd.image(lcd.CENTER+60, lcd.CENTER+40, 'res/light.jpg')
label_info1 = M5TextBox(20, 200, "scan loop:", lcd.FONT_Default,0xDFCF1F, rotate=0)
label_info2 = M5TextBox(20, 220, "text loop:", lcd.FONT_Default,0xDFCF1F, rotate=0)
label_info3 = M5TextBox(160, 220, "sleep:", lcd.FONT_Default,0xDFCF1F, rotate=0)
#label2 = M5TextBox(10, 100, "Text", lcd.FONT_DejaVu24,0xFFAAAA, rotate=0)

BH =BH1750(a)
Scan_EN = True

while True:
    #lcd.setBrightness(0)  
    #start = time.ticks_ms() # get millisecond counter
    if btnC.isPressed():
       buttonC_wasPressed()   
    if btnB.isPressed():
       buttonB_wasPressed()
    if btnA.isPressed():
       buttonA_wasPressed()   

    lcd.setBrightness(LCD_EN)

    start = time.ticks_us() # get millisecond counter   
    if Scan_EN :
       addrList = a.scan()

    #data = i2c1.read_u16(BH1750_add, byteorder='big')
    
    
    delta_IIC = time.ticks_diff(time.ticks_us(), start) # compute time difference
    start = time.ticks_ms() # get millisecond counter   
    l=len(addrList)
    label1.setText(str(addrList))
    #lcd.print("%02x%%" % ((addrList)), 10, 100, COLOR_GREEN)
    lcd.print("0x", 10, 80, 0xFFAAAA)
    #------display in hex format
    for i in range(0, l, 1):
     lcd.print("%x%%" % ((addrList[i])), 45+i*60, 80, 0xFFAAAA)
    lcd.font(lcd.FONT_Comic)
    lcd.print( "total addr: "+str(l), 10, 170, 0xFFAAFF)
    #lcd.print( " ".join(hex(ord(n)) for n in addrList),10, 200, COLOR_GREEN)
    #-------------time diff -----------------
    #delta = time.ticks_diff(time.ticks_ms(), start) # compute time difference
    label_t = M5TextBox(10, 20, "Current IIC address list:", lcd.FONT_Ubuntu,0xF0DFA0, rotate=0)
    label_info1.setText("IIC scan loop: "+str(delta_IIC)+" us")  #takes about 2ms
    delta = time.ticks_diff(time.ticks_ms(), start) # compute time difference
    label_info2.setText("text loop: "+str(delta)+" ms")
    
    try:   

           #a.writeto(BH1750_add, bytes([0x20]))        # send BH1750_add 0x20, working!
           #i2c.writeto(0x42, 'hello')          # write 5 bytes to slave with address 0x42

           #time.sleep_ms(125)
           #data=a.readfrom(BH1750_add, 2)
           
           data=BH.luminance(BH1750.ONCE_HIRES_1)
           label_iic.setText("lumi.: "+str("%.1f" % data)+"lux")  
    except:
           label_iic.setText("IIC read: N.A. ")  
           pass

    
    
    
    
    
    
    
    start = time.ticks_ms() # get millisecond counter   
    #machine.deepsleep(1000)# put the device to sleep for 1 seconds
    time.sleep_ms(100)
    delta = time.ticks_diff(time.ticks_ms(), start) # compute time difference
    label_info3.setText("sleep: "+str(delta)+" ms")
    #if Sleep_EN:
       #machine.deepsleep(2000)# put the device to sleep for 1 seconds
    #if machine.reset_cause() == machine.DEEPSLEEP_RESET:
    #label_info2.print('woke from a deep sleep')
    #time.sleep_ms(100)



