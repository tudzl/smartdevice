#Smart device 6-Axel ACC test code  2019.7.25
# need position compensation
# V3 improve vibration algorithm, add ball move2 app page
# v2.2 bug fix: linear direction error of ball gui
# version 2.1, add Ball GUI, debugs
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
ACC_total = 0
Vibration = 0
Tilt_angel = 0.0
run_cnt = 0
# offset renew
def buttonA_wasPressed():
  # global params
  global ACC_X_offset
  global ACC_Y_offset
  global ACC_Z_offset
  if get_bmm150_status(): 
    speaker.tone(346, 120, 1)
    wait(0.2)
    label0.setText("GRYO reset Offet: OK")
    ACC_X_offset = imu0.acceleration[0]
    ACC_Y_offset = imu0.acceleration[1]
    ACC_Z_offset = imu0.acceleration[2]
  pass
#reset ACC_Max and Bremsen_Max
def buttonB_wasPressed():
  # global params
  global ACC_Max
  global Bremsen_Max
  global ball_GUI2_EN
  speaker.tone(546, 120, 1)
  label0.setText("Max reset : OK      ")
  wait(0.1)
  ACC_Max = 0
  Bremsen_Max = 0
  ball_GUI2_EN = 1-ball_GUI2_EN
  pass


def buttonC_wasPressed():
  # global params
  global ball_GUI_EN
  ball_GUI_EN = 1 - ball_GUI_EN
  speaker.tone(646, 120, 1)
  
  setScreenColor(0x222222) # clear LCD
  if ball_GUI_EN :
    # draw out lines
     #M5Title(title="Ball 1", x=1 , fgcolor=0xff9900, bgcolor=0x1F1FAF)
     lcd.circle(120, 120, 116, lcd.WHITE, 0x332222)  # draw
     lcd.circle(120, 120, 115, lcd.WHITE, 0x332222)  # draw
     lcd.circle(120, 120, 114, lcd.WHITE, 0x332222)  # draw
    # draw middle lines
     lcd.circle(120, 120, 75, lcd.WHITE-100, 0x332222)  # draw
     lcd.circle(120, 120, 74, lcd.WHITE-100, 0x332222)  # draw
    # draw middle lines
     lcd.circle(120, 120, 42, lcd.WHITE-100, 0x332222)  # draw
     lcd.circle(120, 120, 41, lcd.WHITE-100, 0x332222)  # draw
     #lcd.set_fg(lcd.WHITE)
     GUI_label_1 = M5TextBox(235, 2, "", lcd.FONT_Ubuntu ,0xF0DEAA, rotate=0)
     GUI_label_2 = M5TextBox(235, 200, "", lcd.FONT_Ubuntu ,0xF0DEAA, rotate=0)
     GUI_label_1.setText("G Max:") # may dissapear
     GUI_label_2.setText("Bremsen:") # may dissapear
     #GUI_label_1.setText("G Max:")
     lcd.font(lcd.FONT_Default)
     lcd.print("0.4G" , 150, 122)
     lcd.print("0.7G" , 180, 108)
     lcd.print("1 G" , 240, 115)
     lcd.image(235,55, 'res/m.jpg') #test
  else:
     M5Title(title="     BMW G meter demo  2020.3", x=3 , fgcolor=0xff9900, bgcolor=0x1F1FAF)
  pass
# ball move according to gravity force 
def G_ball_move(x, y, color,fillcolor):
  #200*200 pixels rect
    global ball_pos
    global ballcolor_shade
    
    Radis=8
    if x > 100:
        x = 100
    elif x < -100:
        x = -100
    if y > 100:
        y = 100
    elif y < -100:
        y = -100
    # center offset
    x += 120
    y += 120
    xp = math.floor(x)
    yp = math.floor(y)
    if (not xp == ball_pos[0]) or (not yp == ball_pos[1]):
       #lcd.rect(x, y, w, h, [clr, fillcolor]) draw a rectangle with coordinates x, y, width w, height h, color clr, fill color fillcolor. If clr is not specified, it will use the current text color. If fillcolor is not specified, the figure is shaded;
        lcd.circle(ball_pos[0], ball_pos[1], Radis+1, 0x332222, 0x332222)# clean last ball drawing
        #lcd.circle(ball_pos[0]-Radis-1, ball_pos[1]-Radis-1, Radis*2+2, Radis*2+2, 0x332222, 0x332222)  # clean last ball drawing
        lcd.circle(ball_pos[0], ball_pos[1], 4, ballcolor_shade, ballcolor_shade)  # draw shades of last posistion
        lcd.circle(xp, yp, Radis, color, fillcolor)  # draw
        #renew position array
        ball_pos[0] = xp
        ball_pos[1] = yp
    lcd.line(120, 4, 120, 116+116 ,lcd.WHITE-60) 
    lcd.line(4, 120, 116+116, 120 ,lcd.WHITE-60) 
    
    
