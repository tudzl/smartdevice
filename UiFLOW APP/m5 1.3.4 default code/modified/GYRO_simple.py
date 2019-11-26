#Smart device 3-Axel ACC test code 
# version 1.2, debug  vibration
# version 1.1, add try except bug handling code semms more stable??
# version 1.02, SH200Q  6-axel data display, 2019.7.3, have bugs run freez??
# version 1.01, added Vibration calc, 2019.7.2
#written by zl, test ok
#version 0.2, 2019.6.28
from m5stack import *
from m5ui import *
from uiflow import *
import imu
import unit
import math
import gc
import esp32
import  machine

ACC_X_offset = 0
ACC_Y_offset = 0
ACC_Z_offset = 0
ACC_Max = 0
Vibration = 0
run_cnt = 0
# offset renew
def buttonA_wasPressed():
  # global params
  global ACC_X_offset
  global ACC_Y_offset
  global ACC_Z_offset
  if get_bmm150_status(): 
    speaker.tone(1046, 120, 1)
    wait(0.2)
    label0.setText("GRYO reset Offet: OK")
    ACC_X_offset = imu0.acceleration[0]
    ACC_Y_offset = imu0.acceleration[1]
    ACC_Z_offset = imu0.acceleration[2]
  pass

def buttonB_wasPressed():
  # global params
  global ACC_Max
  speaker.tone(1046, 120, 1)
  label0.setText("Max reset : OK      ")
  wait(0.1)
  ACC_Max = 0
  pass

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
    
    
setScreenColor(0x222222)
#rfid0 = unit.get(unit.RFID, unit.PORTA)



#rfid0 = unit.get(unit.RFID, unit.PORTA)
title = M5Title(title="     ESP32 GYRO demo  2019.7", x=3 , fgcolor=0xff9900, bgcolor=0x1F1FAF)


imu0 = imu.IMU()

label0 = M5TextBox(20, 40, "GRYO status:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label1 = M5TextBox(20, 160, "Acc_Max: ", lcd.FONT_DejaVu24,0xAFFFAF, rotate=0)
label_Yaw = M5TextBox(20, 190, "Yaw: ", lcd.FONT_Default,0xCFCFCF, rotate=0)
#label_tmp = M5TextBox(20, 210, "tmp: ", lcd.FONT_Default,0xCFCFCF, rotate=0)
label_Vib = M5TextBox(20, 120, "Vib: ", lcd.FONT_DejaVu24,0xAFFFFF, rotate=0)
label_cnt = M5TextBox(220, 225, "CNT: ", lcd.FONT_Default,0xFFEEBB, rotate=0)
#Vb values
label_akku = M5TextBox(20, 225, "Text", lcd.FONT_Default,0xFFFFAA, rotate=0)
label_sys = M5TextBox(20, 205, "CNT: ", lcd.FONT_Default,0xF0CEAA, rotate=0)
#sys info indication
label_sys.setText("Free HEAP: "+str(gc.mem_free())+" Bytes" )
label_data_T_CPU = M5TextBox(125, 225, "T_CPU:", lcd.FONT_Default,0xAFCFFF, rotate=0)
adc = machine.ADC(35) 
#------------ADC V akku G35
adc.atten(adc.ATTN_11DB)

if get_bmm150_status(): 
 label0.setText("GRYO status: OK")
 wait(0.5)
 ACC_X_offset = imu0.acceleration[0]
 ACC_Y_offset = imu0.acceleration[1]
 ACC_Z_offset = imu0.acceleration[2]
 ACC_Max = 0

while True:
  Vb_data = adc.readraw()
  ratio =2
  Vb= Vb_data/4096*1.05*3.55*ratio
  Vb="%.3f" % Vb
  #Vb= Vb_data/4096*2.5
  label_akku.setText("Akku: "+str(Vb)+"V")

  # Btn A: reset offset values
  if btnA.isPressed():
    buttonA_wasPressed() 
  #Btn B: reset Max value
  if btnB.isPressed():
    buttonB_wasPressed() 
  #tmp = imu0.gyro[0]  # angel speed?
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
      ACC_total = (ACC_X**2+ACC_Y**2+ACC_Z**2)**0.5
      Vibration = (ACC_X**2+ACC_Y**2+(ACC_Z-ACC_Z_offset)**2)**0.5
      ACC_hor = (ACC_X**2+ACC_Y**2)**0.5
      if (ACC_hor>ACC_Max):
        ACC_Max = ACC_hor
      lcd.print("Acc_X: "+"%.3f" % (ACC_X)+" ", 20, 60, 0xFFAAAA)
      lcd.print("Acc_Y: "+"%.3f" % (ACC_Y)+" ", 20, 80, 0xFFAAAA)
      lcd.print("Acc_Z: "+"%.3f" % (ACC_Z)+" ", 20, 100, 0xFFAAAA)
      #lcd.print("Vibration: "+"%.3f" %(Vibration)+" m/s^2", 20, 120, 0xFFEEBB)
      lcd.print("Acc_Hor: "+"%.3f" %(ACC_hor), 20, 144, 0xFFEEEE)
      label1.setText("Acc_Max: "+"%.3f" %(ACC_Max)+" G")
      #label_Yaw.setText("Yaw:"+"%.2f" %(yaw)+" roll:"+"%.2f" %(roll)+" pitch:"+"%.2f" %(pitch))
      #label_Yaw.setText("roll:"+"%.2f" %(roll)+" pitch:"+"%.2f" %(pitch))
      label_Yaw.setText("XAS:"+str(yaw)+" YAS:"+str(roll)+" ZAS:"+str(pitch)) ## in use
      #.setText("XAS:"+"%.2f" %(yaw)+"YAS:"+"%.2f" %(roll)+" ZAS:"+"%.2f" %(pitch))
      #label_tmp.setText("tmp:"+"%.3f" %(tmp))
      label_Vib.setText("Vibration:"+"%.3f" %(Vibration)+"m/s^2")
      run_cnt = run_cnt+1
      label_cnt.setText("Run: "+str(run_cnt) )
      #lcd.print("Run: "+str(run_cnt)), 220, 200, 0xCCCCCC)
  except:
      label_data_T_CPU.setText("Unknown error occurred")
      gc.collect()
      pass
  tmp_str= str ("%.1f" % ((esp32.raw_temperature() -32)/1.8))
  label_data_T_CPU.setText("T_CPU: "+tmp_str )
  label_sys.setText("Free HEAP: "+str(gc.mem_free())+" Bytes" )
  gc.collect()
  wait_ms(1)# about 6 FPS @ 5 ms