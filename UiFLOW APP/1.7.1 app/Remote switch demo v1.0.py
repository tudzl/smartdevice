#working version 2021.4.12, tested with M5 Fire 706444E0
from m5stack import *
from m5ui import *
from uiflow import *
remoteInit()
setScreenColor(0x222222)
rgb.setColorAll(0xffffff)

BN = None
switch = None
LED_brightness = None
sys_cnt =0

label0_switch = M5TextBox(20, 100, "Text", lcd.FONT_Ubuntu, 0xFFFFFF, rotate=0)
label1_Brightness = M5TextBox(20, 160, "Text", lcd.FONT_Ubuntu, 0xFFFF22, rotate=0)



def _remote_LightSwitch():
  global switch, LED_brightness, BN 
  switch = 1 - switch

def _remote_Brightness(BN):
  global switch, LED_brightness 
  LED_brightness = BN
  rgb.setBrightness(LED_brightness)

setScreenColor(3)
lcd.setBrightness(30)
switch = 0
BN = 20
LED_brightness = BN
rgb.setBrightness(LED_brightness)
lcd.qrcode('http://flow-remote.m5stack.com/?remote=578974715498987520', 90, 32, 176)
label0_switch.setText(str(switch))
while True:
  label0_switch.setText(str(switch))
  label1_Brightness.setText(str(LED_brightness))
  #Brightness = x
  sys_cnt =sys_cnt+1
  if (sys_cnt%50 ==0):
      print('label0_switch:'+str(switch))
      print('label1_Brightness:'+str(LED_brightness))
      print('Sys run cnt:'+str(sys_cnt))
      #lcd.qrcode('http://flow-remote.m5stack.com/?remote=578974715498987520', 72, 32, 176)
      wait_ms(500)
      #setScreenColor(0x222222)
  wait_ms(20)

# Describe this function...

# Describe this function...
