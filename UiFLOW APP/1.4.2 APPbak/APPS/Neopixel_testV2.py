from m5stack import *
from m5ui import *
from uiflow import *
import unit


Flag_index_max = 6
Pixel_Flag_index = 0
Pixel_brightness =30

Color_Blue = 0x0000FF
Color_Red = 0xFF0000
Color_RedPink = 0xFF1644
Color_DeepPink = 0xFF198C
Color_Green = 0x00FF00
Color_Orange = 0xFF9500
Color_Yellow = 0xFFFF00
Color_White = 0xFFFFE0

def warning_flag():
      #clear
      neopixel0.setColorFrom(1, 64, 0x000000)
      #frame
      neopixel0.setColorFrom(2, 7, Color_Yellow)
      neopixel0.setColor(9, Color_Yellow)
      neopixel0.setColor(16, Color_Yellow)
      neopixel0.setColor(17, Color_Yellow)
      neopixel0.setColor(24, Color_Yellow)
      neopixel0.setColor(25, Color_Yellow)
      neopixel0.setColor(32, Color_Yellow)
      neopixel0.setColor(34, Color_Yellow)
      neopixel0.setColor(39, Color_Yellow)
      neopixel0.setColor(42, Color_Yellow)
      neopixel0.setColor(47, Color_Yellow)
      neopixel0.setColor(51, Color_Yellow)
      neopixel0.setColor(54, Color_Yellow)
      neopixel0.setColorFrom(60, 61, Color_Yellow)
      #!
      neopixel0.setColorFrom(12, 13, Color_Red)
      neopixel0.setColorFrom(20, 21, Color_Red)
      neopixel0.setColorFrom(28, 29, Color_Red)
      neopixel0.setColorFrom(36, 37, Color_Red)
      neopixel0.setColorFrom(52, 53, Color_RedPink)
      wait(0.5)
      neopixel0.setColorFrom(12, 13, 0x000000)
      neopixel0.setColorFrom(20, 21, 0x000000)
      neopixel0.setColorFrom(28, 29, 0x000000)
      neopixel0.setColorFrom(36, 37, 0x000000)
      neopixel0.setColorFrom(52, 53, 0x000000)
      wait(0.5)
      #neopixel0.setColorFrom(1, 64, 0x000000)


def Right_mark():
      #page 1
      neopixel0.setColor(5, 0xffcc20)
      neopixel0.setColor(14, 0xffcc20)
      neopixel0.setColor(23, 0xffcc20)
      neopixel0.setColorFrom(25, 32, 0xffcc00)
      neopixel0.setColor(39, 0xffcc20)
      neopixel0.setColor(46, 0xffcc20)
      neopixel0.setColor(53, 0xffcc20)
      wait(0.5)
      neopixel0.setColorFrom(1, 64, 0x000000)
      #page 2
      neopixel0.setColorFrom(1, 64, 0x00f0f0)
      wait(0.5)
      neopixel0.setColorFrom(1, 64, 0x000000)

def buttonA_wasPressed():
  # global params
  global Pixel_brightness
  Pixel_brightness = Pixel_brightness -10
  if (Pixel_brightness<0) : 
      Pixel_brightness =0
  
  
  neopixel0.setBrightness(Pixel_brightness)
  pass

def buttonC_wasPressed():
  # global params
  global Pixel_brightness
  Pixel_brightness = Pixel_brightness + 10
  if (Pixel_brightness>255) : 
      Pixel_brightness =255
  
  
  neopixel0.setBrightness(Pixel_brightness)
  pass

def buttonB_wasPressed():
  # global params
  global Pixel_Flag_index
  Pixel_Flag_index = Pixel_Flag_index + 1
  if (Pixel_Flag_index>Flag_index_max) : 
      Pixel_Flag_index =0
  
  
  neopixel0.setBrightness(Pixel_brightness)
  pass


run_cnt = 0
setScreenColor(0x111111)
neopixel0 = unit.get(unit.NEOPIXEL, unit.PORTB, 64)

label0 = M5TextBox(6, 11, "64 Pixel LED demo v2.1", lcd.FONT_DejaVu24,0xFFFFFF, rotate=0)
     #lcd.set_fg(lcd.WHITE)
GUI_label_BR = M5TextBox(6, 60, "Pixel brightness: 30", lcd.FONT_DejaVu18 ,0xF0DE2A, rotate=0)
GUI_label_IDX = M5TextBox(6, 100, "Patten idx: 0", lcd.FONT_DejaVu18,0xF0DEAA, rotate=0)
label_cnt = M5TextBox(220, 225, "CNT: ", lcd.FONT_Default,0xFFEE4B, rotate=0)

while True:
  
    # Btn A: reset offset values
  if btnA.isPressed():
    buttonA_wasPressed() 
  #Btn B: reset Max value
  if btnB.isPressed():
    buttonB_wasPressed() 
  if btnC.isPressed():
    buttonC_wasPressed() 
    
    
  if Pixel_Flag_index == 0:
     warning_flag()
  elif Pixel_Flag_index == 1:
     Right_mark()
  elif Pixel_Flag_index == 2:
     neopixel0.setColorFrom(1, 64, Color_White)
     wait(0.25)
  elif Pixel_Flag_index == 3:
     neopixel0.setColorFrom(1, 64, Color_Green)
     wait(0.25)
  elif Pixel_Flag_index == 4:
     neopixel0.setColorFrom(1, 64, Color_Red)
     wait(0.25)
  elif Pixel_Flag_index == 5:
     neopixel0.setColorFrom(1, 64, Color_Blue)
     wait(0.25)
  elif Pixel_Flag_index == 6:
     neopixel0.setColorFrom(1, 64, Color_Orange)
     wait(0.25)   
  else:
     neopixel0.setColorFrom(1, 64, 0x000000)
     
     
  GUI_label_BR.setText("Pixel brightness: "+str(Pixel_brightness) )
  GUI_label_IDX.setText("Patten idx: "+str(Pixel_Flag_index) )
  run_cnt = run_cnt+1
  label_cnt.setText("Run: "+str(run_cnt) )
  
  #if run_cnt % 10 == 0 :
  #   label_cnt.setText("Run: "+str(run_cnt) )
  #   GUI_label_BR.setText("Pixel brightness: "+str(Pixel_brightness) )
     



