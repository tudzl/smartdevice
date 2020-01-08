#Smart device test MLX90614 demo
# version 2.1, added LCD BK , 2019.7.22
# version 2.03, added I2C detection,CPU T, 2019.7.8
# version 1.02, added run cnt, altitude calculation,, 2019.7.2
#Author ling zhou, last edit 26.06.2019
#ENV Unit:集成DHT12(环境温湿度传感器)和压强传感器BMP280; Address: 0x5C, 0x76
#T范围: 20 ~ 60℃, ±0.2
#H范围: 20 ~ 95℃  0.1%
#A范围: 300 ~ 1100hPa

from micropython import const
import gc
from m5stack import *
from m5ui import *
from uiflow import *
import  unit 
import  machine
from mstate import *
import i2c_bus
import esp32
import network
IIC_Address_MLX90614 = 0x5A
IIC_Address_ENV = 0x5C
LED_EN = 0
CPU_freq=machine.freq()/1000000
#toggle RGB led
def buttonC_wasPressed():
  # global params
  global LED_EN
  speaker.tone(1046, 120, 1)
  LED_EN =1 -LED_EN
  wait(0.1)
  pass

#turn off RGB
def buttonA_wasPressed():
  # global params
  speaker.tone(660, 120, 1)
  rgb.setBrightness(0)
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
lcd.image(lcd.CENTER, lcd.CENTER, 'res/mlx90614.jpg') #test
time.sleep(1) 

setScreenColor(0x1111111)
LCD_EN=25
#lcd.image(lcd.CENTER, lcd.CENTER, 'img/dot.jpg')
#time.sleep(0.5)


#save some power
wlan = network.WLAN(network.STA_IF) # create station interface
wlan.active(False) 


title = M5Title(title="   Smart Device ESP32 Infrad sensor demo", x=3 , fgcolor=0xff9900, bgcolor=0x1F1F1F)

circle_T = M5Circle(308, 72, 3, 0xFFFF00, 0x1111111)#degree
circle_T.setBorderColor(0xFFFF00)
circle_T.setBgColor(0x1111111)