def G_ball_move2(ACC_X, ACC_Y,Tilt_angel, ACC_total,color2,fillcolor):
  #200*200 pixels rect
    global ball_pos
    global ballcolor_shade
    #global Tilt_angel
    #global ACC_total
    Radis=7
    
    if (Tilt_angel > 15):
      return
    #elif (ACC_total < 1.1):
      #return
    x = -ACC_X*100
    y = -ACC_Y*100
    if x > 100:
        x = 100
    elif x < -100:
        x = -100
    if y > 100:
        y = 100
    elif y < -100:
        y = -100
    # center offset
    x += 120
    y += 120
    xp = math.floor(x)
    yp = math.floor(y)
    if (not xp == ball_pos[0]) or (not yp == ball_pos[1]):
       #lcd.rect(x, y, w, h, [clr, fillcolor]) draw a rectangle with coordinates x, y, width w, height h, color clr, fill color fillcolor. If clr is not specified, it will use the current text color. If fillcolor is not specified, the figure is shaded;
        lcd.circle(ball_pos[0], ball_pos[1], Radis+1, 0x332222, 0x332222)# clean last ball drawing
        #lcd.circle(ball_pos[0]-Radis-1, ball_pos[1]-Radis-1, Radis*2+2, Radis*2+2, 0x332222, 0x332222)  # clean last ball drawing
        lcd.circle(ball_pos[0], ball_pos[1], 4, ballcolor_shade, ballcolor_shade)  # draw shades of last posistion
        lcd.circle(xp, yp, Radis, color2, fillcolor)  # draw
        #renew position array
        ball_pos[0] = xp
        ball_pos[1] = yp
    lcd.line(120, 4, 120, 116+116 ,lcd.WHITE-60) 
    lcd.line(4, 120, 116+116, 120 ,lcd.WHITE-60) 

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
    
    
setScreenColor(0x222222)
#rfid0 = unit.get(unit.RFID, unit.PORTA)

lcd.image(lcd.CENTER, lcd.CENTER, 'res/BMW_next.jpg') #test
time.sleep(1.5) 
lcd.image(lcd.CENTER, lcd.CENTER, 'res/frame_B.jpg') #test
time.sleep(1) 
setScreenColor(0x222222)
lcd.setBrightness(25)
#rfid0 = unit.get(unit.RFID, unit.PORTA)
title = M5Title(title="     BMW G meter demo  2020.3", x=3 , fgcolor=0xff9900, bgcolor=0x1F1FAF)


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

GUI_label_1 = M5TextBox(235, 2, "", lcd.FONT_Ubuntu ,0xF0DEAA, rotate=0)
GUI_label_1_content = M5TextBox(215, 22, "", lcd.FONT_DejaVu24,0xAFFFAF, rotate=0)
GUI_label_2 = M5TextBox(235, 200, "", lcd.FONT_Ubuntu ,0xF0DEAA, rotate=0)
GUI_label_2_content = M5TextBox(220, 180, "", lcd.FONT_DejaVu18,0xF0CE9A, rotate=0)
#ball move:
ball_GUI_EN = 0
ball_GUI2_EN = 0
ball_pos = [0, 0]
ballcolor= lcd.RED
ballcolor2 = lcd.GREEN
ballcolor_f=0xFFF549
ballcolor_f2= 0xFF5630
ballcolor_shade= 0x5EAEDF
#------------ADC V akku G35
adc = machine.ADC(35) 
adc.atten(adc.ATTN_11DB)

ACC_hor = 0.0
ACC_Max = 0
Bremsen_Max = 0
if get_bmm150_status(): 
 label0.setText("GRYO status: OK")
 wait(0.5)
 ACC_X_offset = imu0.acceleration[0]
 ACC_Y_offset = imu0.acceleration[1]
 ACC_Z_offset = imu0.acceleration[2]
 Tilt_angel=tilt_calc(ACC_X_offset,ACC_Y_offset,ACC_Z_offset)
 if (Tilt_angel>3):
      label0.setText("Offset compensation failed")
 else:
      label0.setText("Offset compensation OK")
 wait(0.05)
 #average
 ACC_X_offset = (imu0.acceleration[0]+ACC_X_offset)/2
 ACC_Y_offset = (imu0.acceleration[1]+ACC_Y_offset)/2
 ACC_Z_offset = (imu0.acceleration[2]+ACC_Z_offset)/2

