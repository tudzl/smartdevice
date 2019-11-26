# WS2812 RGB led test  v1.1
# Author l.zhou

from m5stack import *
from m5ui import *
from uiflow import *
import unit
import random
import time


#Btn C: DISPLAY rgb NUMBER control, 
def buttonC_wasPressed():
  # global params
  global rgb_num

  #speaker.tone(1046, 120)

  if rgb_num == 24:
     rgb_num = 16
  elif rgb_num == 16:
     rgb_num = 4
  elif rgb_num == 4:
     rgb_num = 24


  label1.setText("LED num: "+str(rgb_num))

  wait(0.1)
  pass







setScreenColor(0x222222)
neopixel0 = unit.get(unit.NEOPIXEL, unit.PORTB, 24)

run_cnt = 0
rgb_num =24
lcd.setBrightness(25)
Sleep_EN = 0
LCD_EN = 10
setScreenColor(0x111111)
title = M5Title(title="  ESP32 neopixel debug", x=3 , fgcolor=0xff99aa, bgcolor=0x1F1F1F)
label1 = M5TextBox(15, 50, "LED Info: 24 ", lcd.FONT_DejaVu18,0xFFFFFF, rotate=0) #status info
label_cnt = M5TextBox(220, 225, "CNT: ", lcd.FONT_Default,0xFFEEBB, rotate=0)  # run count
label_info1 = M5TextBox(20, 200, "LED loop:", lcd.FONT_Default,0xDFCF1F, rotate=0)

while True:
  if btnC.isPressed():
     buttonC_wasPressed() 
  start = time.ticks_ms() # get millisecond counter   
  random1 = random.randint(0, 255)
  random2 = random.randint(0, 255)
  random3 = random.randint(0, 255)
  R = random1
  G = random2
  B = random3
  
  #neopixel0.setColorFrom(1, 12, 0xFF0000)

  for i in range(10, 180, 10):
      rgb.setColorFrom(1 , 12 ,(R << 16) | (G << 8) | B)
      neopixel0.setColorFrom(1, int(rgb_num/2), (R << 16) | (G << 8) | B)
      neopixel0.setColorFrom(1+int(rgb_num/2), int(rgb_num), (G << 16) | (B << 8) | R)
      #rgb.setColorFrom(1 , 5 ,(R << 16) | (G << 8) | B)
      neopixel0.setBrightness(i)
      rgb.setBrightness(i)
      wait_ms(1)
  for i in range(180, 10, -10):
      rgb.setColorFrom(1 , 12 ,(R << 16) | (G << 8) | B)
      neopixel0.setColorFrom(1, int(rgb_num/2), (R << 16) | (G << 8) | B)
      neopixel0.setColorFrom(1+int(rgb_num/2), int(rgb_num), (G << 16) | (B << 8) | R)
      #rgb.setColorFrom(1 , 5 ,(R << 16) | (G << 8) | B)
      rgb.setBrightness(i)
      neopixel0.setBrightness(i)
      wait_ms(10)

  delta_IIC = time.ticks_diff(time.ticks_ms(), start) # compute time difference
  label_info1.setText("LED loop time: "+str(delta_IIC)+" ms")  #takes about 2ms
  wait_ms(200)
  run_cnt = run_cnt+1
  label_cnt.setText("Run: "+str(run_cnt) )
     #fade time step