label0 = M5TextBox(20, 30, "Stäubli HZ MTC L.Zhou 2019", lcd.FONT_Default,0xAFFFFF, rotate=0)
#label0 = M5TextBox(176, 80, "NCIRT:", lcd.FONT_Default,0xFFFF00, rotate=0)
label1 = M5TextBox(176, 130, "T_max:", lcd.FONT_Default,0xAAFFFF, rotate=0)
label1B = M5TextBox(176, 155, "T_min:", lcd.FONT_Default,0xAAFFBB, rotate=0)
#label11 = M5TextBox(280 163, "hpa", lcd.FONT_Default,0xAAFFFF, rotate=0)  #bug??
label2 = M5TextBox(176, 180, "CPU:", lcd.FONT_Default,0xAAAAFF, rotate=0)
#values from sensors
label3 = M5TextBox(225, 70, "T", lcd.FONT_DejaVu24,0xFFFF00, rotate=0)
label4 = M5TextBox(225, 130, "A", lcd.FONT_Default,0xAAFFFF, rotate=0)
label_alt = M5TextBox(225, 155, "Al", lcd.FONT_Default,0xAAFFBB, rotate=0)
label5 = M5TextBox(225, 180, "H", lcd.FONT_Default,0xAAAAFF, rotate=0)
label5.setText(str(CPU_freq)+"MHz" )
#Vb values
label6 = M5TextBox(20, 220, "Text", lcd.FONT_Default,0xFFFFAA, rotate=0)
label_data_T_CPU = M5TextBox(125, 220, "T_CPU:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label_cnt = M5TextBox(220, 220, "CNT: ", lcd.FONT_Default,0xFFEEBB, rotate=0)
label_sys = M5TextBox(20, 200, "CNT: ", lcd.FONT_Default,0xF0CEAA, rotate=0)
label_sys.setText("Free HEAP: "+str(gc.mem_free())+" Bytes" )

image_T = M5Img(20, 70, "res/NCIR_s.jpg", True)

image_T_icon = M5Img(168, 65, "res/T_icon.jpg", True)
# ------------- I2C -------------
i2c = i2c_bus.get(i2c_bus.M_BUS)
#If an error is encountered, a try block code execution is stopped and transferred down to the except block. 
#The code in the finally block will be executed regardless of whether an exception occurs.
try:
            #env0 = unit.get(unit.ENV, unit.PORTA)
            ncir0 = unit.get(unit.NCIR, unit.PORTA)

            #(ncir0.temperature)
            lcd.print("%.1f" % env.temperature+"'C", 210, 120)

except:
            label3.setText("N.A. ")
            ENV_insert = 1
            while ENV_insert:
               if i2c.is_ready(IIC_Address_MLX90614):
                ENV_insert =0
                ncir0 = unit.get(unit.NCIR, unit.PORTA)
               wait_ms(200) 
            pass



T_MLX = ncir0.temperature
T_max = T_MLX
T_min = T_MLX
#label7 = M5TextBox(280 163, "hpa", lcd.FONT_Default,0xAAFFFF, rotate=0) 
#label7 = M5TextBox(280 163, "hpa", lcd.FONT_Default,0xAAFFFF, rotate=0)

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
  if btnB.isPressed():
    buttonB_wasPressed()

  lcd.setBrightness(LCD_EN)
  Vb_data = adc.readraw()
  ratio =2
  Vb= Vb_data/4096*1.05*3.55*ratio
  Vb="%.3f" % Vb
  #Vb= Vb_data/4096*2.5
  label6.setText("Akku: "+str(Vb)+"V")
  #label6.setText(str(Vb_data)+" :"+str(Vb)+"V")
 #obj['adc'] = adc
  T_MLX = ncir0.temperature
  label3.setText(str(T_MLX))
  #label3.setText(str(env0.temperature)+" '")

  wait(0.1)
 
   
        #---------------breath RGB LED based on humidity and temperature
    #humidity: left rgb leds
  if (LED_EN):

    #temp right leds
    #Rt = int (env0.temperature/50*255)
     Rt = int (120+ 20*(T_MLX-24))
     if Rt>255:
      Rt=255
     if Rt<1:
      Rt=0
     #decrease Green as T rises   
     Gt = int ( 80-20*(T_MLX-24))
     if Gt>255:
      Gt=255
     if Gt<1:
      Gt=0
     #show blue if T is under zero
     Bt = int ( -20*(T_MLX-10))
     if Bt>255:
      Bt=255
     if Bt<1:
      Bt=0
     R = Rt
     G = Gt
     B = Bt
     for i in range(60, 180, 20):
      rgb.setColorFrom(6 , 10 ,(R << 16) | (G << 8) | B)
      rgb.setColorFrom(1 , 5 ,(Rt << 16) | (Gt << 8) | Bt)
      rgb.setBrightness(i)
      wait_ms(1)
     for i in range(180, 60, -20):
      rgb.setColorFrom(6 , 10 ,(R << 16) | (G << 8) | B)
      rgb.setColorFrom(1 , 5 ,(Rt << 16) | (Gt << 8) | Bt)
      rgb.setBrightness(i)
      wait_ms(10)
     #fade time step
  
  if (T_MLX>T_max):
      T_max = T_MLX
  if (T_MLX<T_min):
      T_min = T_MLX
  label4.setText(str(T_max))  
  label_alt.setText(str(T_min))    
  tmp_str= str ("%.1f" % ((esp32.raw_temperature() -32)/1.8))
  label_data_T_CPU.setText("T_CPU: "+tmp_str )
  run_cnt = run_cnt+1
  label_cnt.setText("Run: "+str(run_cnt) )  
  label_sys.setText("Free HEAP: "+str(gc.mem_free())+" Bytes" )
  wait_ms(50)