while True:
  
  # Btn A: reset offset values
  if btnA.isPressed():
    buttonA_wasPressed() 
  #Btn B: reset Max value
  if btnB.isPressed():
    buttonB_wasPressed() 
  if btnC.isPressed():
    buttonC_wasPressed()   
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
      Vibration = (ACC_X**2+ACC_Y**2+ACC_Z**2-ACC_Z_offset**2)
      if Vibration>0:
         Vibration = Vibration**0.5
      else:
         Vibration = (-Vibration)**0.5
      
      #tilt angel
      Tilt_angel=tilt_calc(ACC_X,ACC_Y,ACC_Z)
      if (Tilt_angel<10):
        ACC_hor = (ACC_X**2+ACC_Y**2)**0.5
      
      #M5stack head if negative Y axel
      if (ACC_hor>ACC_Max) and (ACC_Y < 0)  :
           ACC_Max = ACC_hor
      if (ACC_hor>Bremsen_Max) and (ACC_Y > 0)  :
           Bremsen_Max = ACC_hor

      #lcd.print("Run: "+str(run_cnt)), 220, 200, 0xCCCCCC)
  except:
      label_data_T_CPU.setText("Unknown error occurred")
      gc.collect()
      time.sleep(0.1) 
      pass
  # ball indication app page
  if ball_GUI_EN == 1:
    
    if ball_GUI2_EN == 1:
       #M5Title(title="Ball 2", x=1 , fgcolor=0xff9900, bgcolor=0x1F1FAF)
       G_ball_move2(ACC_X, ACC_Y,Tilt_angel,ACC_total, ballcolor2,ballcolor_f)
    else:
       x = ACC_X*100
       y = ACC_Y*100
       G_ball_move(x, y, ballcolor,ballcolor_f)
       
    GUI_label_1_content.setText("%.3f" %(ACC_Max)+" G")
    GUI_label_2_content.setText("%.3f" %(Bremsen_Max)+" G")
  else:
      lcd.print("Acc_X: "+"%.3f" % (ACC_X)+" ", 20, 60, 0xFFAAAA)
      lcd.print("Acc_Y: "+"%.3f" % (ACC_Y)+" ", 20, 80, 0xFFAAAA)
      lcd.print("Acc_Z: "+"%.3f" % (ACC_Z)+" ", 20, 100, 0xFFAAAA)
      #lcd.print("Vibration: "+"%.3f" %(Vibration)+" m/s^2", 20, 120, 0xFFEEBB)
      lcd.print("Acc_Hor: "+"%.3f" %(ACC_hor)+"; Tilt: "+"%.1f"%(Tilt_angel), 20, 144, 0xFFEEEE)
      if ACC_Max > Bremsen_Max:
         label1.setText("Acc_Max: "+"%.3f" %(ACC_Max)+" G")
      else:   
         label1.setText("Acc_Max: "+"%.3f" %(Bremsen_Max)+" G")
      #label_Yaw.setText("Yaw:"+"%.2f" %(yaw)+" roll:"+"%.2f" %(roll)+" pitch:"+"%.2f" %(pitch))
      #label_Yaw.setText("roll:"+"%.2f" %(roll)+" pitch:"+"%.2f" %(pitch))
      label_Yaw.setText("XAS:"+str(yaw)+" YAS:"+str(roll)+" ZAS:"+str(pitch)) ## in use
      #.setText("XAS:"+"%.2f" %(yaw)+"YAS:"+"%.2f" %(roll)+" ZAS:"+"%.2f" %(pitch))
      #label_tmp.setText("tmp:"+"%.3f" %(tmp))
      label_Vib.setText("Vibration:"+"%.3f" %(Vibration)+"m/s^2")
      label_sys.setText("Free HEAP: "+str(gc.mem_free())+" Bytes" )
      tmp_str= str ("%.1f" % ((esp32.raw_temperature() -32)/1.8))
      label_data_T_CPU.setText("T_CPU: "+tmp_str )
  
  run_cnt = run_cnt+1
  label_cnt.setText("Run: "+str(run_cnt) )
  if run_cnt % 10 == 0 :
      Vb_data = adc.readraw()
      ratio =2
      Vb= Vb_data/4096*1.05*3.55*ratio
      Vb="%.3f" % Vb
      #Vb= Vb_data/4096*2.5
      label_akku.setText("Akku: "+str(Vb)+"V")

  gc.collect()
  #wait_ms(1)# about 6 FPS @ 5 ms