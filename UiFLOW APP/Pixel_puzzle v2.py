#Pixel puzzle game  v 1.1
#28.03.2020 bug fixed
from m5stack import *
from m5ui import *
from uiflow import *
import unit
import imu
import math
import gc
import esp32
import  machine
import random


upside_down = False
Pixel_pos_pre = 28
Pixel_pos_init = 28
Pixel_pos =0
Pixel_target_index = 32
Pixel_row_num= 4
Pixel_col_num= 4
Pixel_brightness =30
Pixel_brightness_m = 15
Pixel_brightness_dk = 5

Color_Blue = 0x0000FF
Color_Red = 0xFF0000
Color_Green = 0x00FF00
Color_Orange = 0xFFA500
Color_Yellow = 0xFFFF00
Color_White = 0xFFFFE0
Color_BK = 0x000000
Color_RedPink = 0xFF1644
Color_DeepPink = 0xFF198C
Color_Random = 0x000000


def get_bmm150_status():
    import bmm150
    state = 0
    bmm = bmm150.Bmm150()
    if bmm.available():
        if bmm.readID() == 0x32:
            state = 1
            bmm.set_normal_mode()
            if bmm.readData()[1] == 0:
                time.sleep_ms(200)
                if bmm.readData()[1] == 0:
                    state = 0
    return state
    
def tilt_calc(x,y,z):
    unit_g =  x**2 + y**2 + z**2
    unit_g = math.sqrt(unit_g)
    if z<0:
      z= -z
    if (unit_g < 1.25):
       tilt_angle = math.acos(z / unit_g)
    else:
       tilt_angle = math.acos(z / unit_g) #need improve
    tilt_angle = tilt_angle * 57.296 #RAD_TO_DEG
    return tilt_angle
    
    
def init_puzzle(Pixel_pos,Pixel_target_index):
      #page 1
      #global Pixel_target_index
      neopixel0.setColorFrom(1, 64, 0x000000)
      neopixel0.setColor(Pixel_pos, Color_Yellow)
      neopixel0.setColor(Pixel_target_index, Color_Green)
      
      pass


def win_flag():
      #clear
      neopixel0.setColorFrom(1, 64, 0x000000)
      wait(0.05)
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
      neopixel0.setColorFrom(12, 13, Color_Red)
      neopixel0.setColorFrom(20, 21, Color_Red)
      neopixel0.setColorFrom(28, 29, Color_Red)
      neopixel0.setColorFrom(36, 37, Color_Red)
      neopixel0.setColorFrom(52, 53, Color_RedPink)
      wait(0.25)
      neopixel0.setColorFrom(1, 64, 0x000000)
      pass
      
def pixel_row_col_update(Pixel_pos):
    global Pixel_row_num, Pixel_col_num
    Pixel_row_num =int(math.floor((Pixel_pos-1)/8)+1 )
    if Pixel_pos>8:
        Pixel_col_num = int(math.fmod(Pixel_pos, 8) )
        if Pixel_col_num ==0:
           Pixel_col_num =8
    else:
        Pixel_col_num = int(Pixel_pos)
   
    pass   

def pixel_up():
    global Pixel_pos, Pixel_pos_pre
    Pixel_pos = Pixel_pos_pre -8
    

    pass    
  
def pixel_down():
    global Pixel_pos, Pixel_pos_pre
    Pixel_pos = Pixel_pos_pre +8
    

    pass    
def pixel_update(pos, pos_pre):
      #page 1
      #neopixel0.setColorFrom(1, 64, 0x000000)
      neopixel0.setBrightness(Pixel_brightness_m)
      wait(0.15)
      neopixel0.setBrightness(Pixel_brightness_dk)
      neopixel0.setColor(pos_pre, int(Color_Yellow/3))
      neopixel0.setColor(pos, int(Color_Yellow/2))
      wait(0.05)
      neopixel0.setBrightness(Pixel_brightness_m)
      neopixel0.setColor(pos_pre, int(Color_Yellow/2))
      neopixel0.setColor(pos, int(Color_Yellow/3))
      wait(0.05)
      neopixel0.setBrightness(Pixel_brightness)
      neopixel0.setColor(pos_pre, Color_BK)
      neopixel0.setColor(pos, Color_Yellow)
      wait(0.1)
      pass
    
def pixel_draw(pos, color):
      #page 1
      #neopixel0.setColorFrom(1, 64, 0x000000)
      neopixel0.setColor(pos, color)
      wait(0.05)
      pass

run_cnt = 0
setScreenColor(0x111111)
neopixel0 = unit.get(unit.NEOPIXEL, unit.PORTB, 64)
neopixel0.setBrightness(Pixel_brightness)
label0 = M5TextBox(6, 10, "Pixel puzzle demo v1.01", lcd.FONT_DejaVu24,0xFFFFFF, rotate=0)
     #lcd.set_fg(lcd.WHITE)
label1 = M5TextBox(20, 40, "IMU status:", lcd.FONT_Default,0xAFCFFF, rotate=0)
#labelT = M5TextBox(20, 140, "Tilt:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label_data_T_CPU = M5TextBox(125, 225, "T_CPU:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label_pixel = M5TextBox(20, 180, "Pixel:", lcd.FONT_Default,0xAFCF3F, rotate=0)
label_cnt = M5TextBox(220, 225, "CNT: ", lcd.FONT_Default,0xFFEEBB, rotate=0)
#Vb values
label_akku = M5TextBox(20, 225, "Akku", lcd.FONT_Default,0xFFFFAA, rotate=0)
label_sys = M5TextBox(20, 205, "sys: ", lcd.FONT_Default,0xF0CEAA, rotate=0)
labelT = M5TextBox(20, 140, "Tilt:", lcd.FONT_DejaVu24,0xAFCFFF, rotate=0)
#label_ACC = M5TextBox(20, 60, "Acc_X: ", lcd.FONT_Default,0xCFCFCF, rotate=0)
Pixel_target_index = random.randint(1, 60)
init_puzzle(Pixel_pos_init,Pixel_target_index)
Pixel_pos_pre = Pixel_pos_init
Pixel_pos = Pixel_pos_pre
imu0 = imu.IMU()

