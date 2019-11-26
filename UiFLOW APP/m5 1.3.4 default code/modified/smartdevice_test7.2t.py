#Smart device test code based on weather station demo
# version 1.02, added run cnt, altitude calculation,, 2019.7.2
#Author ling zhou, last edit 26.06.2019
from micropython import const
import gc
from m5stack import *
from m5ui import *
from uiflow import *
import  unit 
import  machine
from mstate import *
import i2c_bus

LED_EN = 0
#toggle RGB led
def buttonC_wasPressed():
  # global params
  global LED_EN
  speaker.tone(1046, 120, 1)
  LED_EN =1 -LED_EN
  wait(0.1)
  pass
def buttonA_wasPressed():
  # global params
  speaker.tone(660, 120, 1)
  rgb.setBrightness(0)
  wait(0.1)
  pass


setScreenColor(0x000000)
#lcd.image(lcd.CENTER, lcd.CENTER, 'img/combitac_logo_b.jpg')
#time.sleep(1) 
lcd.image(lcd.CENTER, lcd.CENTER, 'res/combitac_logo_c.jpg')
#time.sleep(1) 
#lcd.image(lcd.CENTER, lcd.CENTER, 'image_app/p2.jpg')
#lcd.image(lcd.CENTER, lcd.CENTER, 'image_app/ghost_in_the_shell.jpg')
#lcd.image(lcd.CENTER, lcd.CENTER, 'res/error.jpg')
time.sleep(1) 
#wait(1) #OK!
setScreenColor(0x1111111)

#lcd.image(lcd.CENTER, lcd.CENTER, 'img/dot.jpg')
#time.sleep(0.5)
env0 = unit.get(unit.ENV, unit.PORTA)


title = M5Title(title="   Smart Device ESP32 demo", x=3 , fgcolor=0xff9900, bgcolor=0x1F1F1F)
circle4 = M5Circle(250, 50, 19, 0x1111111, 0x1111111)#sun
circle0 = M5Circle(87, 98, 20, 0xFFFFFF, 0xFFFFFF)#5 clouds, 0,1,2,3,5
circle1 = M5Circle(116, 97, 32, 0xFFFFFF, 0xFFFFFF)
circle2 = M5Circle(95, 109, 20, 0xFFFFFF, 0xFFFFFF)
circle3 = M5Circle(66, 109, 20, 0xFFFFFF, 0xFFFFFF)
circle5 = M5Circle(143, 109, 20, 0xFFFFFF, 0xFFFFFF)
circle_T = M5Circle(308, 92, 3, 0xFFFF00, 0x1111111)#degree
circle_T.setBorderColor(0xFFFF00)
circle_T.setBgColor(0x1111111)
label0 = M5TextBox(20, 30, "Stäubli HZ MTC L.Zhou 2019", lcd.FONT_Default,0xAFFFFF, rotate=0)
label0 = M5TextBox(176, 100, "Temp:", lcd.FONT_Default,0xFFFF00, rotate=0)
label1 = M5TextBox(176, 130, "Air P:", lcd.FONT_Default,0xAAFFFF, rotate=0)
label1B = M5TextBox(176, 155, "Alt.:", lcd.FONT_Default,0xAAFFBB, rotate=0)

