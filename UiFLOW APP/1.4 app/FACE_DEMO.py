#Face ENCODER module test code by L.zhou 2020.1.8
#V1.1 added RGB ring loop  display , LCD brightness display
#V1.0 basic test
#Face ENCODER module contain mega328(arduino) + 12* 3535 RGB LED 
from m5stack import *
from m5ui import *
from uiflow import *
import face
import time

# RGB LED
RGB_RING_Len = 12
RGB_RING_ON_Nr = 0
R = 10
G = 20
B = 120
Rt = 10
Gt = 10
Bt = 120
RGB_color = 0xF0F0F0

#Btn A: LCD ON/OFF control
def buttonA_wasPressed():
  # global params
  global LCD_EN
  speaker.tone(1046, 80, 1)
  LCD_EN = 1-LCD_EN
  if LCD_EN < 0:
     LCD_EN = 1
  wait(0.1)
  pass

#Btn B: reset encoder
def buttonB_wasPressed():
  # global params
  global faces_encode
  global RGB_RING_Len
  speaker.tone(666, 60, 1)
  faces_encode.clearValue()
  for i in range(0, RGB_RING_Len, 1):
        faces_encode.setLed(i, 0x000000)
  pass

#Btn C: turn off RGB_ring
def buttonC_wasPressed():
  # global params
  global RGB_RING_Len
  global faces_encode
  speaker.tone(440, 60, 1)
  for i in range(0, RGB_RING_Len, 1):
        faces_encode.setLed(i, 0x000000)

  pass









lcd.setBrightness(25)
Sleep_EN = 0
LCD_EN = 1
LCD_BK = 25
setScreenColor(0x111111)
lcd.image(lcd.CENTER, lcd.CENTER, 'res/ghost_in_the_shell.jpg')
title = M5Title(title=" Face ENCODER demo", x=3 , fgcolor=0xff99aa, bgcolor=0x1F1F1F)
label_info1 = M5TextBox(20, 200, "ENC sacn loop:", lcd.FONT_Default,0xDFCF1F, rotate=0)

label1 = M5TextBox(20,  80, "ENC",  lcd.FONT_DejaVu24,0xFFFFFF, rotate=0)
label2 = M5TextBox(20,  55, "Dir",  lcd.FONT_DejaVu24,0xFFFF00, rotate=0)
label3 = M5TextBox(180, 55, "PressBtn",  lcd.FONT_Comic,0xDFCF1F, rotate=0)
label4 = M5TextBox(180, 80, "Press_cnt",  lcd.FONT_DejaVu18,0xDFCF1F, rotate=0)

faces_encode = face.get(face.ENCODE)


#faces_encode.setLed(1, 0xff0000)
#faces_encode.clearValue()
#print(faces_encode.getPress())
#print(faces_encode.getDir())
#print(faces_encode.getValue())
press_cnt = 0
press_cnt_pre = 0
tmp_ENC = 0
tmp_ENC_pre = 0




while True:
    #lcd.setBrightness(0)  
    #start = time.ticks_ms() # get millisecond counter
    if btnC.isPressed():
       buttonC_wasPressed()   
    if btnB.isPressed():
       buttonB_wasPressed()
    if btnA.isPressed():
       buttonA_wasPressed()   
    start = time.ticks_ms() # get millisecond counter   
    tmp_ENC = faces_encode.getValue()
    tmp_DIR = faces_encode.getDir()
    tmp_press = 1-faces_encode.getPress()  #1 not pressed
    delta_IIC = time.ticks_diff(time.ticks_ms(), start) # compute time difference

    label1.setText(str(tmp_ENC))
    if tmp_DIR:
        label2.setText("CounterCW")
    else:
        label2.setText("Clockwise")

    if tmp_press:
        press_cnt = press_cnt+1
        label3.setText("Pressed!")
        time.sleep_ms(80)
    else:
        label3.setText("released")

    # RGB LED ring GUI
    if (tmp_ENC - tmp_ENC_pre )>=1 :  #increase
        RGB_RING_ON_Nr = RGB_RING_ON_Nr +1    
    elif (tmp_ENC - tmp_ENC_pre )< 0 : #decrease
        RGB_RING_ON_Nr = RGB_RING_ON_Nr -1 
        #check range
    tmp_ENC_pre = tmp_ENC
    RGB_RING_ON_Nr = max(0,RGB_RING_ON_Nr)
    RGB_RING_ON_Nr = min(12,RGB_RING_ON_Nr)
    #RGB_color = (Rt << 16) | (Gt << 8) | Bt
    for i in range(0, RGB_RING_ON_Nr, 1):
        Rt = R + i*20
        Rt = min(255,Rt)
        Gt = G + (6-i)*20
        Gt = max(0,Gt)
        Bt = B - i*10
        RGB_color = (Rt << 16) | (Gt << 8) | Bt
        faces_encode.setLed(i, RGB_color)
    for i in range(RGB_RING_ON_Nr, RGB_RING_Len,1): # set other led to off
        faces_encode.setLed(i, 0x000000)

    press_cnt_pre = press_cnt

    label_info1.setText("ENC scan loop: "+str(delta_IIC)+" ms")  #takes about 2ms
    label4.setText(str(press_cnt))
    if LCD_EN:
        lcd.setBrightness(LCD_BK)
    else:
        lcd.setBrightness(0)
    time.sleep_ms(5)