#test puzzle  
from m5stack import *
from m5ui import *
from uiflow import *
import unit
import imu
import math
import gc
import esp32
import  machine


Pixel_pos_pre = 6
Pixel_pos_init = 29
Pixel_brightness =30

Color_Green = 0x00FF00
Color_Orange = 0xFFA500
Color_Yellow = 0xFFFF00
Color_White = 0xFFFFE0
Color_BK = 0x000000

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
    if (unit_g < 1.25):
       tilt_angle = math.acos(z / unit_g)
    else:
       tilt_angle = math.acos(z / 1.1)
    tilt_angle = tilt_angle * 57.296 #RAD_TO_DEG
    
      
    return tilt_angle
    
    
def init_puzzle():
      #page 1
      neopixel0.setColorFrom(1, 64, 0x000000)
      neopixel0.setColor(Pixel_pos_init, Color_Yellow)
      pass
def pixel_update(pos, pos_pre):
      #page 1
      #neopixel0.setColorFrom(1, 64, 0x000000)
      neopixel0.setColor(pos_pre, Color_Yellow/2)
      neopixel0.setColor(pos, Color_Yellow/4)
      wait(0.05)
      neopixel0.setColor(pos_pre, Color_Yellow/4)
      neopixel0.setColor(pos, Color_Yellow/2)
      wait(0.05)
      neopixel0.setColor(pos_pre, Color_BK)
      neopixel0.setColor(pos, Color_Yellow)
      wait(0.15)
      pass

run_cnt = 0
setScreenColor(0x111111)
neopixel0 = unit.get(unit.NEOPIXEL, unit.PORTB, 64)
neopixel0.setBrightness(Pixel_brightness)
label0 = M5TextBox(6, 10, "Pixel puzzle demo v0.1", lcd.FONT_DejaVu24,0xFFFFFF, rotate=0)
     #lcd.set_fg(lcd.WHITE)
label1 = M5TextBox(20, 40, "IMU status:", lcd.FONT_Default,0xAFCFFF, rotate=0)
labelT = M5TextBox(20, 140, "Tilt:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label_data_T_CPU = M5TextBox(125, 225, "T_CPU:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label_cnt = M5TextBox(220, 225, "CNT: ", lcd.FONT_Default,0xFFEEBB, rotate=0)
#Vb values
label_akku = M5TextBox(20, 225, "Text", lcd.FONT_Default,0xFFFFAA, rotate=0)
label_sys = M5TextBox(20, 205, "sys: ", lcd.FONT_Default,0xF0CEAA, rotate=0)
#label_ACC = M5TextBox(20, 60, "Acc_X: ", lcd.FONT_Default,0xCFCFCF, rotate=0)
init_puzzle()
imu0 = imu.IMU()

if get_bmm150_status(): 
 label1.setText("IMU status: OK")
 wait(0.2)
 ACC_X_offset = imu0.acceleration[0]
 ACC_Y_offset = imu0.acceleration[1]
 ACC_Z_offset = imu0.acceleration[2]
 Tilt_angel=tilt_calc(ACC_X_offset,ACC_Y_offset,ACC_Z_offset)
 if (Tilt_angel>3):
      label1.setText("Offset compensation failed")
 else:
      label1.setText("Offset compensation OK")
 wait(0.05)
 
 while True:
  try:
      yaw = "%.2f" %  imu0.gyro[0] #SH200Q only have gyro?
      #yaw = imu0.ypr[0]  #??
      pitch ="%.2f" %  imu0.gyro[1]
      #pitch = imu0.ypr[1]
      roll = "%.2f" % imu0.gyro[2]
      #roll = imu0.ypr[2]
      ACC_X = imu0.acceleration[0]-ACC_X_offset
      ACC_Y = imu0.acceleration[1]-ACC_Y_offset
      ACC_Z = imu0.acceleration[2]
      Tilt_angel=tilt_calc(ACC_X,ACC_Y,ACC_Z)
      ACC_total = (ACC_X**2+ACC_Y**2+ACC_Z**2)**0.5
      lcd.FONT_Default
      lcd.print("Acc_X: "+"%.3f" % (ACC_X)+" ", 20, 60, 0xFFAAAA)
      lcd.print("Acc_Y: "+"%.3f" % (ACC_Y)+" ", 20, 80, 0xFFAAAA)
      lcd.print("Acc_Z: "+"%.3f" % (ACC_Z)+" ", 20, 100, 0xFFAAAA)
      labelT.setText("Tilt: "+str(Tilt_angel))
      label_sys.setText("Free HEAP: "+str(gc.mem_free())+" Bytes" )
      if Tilt_angel>5:
        if 
      
      
      
  except:
      label_data_T_CPU.setText("Unknown error occurred")
      gc.collect()
      time.sleep(0.1) 
      pass

  