#label11 = M5TextBox(280 163, "hpa", lcd.FONT_Default,0xAAFFFF, rotate=0)  #bug??
label2 = M5TextBox(176, 180, "Hum:", lcd.FONT_Default,0xAAAAFF, rotate=0)
#values from sensors
label3 = M5TextBox(225, 90, "T", lcd.FONT_DejaVu24,0xFFFF00, rotate=0)
label4 = M5TextBox(225, 130, "A", lcd.FONT_Default,0xAAFFFF, rotate=0)
label_alt = M5TextBox(225, 155, "Al", lcd.FONT_Default,0xAAFFBB, rotate=0)
label5 = M5TextBox(225, 180, "H", lcd.FONT_Default,0xAAAAFF, rotate=0)
#Vb values
label6 = M5TextBox(20, 220, "Text", lcd.FONT_Default,0xFFFFAA, rotate=0)
label_cnt = M5TextBox(220, 220, "CNT: ", lcd.FONT_Default,0xFFEEBB, rotate=0)
#label7 = M5TextBox(280 163, "hpa", lcd.FONT_Default,0xAAFFFF, rotate=0) 
#label7 = M5TextBox(280 163, "hpa", lcd.FONT_Default,0xAAFFFF, rotate=0)
rect3 = M5Rect(70, 132, 1, 2, 0xFFFFFF, 0xFFFFFF)
rect4 = M5Rect(91, 132, 1, 2, 0xFFFFFF, 0xFFFFFF)
rect5 = M5Rect(114, 132, 1, 2, 0xFFFFFF, 0xFFFFFF)
rect6 = M5Rect(138, 132, 1, 2, 0xFFFFFF, 0xFFFFFF)
run_cnt = 0
#dac = machine.DAC(25)
adc = machine.ADC(35) 
#------------ADC V akku G35
adc.atten(adc.ATTN_11DB)
#adc.width(WIDTH_10BIT)
import random

random2 = None
i = None
#--------cloud??
#m5cloud = M5Cloud()

# gc.collect()

# Why this work error, look run error..........
# gc.threshold(1000)

#m5cloud.run(thread=False)

while True:
  #------------ADC V akku G34
 #adc = machine.ADC(35)
# Per design the ADC reference voltage is 1100mV, however the true reference voltage can range from 1000mV to 1200mV amongst different ESP32s
  # value: ATTN_0DB (range 0 ~ 1.1V)
#        ATTN_2_5DB (range 0 ~ 1.5V)
#        ATTN_6DB (range 0 ~ 2.5V)
#        ATTN_11DB (range 0 ~ 3.9V)  11db==> Vi/Vo = 3.548, 1.1V*3.548=3.9
 #adc.atten(adc.ATTN_9DB)  11: 1 dB, 10: 6 dB, 01: 3 dB, 00: 0 dB,