if get_bmm150_status(): 
   label1.setText("IMU status: OK")
   wait(0.2)
   ACC_X_offset = imu0.acceleration[0]
   ACC_Y_offset = imu0.acceleration[1]
   ACC_Z_offset = imu0.acceleration[2]
   Tilt_angel=tilt_calc(ACC_X_offset,ACC_Y_offset,ACC_Z_offset)
   if ACC_Z_offset<-0.5:
      upside_down = True
   if (Tilt_angel>5):
        label1.setText("Offset compensation failed")
   else:
        label1.setText("Offset compensation OK")
   wait(0.05)
 
speaker.setVolume(1)
#speaker.sing(220, 1)

while True:
      run_cnt = run_cnt+1
  #try:
      #roll = imu0.ypr[2]
      ACC_X = imu0.acceleration[0]-ACC_X_offset
      ACC_Y = imu0.acceleration[1]-ACC_Y_offset
      ACC_Z = imu0.acceleration[2]
      if ACC_Z<-0.5:
         upside_down = True
      else:
         upside_down = False
      Tilt_angel=tilt_calc(ACC_X,ACC_Y,ACC_Z)
      #ACC_total = (ACC_X**2+ACC_Y**2+ACC_Z**2)**0.5
      lcd.print("Acc_X: "+"%.3f" % (ACC_X)+" ", 20, 60, 0xFFAAAA)
      lcd.print("Acc_Y: "+"%.3f" % (ACC_Y)+" ", 20, 80, 0xFFAAAA)
      lcd.print("Acc_Z: "+"%.3f" % (ACC_Z)+" ", 20, 100, 0xFFAAAA)
      labelT.setText("Tilt: "+str(Tilt_angel))
      
      pixel_row_col_update(Pixel_pos)
      
      if Tilt_angel>12:
        #speaker.sing(220, 1)
        if ACC_X> 0.3 :
          
             if upside_down:
                if Pixel_col_num<8 :
                  Pixel_pos= Pixel_pos_pre +1
                else:
                  speaker.sing(180, 1)
             else:
                if Pixel_col_num>1 :
                  Pixel_pos= Pixel_pos_pre -1
                else:
                  speaker.sing(220, 1)
             
             
        elif ACC_X <-0.3:
           
              if upside_down:
                if Pixel_col_num>1 :
                   Pixel_pos= Pixel_pos_pre -1
                else:
                   speaker.sing(180, 1)
              else:
                if Pixel_col_num<8 :
                   Pixel_pos= Pixel_pos_pre +1
                else:
                   speaker.sing(220, 1)

          
        if ACC_Y> 0.3 :
          if Pixel_row_num<8 :
             pixel_down()
          else:
             speaker.sing(180, 1)
        elif ACC_Y< -0.3 :
            if Pixel_row_num>1 :
               pixel_up()
            else:
               speaker.sing(180, 1)
        
        #speaker.sing(220, 1)
        if Pixel_pos>64:
           Pixel_pos = 64
           speaker.sing(220, 1)
        if Pixel_pos<1:
           Pixel_pos = 1
           speaker.sing(220, 1)
           
        pixel_update(Pixel_pos, Pixel_pos_pre)  
        #pixel_draw(Pixel_pos, Pixel_pos_pre)  
        Pixel_pos_pre = Pixel_pos 
      
      
      if Pixel_pos == Pixel_target_index:
             label_pixel.setText("Pixel: "+str(Pixel_pos)+" Col:"+str(Pixel_col_num)+ "Target:"+str(Pixel_target_index))
             speaker.sing(660, 1)
             wait(0.15)
             win_flag()
             wait(0.15)
             #try:
                #contain bugs!!!
             Pixel_target_index = int (math.fmod(run_cnt,64)+1)
             Pixel_pos = int(random.randint(1, 63)+1)
             label_pixel.setText("Pixel: " + str(Pixel_pos)+ " Col:" + str(Pixel_col_num) +" Row:" + str(Pixel_row_num) + "Target:"+str(Pixel_target_index))
             #except:
               # label_data_T_CPU.setText("win funct error")
                #label1.setText("random.randint error")
                #pass
             label1.setText("Reset puzzle now!")
             init_puzzle(Pixel_pos,Pixel_target_index)
             Color_Random = int(random.randint(0x1F, 0xffffff))
             pixel_draw(Pixel_target_index, Color_Random)
             wait(0.25)
         
      gc.collect()
      label_pixel.setText("Pixel: " + str(Pixel_pos)+ " Col:" + str(Pixel_col_num) +" Row:" + str(Pixel_row_num) + "Target:"+str(Pixel_target_index))
      label_sys.setText("Free HEAP: "+str(gc.mem_free())+" Bytes" )
      label_data_T_CPU.setText("")
      label_cnt.setText("Run: "+str(run_cnt) )
  #except:
      #label_data_T_CPU.setText("Unknown error occurred")
      #time.sleep(0.1) 
      #pass

  