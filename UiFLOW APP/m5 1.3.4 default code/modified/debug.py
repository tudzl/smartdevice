#IIC scan debug code by l.zhou
#V2.0 added loop time display , LCD brightness display
from m5stack import *
from uiflow import *
from m5ui import *
import  machine
import i2c_bus
import time

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
  speaker.tone(646, 120, 1)
  Sleep_EN = 1-Sleep_EN
  wait(0.1)
  pass

lcd.setBrightness(25)
Sleep_EN = 0
LCD_EN = 10
setScreenColor(0x111111)
lcd.image(lcd.CENTER, lcd.CENTER, 'res/ghost_in_the_shell.jpg')
time.sleep(1) 
setScreenColor(0x111111)
title = M5Title(title="  ESP32 IIC debug, list iic addresses", x=3 , fgcolor=0xff99aa, bgcolor=0x1F1F1F)
a = i2c_bus.get(i2c_bus.PORTA)
label1 = M5TextBox(10, 40, "Text", lcd.FONT_DejaVu24,0xFFFFAA, rotate=0)
label_info1 = M5TextBox(20, 200, "info:", lcd.FONT_Default,0xDFCF1F, rotate=0)
label_info2 = M5TextBox(20, 220, "info:", lcd.FONT_Default,0xDFCF1F, rotate=0)
label_info3 = M5TextBox(160, 220, "info:", lcd.FONT_Default,0xDFCF1F, rotate=0)
#label2 = M5TextBox(10, 100, "Text", lcd.FONT_DejaVu24,0xFFAAAA, rotate=0)
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

    start = time.ticks_ms() # get millisecond counter   
    addrList = a.scan()
    delta_IIC = time.ticks_diff(time.ticks_ms(), start) # compute time difference
    l=len(addrList)
    label1.setText(str(addrList))
    #lcd.print("%02x%%" % ((addrList)), 10, 100, COLOR_GREEN)
    lcd.print("0x", 10, 80, 0xFFAAAA)
    #------display in hex format
    for i in range(0, l, 1):
     lcd.print("%x%%" % ((addrList[i])), 45+i*60, 80, 0xFFAAAA)
    lcd.print( "total addr: "+str(l), 10, 150, 0xFFAAFF)
    #lcd.print( " ".join(hex(ord(n)) for n in addrList),10, 200, COLOR_GREEN)
    #-------------time diff -----------------
    #delta = time.ticks_diff(time.ticks_ms(), start) # compute time difference
    label_info1.setText("IIC scan loop: "+str(delta_IIC)+" ms")  #takes about 2ms
    delta = time.ticks_diff(time.ticks_ms(), start) # compute time difference
    label_info2.setText("text loop: "+str(delta)+" ms")
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