# value: WIDTH_9BIT - capture width is 9Bit
#        WIDTH_10BIT - capture width is 10Bit
#        WIDTH_11BIT - capture width is 11Bit
#        WIDTH_12BIT - capture width is 12Bit
#adc.width( WIDTH_12BIT)
# Read the ADC value as voltage (in mV)
  if btnC.isPressed():
    buttonC_wasPressed() 
  if btnA.isPressed():
    buttonA_wasPressed() 
  Vb_data = adc.readraw()
  ratio =2
  Vb= Vb_data/4096*1.05*3.55*ratio
  Vb="%.3f" % Vb
  #Vb= Vb_data/4096*2.5
  label6.setText("Akku: "+str(Vb)+"V")
  #label6.setText(str(Vb_data)+" :"+str(Vb)+"V")
 #obj['adc'] = adc
  label3.setText(str(env0.temperature))
  #label3.setText(str(env0.temperature)+" '")
  label4.setText("%.1f" % env0.pressure+" hpa")
  Altitude = 8.5*(1013.25-env0.pressure) # in meter @ 25 degree
  Altitude = Altitude + (env0.temperature-22)*0.2273# compensation for temperature, 
  #https://www.mide.com/pages/air-pressure-at-altitude-calculator
  label_alt.setText("%.1f" % Altitude+" M")
  #label6.setText(str(Vb_data)+" V")
 #obj['adc'] = adc
  #label4.setText(str(env0.pressure)+"hpa")
  #label5.setText(str(env0.humidity)+" %")
  label5.setText("%.1f" % (env0.humidity)+" %")
  wait(0.1)
  if (env0.humidity) >= 55:
    circle4.setBgColor(0x1111111)
    circle4.setBorderColor(0x1111111)
    #rgb.setColorAll(0x000099)
    circle0.setBgColor(0xCCCCCC)
    circle1.setBgColor(0xCCCCCC)
    circle2.setBgColor(0xCCCCCC)
    circle3.setBgColor(0xCCCCCC)
    circle5.setBgColor(0xCCCCCC)
    circle0.setBorderColor(0xCCCCCC)
    circle1.setBorderColor(0xCCCCCC)
    circle2.setBorderColor(0xCCCCCC)
    circle3.setBorderColor(0xCCCCCC)
    circle5.setBorderColor(0xCCCCCC)
    rect3.setBorderColor(0x000099)
    rect4.setBorderColor(0x3366ff)
    rect5.setBorderColor(0x33ccff)
    rect6.setBorderColor(0x66ffff)
    random2 = random.randint(2, 50)
    rect3.setSize(height=random2)
    random2 = random.randint(2, 50)
    rect4.setSize(height=random2)
    random2 = random.randint(2, 50)
    rect5.setSize(height=random2)
    random2 = random.randint(2, 50)
    rect6.setSize(height=random2)
        #---------------breath RGB LED based on humidity and temperature
    #humidity: left rgb leds
    if (LED_EN):
     R = 11
     G = 55
     B = int (100+ 2*(env0.humidity-50))
    #temp right leds
    #Rt = int (env0.temperature/50*255)
     Rt = int (100+ 20*(env0.temperature-20))
     if Rt>255:
      Rt=255
     Gt = 55
     Bt = 0
     for i in range(10, 180, 10):
      rgb.setColorFrom(6 , 10 ,(R << 16) | (G << 8) | B)
      rgb.setColorFrom(1 , 5 ,(Rt << 16) | (Gt << 8) | Bt)
      rgb.setBrightness(i)
      wait_ms(1)
     for i in range(180, 10, -10):
      rgb.setColorFrom(6 , 10 ,(R << 16) | (G << 8) | B)
      rgb.setColorFrom(1 , 5 ,(Rt << 16) | (Gt << 8) | Bt)
      rgb.setBrightness(i)
      wait_ms(10)
     #fade time step
  else: #sunny
    circle0.setBgColor(0xBCF0FF)
    circle1.setBgColor(0xBCF0FF)
    circle2.setBgColor(0xBCF0FF)
    circle3.setBgColor(0xBCF0FF)
    circle5.setBgColor(0xBCF0FF)
    circle0.setBorderColor(0xBCF0FF)
    circle1.setBorderColor(0xBCF0FF)
    circle2.setBorderColor(0xBCF0FF)
    circle3.setBorderColor(0xBCF0FF)
    circle5.setBorderColor(0xBCF0FF)
    rect3.setBorderColor(0x1F1F1F)
    rect4.setBorderColor(0x1F1F1F)
    rect5.setBorderColor(0x1F1F1F)
    rect6.setBorderColor(0x1F1F1F)
    circle4.setBgColor(0xff6600)
    #rgb.setColorAll(0xff6600)
        #---------------breath RGB LED based on humidity and temperature
    #humidity: left rgb leds
    R = 11
    G = 55
    B = int (100+ 2*(env0.humidity-50))
    #temp right leds
    #Rt = int (env0.temperature/50*255)
    Rt = int (100+ 20*(env0.temperature-20))
    if Rt>255:
      Rt=255
    Gt = 55
    Bt = 0
    for i in range(10, 180, 10):
     rgb.setColorFrom(6 , 10 ,(R << 16) | (G << 8) | B)
     rgb.setColorFrom(1 , 5 ,(Rt << 16) | (Gt << 8) | Bt)
     rgb.setBrightness(i)
     wait_ms(1)
    for i in range(180, 10, -10):
     rgb.setColorFrom(6 , 10 ,(R << 16) | (G << 8) | B)
     rgb.setColorFrom(1 , 5 ,(Rt << 16) | (Gt << 8) | Bt)
     rgb.setBrightness(i)
     wait_ms(10)
     #fade time step
    for i in range(20, 31):
      lcd.circle(250, 51, i, color=0xCC9000)
      lcd.circle(250, 51, (i - 1), color=0x221100)
      wait(0.05)
    lcd.circle(250, 51, 30, color=0x221100)
  run_cnt = run_cnt+1
  label_cnt.setText("Run: "+str(run_cnt) )  
  wait_ms(50)







