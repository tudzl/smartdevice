from m5stack import *
from m5ui import *
from uiflow import *
remoteInit()
setScreenColor(0x222222)
lcd.setBrightness(20)
sys_cnt =0
switch = None
Brightness =20
rgb.setColorAll(0xFFFFFF)
rgb.setBrightness(Brightness)
label0_switch = M5TextBox(20, 100, "Text", lcd.FONT_Ubuntu, 0xFFFFFF, rotate=0)
label1_Brightness = M5TextBox(20, 160, "Text", lcd.FONT_Ubuntu, 0xFFFF22, rotate=0)



def _remote_LightSwitch():
  global switch 
  switch = 1-switch

def _remote_BrightnessSlide(bt):
  global switch 
  Brightness = bt
  rgb.setBrightness(Brightness)
  

setScreenColor(3)
switch = 0
lcd.qrcode('http://flow-remote.m5stack.com/?remote=578974715498987520', 72, 32, 176)
#http://flow-remote.m5stack.com/?remote=578974715498987520
#http://flow-remote.m5stack.com/?remote=undefined
label0_switch.setText(str(switch))
while True:
  label0_switch.setText(str(switch))
  label1_Brightness.setText(str(Brightness))
  #Brightness = x
  sys_cnt =sys_cnt+1
  if (sys_cnt%50 ==0):
      print('label0_switch:'+str(switch))
      print('label1_Brightness:'+str(Brightness))
      print('Sys run cnt:'+str(sys_cnt))
      #lcd.qrcode('http://flow-remote.m5stack.com/?remote=578974715498987520', 72, 32, 176)
      wait_ms(500)
      #setScreenColor(0x222222)
  wait_ms(20)

# Describe this function...

# Describe this function...
