#Smart device MQTT test code 
# version 3,4 test
# version 3.1 added alarm date sending,  speaker bugs!! unknown cause!
# version 2.1, add sensors , 2019.7.22, send 3 set of data each time
# version 2.0, Wifi+MQTT OK! , 2019.7.4, send 3 set of data each time
#written by zl, test ok
#Iot Eclipse:   iot.eclipse.org   1883
#stsmd/+/+
from m5stack import *
from m5ui import *
from uiflow import *
from m5mqtt import M5mqtt
import  unit 
import  machine
#import string   not exist
import gc
import i2c_bus
import time

#process MQTT received messages from self
def fun_echo_esp32_mqtt(topic_data):
   global Msg_cnt_R
   label_MQTT_Rs.setText("Rx_echo:"+str(topic_data) )
   Msg_cnt_R = Msg_cnt_R +1
   pass


#process MQTT received messages
def fun_smartdevice_mqtt(topic_data):
  # global params
  global Alarm_flag
  global Msg_cnt_R
  label_MQTT_R.setText("R_Msg:"+str(topic_data))
  Msg_cnt_R = Msg_cnt_R +1
  if topic_data == 'Alarm':
    Alarm_flag = 1
    #speaker.tone(2000, 120, 1)
    rgb.setColorAll(0xff0000)
    for i in range(10, 180, 10):
      rgb.setBrightness(i)
      wait_ms(1)
    for i in range(180, 10, -10):
      rgb.setBrightness(i)
      wait_ms(5)
  elif topic_data == 'RED':
    Alarm_flag = 0
    #speaker.tone(1000, 120, 1)
    rgb.setColorAll(0xCC0000)
  elif topic_data == 'BLUE':
    Alarm_flag = 0
    #speaker.tone(1000, 120, 1)
    rgb.setColorAll(0x0000EE)
  else:
    Alarm_flag = 0
    #speaker.tone(520, 120, 1)
    rgb.setColorAll(0x11EE11)
  pass


#  one time online info
def buttonA_wasPressed():
  # global params
  global Msg_cnt_T
  #speaker.setVolume(1)
  #speaker.tone(346, 120, 1)
  m5mqtt.publish(str('test_ESP32_smartdevice'),str('esp32 is online'))
  wait(0.1)
  m5mqtt.publish(str('strtd/'+Device_ID+'/online'), "Smart device is online!")
  Msg_cnt_T = Msg_cnt_T+2
  wait(0.1)
  pass

#  toggle MQTT sending  
def buttonB_wasPressed():
  # global params
  global MQTT_data_send_flag
  global Msg_cnt_T
  #speaker.setVolume(1)
  #speaker.tone(446, 120, 1)
  MQTT_data_send_flag = 1-MQTT_data_send_flag
  if MQTT_data_send_flag ==1:
   label1.setText('smartdevice sending data...')
   #m5mqtt.publish(str('test_ESP32_smartdevice'), "Smart device ESP32_M5_Zell data:")
   m5mqtt.publish(str('stsmd/'+Device_ID+'/info'), "Smart device ESP32 sending data now:")
   Msg_cnt_T = Msg_cnt_T+1
   wait_ms(100)
  else:
    m5mqtt.publish(str('test_ESP32_smartdevice'), "Smart device ESP32_M5_Zell Go OFF:")
    wait(0.1)
    m5mqtt.publish(str('strtd/'+Device_ID+'/online'), "")
    label1.setText('MQTT STOP sending  data!  ')
    Msg_cnt_T = Msg_cnt_T+2
    pass
  #MQTT_data_send_flag = 1
  m5mqtt.publish(str('test_ESP32_smartdevice'),str('ESP32 MQTT is running'))
  wait(0.1)
  pass

#Btn C: DISPLAY BK light control, MQTT_Alert_send_flag control
def buttonC_wasPressed():
  # global params
  global LCD_EN
  global MQTT_Alert_send_flag
  #speaker.setVolume(1)
  #speaker.tone(1046, 120)
  LCD_EN = LCD_EN-1
  MQTT_Alert_send_flag = 1-MQTT_Alert_send_flag
  if MQTT_Alert_send_flag ==1:
     label1.setText('Smartdevice sending alerts...')
     m5mqtt.subscribe(str('stsmd/'+Device_ID+'/alert'), fun_echo_esp32_mqtt)  #will echo received msg
  else:
     label1.setText('Smartdev stop sending alerts..')
  if LCD_EN <= 0:
     LCD_EN = 20
  wait(0.1)
  pass


#------------------------------script code starts here:-------------
gc.collect()
IIC_Address_ENV = 0x5C
setScreenColor(0x222222)
lcd.setBrightness(25)
LCD_EN = 20
res_tmp = 0


#-------------MQTT vars
Device_ID = 'ESP32_02'
Alert_msg=0x800200  # need replacement & over temperature
MQTT_Server = 'iot.eclipse.org'
Topic_ID = 'stsmd/+/+'
Data_ava_flag= 0b10000111

import wifiCfg
#MQTT lib auto did wifi connections
#wifiCfg.screenShow()
#wifiCfg.doConnect('TC', 'sthz@2020')
#wifiCfg.autoConnect(lcdShow=True)
wait(0.1)

m5mqtt = M5mqtt('M5_SMD01', MQTT_Server, 1883, '', '', 300)
#m5mqtt = M5mqtt('M5_SMD01', 'test.mosquitto.org', 1883, '', '', 300)
wait(0.1)
#global control flags
Alarm_flag = 0
MQTT_data_send_flag = 0
MQTT_Alert_send_flag = 0
run_cnt = 0
Msg_cnt_R =0
Msg_cnt_T =0
kk=[]
data_payload= 0x00
Alert_payload= 0x010101
setScreenColor(0x000000)
title = M5Title(title="  Smart Device ESP32 MQTT demo V3.4", x=3 , fgcolor=0xff9900, bgcolor=0x1F1F1F)
label_info = M5TextBox(15, 30, "<-->"+MQTT_Server+":1883 as "+Device_ID, lcd.FONT_Default,0xDDDDDD, rotate=0)
#label_info = M5TextBox(15, 30, "<-->test.mosquitto.org:1883 as M5_SMD01", lcd.FONT_Default,0xDDDDDD, rotate=0)
label1 = M5TextBox(15, 50, "Status Info", lcd.FONT_DejaVu18,0xFFFFFF, rotate=0) #status info
label2 = M5TextBox(15, 80, "MQTT_send:", lcd.FONT_Ubuntu,0xAFFFAF, rotate=0) #MQTT sent msg display
label3 = M5TextBox(15, 100, "Sensor: ", lcd.FONT_DejaVu18,0x9FFF9F, rotate=0) #MQTT sent msg display
label_MQTT_R = M5TextBox(15, 120, "RX_Msg:", lcd.FONT_DejaVu24,0xFFF000, rotate=0) #MQTT received msg display
label_MQTT_Rs = M5TextBox(15, 145, "Rx_echo:", lcd.FONT_DejaVu18,0x1F1FF2, rotate=0) #MQTT received msg display
label_cnt = M5TextBox(220, 225, "CNT: ", lcd.FONT_Default,0xFFEEBB, rotate=0)  # run count
label_LCD = M5TextBox(220, 205, "LCD_BK: ", lcd.FONT_Default,0xAAEEAA, rotate=0)
label_cnt_msg = M5TextBox(20, 205, "MQTT CNT: ", lcd.FONT_Default,0xAAEEAA, rotate=0) 
label_sys = M5TextBox(20, 185, "Heap: ", lcd.FONT_Default,0xF0CEAA, rotate=0)
image_B_icon = M5Img(140,220, "res/bnt_B.jpg", True)
#Vb values
label_akku = M5TextBox(20, 225, "Akku", lcd.FONT_Default,0xFFFFAA, rotate=0)
adc = machine.ADC(35) 
ratio =2
#------------ADC V akku G35
adc.atten(adc.ATTN_11DB)
#--------MQTT----------------
m5mqtt.subscribe(str('test_ESP32_smartdevice_server'), fun_smartdevice_mqtt)  
wait(0.1)
#m5mqtt.subscribe(str(Topic_ID), fun_echo_esp32_mqtt)  #will echo received msg
#m5mqtt.subscribe(str('stsmd/'+Device_ID+'/alert'), fun_echo_esp32_mqtt)  #will echo received msg
#wait(0.1)
#m5mqtt.subscribe(str('test_ESP32_smartdevice_server'), func_echo_esp32_mqtt)  #will echo received msg
#wifiCfg.doConnect('TC', 'sthz@2020')
m5mqtt.start()
#wait(0.1)
lcd.font(lcd.FONT_Comic)
label1.setText('Hi, SmartDevice MQTT start')
m5mqtt.publish(str('test_ESP32_smartdevice'),str('smart device is now online'))

#----------get time
#rtc = machine.RTC()
#rtc.ntp_sync(server="1.europe.pool.ntp.org", tz="CET-1CEST")
#rtc.ntp_sync(server="hr.pool.ntp.org", tz="CET-1CEST")
#rtc.synced()
#env0 = unit.get(unit.ENV, unit.PORTA)
try:
            env0 = unit.get(unit.ENV, unit.PORTA)
            label3.setText("Sensor: OK")
except:
            label3.setText("Sensor: N.A.")
            ENV_insert = 1
            while ENV_insert:
               if i2c.is_ready(IIC_Address_ENV):
                  ENV_insert =0
                  env0 = unit.get(unit.ENV, unit.PORTA)
                  label3.setText("Sensor: OK!")
                  wait_ms(200) 
            pass


while True:
  if btnC.isPressed():
    buttonC_wasPressed()   
  if btnB.isPressed():
    buttonB_wasPressed() 
  if btnA.isPressed():
    buttonA_wasPressed() 

  lcd.setBrightness(LCD_EN)
  #gather  data
  T=env0.temperature
  A=env0.pressure
  H=env0.humidity
  #Data order: Temperature Data, F: Humidity Data  D: Pressure Data, F: Vibration Data, F: Light Data
  if  MQTT_data_send_flag == 1:
      lcd.circle(220, 165, 4, lcd.ORANGE, lcd.GREEN)  # draw send_flag indication
      """
      label2.setText("MQTT_send:"+"T.="+str(T))
      m5mqtt.publish(str('test_ESP32_smartdevice'),"T.="+str(T))
      #m5mqtt.publish(str('stsmd/'+Device_ID+'/global'),"T.="+str(T))
      wait_ms(200)
      #time.sleep(0.2)
      label2.setText("MQTT_send:"+"A.P.="+str(A))
      m5mqtt.publish(str('test_ESP32_smartdevice'),"A.P.="+str(A))
      #m5mqtt.publish(str('stsmd/'+Device_ID+'/global'),"A.P.="+str(A))
      wait_ms(200)
      #time.sleep(0.2)
      label2.setText("MQTT_send:"+"H.="+str(H))
      m5mqtt.publish(str('test_ESP32_smartdevice'),"H.="+str(H))
      #m5mqtt.publish(str('stsmd/'+Device_ID+'/global'),"H.="+str(H))
      wait_ms(200)
      #time.sleep(0.2)
      Msg_cnt_T =Msg_cnt_T+3
      #Msg_cnt_T =Msg_cnt_T+3
      """
      #Data_ava_flag   0b00000111   0x07
      kk.append(Data_ava_flag)
      data_payload = chr(kk[0])
      kk.pop()
      data_payload = data_payload+str("[")+str(T)+str(",")+str(H)+str(",")+str(A)+str(",0,0] ")
      #wait_ms(200)
      m5mqtt.publish(str('stsmd/'+Device_ID+'/local'),str(data_payload))
      label2.setText("MQTT_send:"+str(data_payload))
      wait_ms(200)
      Msg_cnt_T =Msg_cnt_T+1
      
      """
      kk.append(int(T))
      kk.append(int(H))
      kk.append(int(A))
      kk.append(0)
      kk.append(0)
      data_payload="".join(map(chr,kk)) #error: can't convert float to int
      kk.pop()
      kk.pop()
      kk.pop()
      kk.pop()
      kk.pop()
      kk.pop()
      """
  else:
      lcd.circle(220, 165, 5, lcd.BLACK, lcd.BLACK)  # clean send_flag indication
      
  if Alarm_flag == 1:
    lcd.circle(220, 175, 4, lcd.RED, lcd.RED)  # draw Alarm_flag indication
    rgb.setColorAll(0xff0000)
    for i in range(10, 180, 10):
      rgb.setBrightness(i)
      #speaker.setVolume(1)
      #speaker.sing(448, 1)
      wait_ms(1)
    for i in range(180, 10, -10):
      rgb.setBrightness(i)
      #speaker.tone(2000+i, 10)
      wait_ms(5)
  else:
    lcd.circle(220, 175, 5, lcd.BLACK, lcd.BLACK)  # clean Alarm_flag indication
  
  #under tests
  if MQTT_Alert_send_flag ==1:
      #label2.setText("Alert_sent: "+"Alert:"+"%x%%" % (Alert_payload) )
      #m5mqtt.publish(str('test_ESP32_smartdevice'),"Alert:"+str(Alert_payload) )
      #Alert_msg
      #m5mqtt.publish(str('stsmd/'+Device_ID+'/alert'),str(Alert_payload_str) )
      kk.append(Alert_msg>>16)
      kk.append(0xFF&(Alert_msg>>8))
      kk.append(0xFF&Alert_msg)
      Alert_payload_str= "".join(map(chr,kk))
      kk.pop()
      kk.pop()
      kk.pop()
      m5mqtt.publish(str('stsmd/'+Device_ID+'/alert'),str(Alert_payload_str) )
      Msg_cnt_T =Msg_cnt_T+1
      label2.setText("Alert_sent: "+"Alert:0x"+"%x%%" % (Alert_msg) ) #show in hex, ok
      wait_ms(400)
      m5mqtt.publish(str('stsmd/'+Device_ID+'/alert'),str(Alert_msg) ) #only for test
      Msg_cnt_T =Msg_cnt_T+1

  label_cnt_msg.setText("MQTT Tx: "+str(Msg_cnt_T)+" Rx: "+str(Msg_cnt_R) )
  label_cnt.setText("Run: "+str(run_cnt) )
  # GUI info display  --- bugs?? 
  run_cnt = run_cnt+1
  #res_tmp= res_tmp +math.floor(run_cnt /10) 
  if run_cnt % 5 == 0 :
    #Alert info process
    Vb_data = adc.readraw()
    Vb= Vb_data/4096*1.05*3.55*ratio
    Vb="%.3f" % Vb
    #Vb= Vb_data/4096*2.5
    #m5mqtt.publish(str('stsmd/'+Device_ID+'/global'),str(data_payload))
    time.sleep(0.2)
    label_akku.setText("Akku: "+str(Vb)+"V")
    label_LCD.setText("LCD_BK: "+str(LCD_EN))
    label_sys.setText("Free HEAP: "+str(gc.mem_free())+" Bytes" )
    gc.collect()
  if run_cnt % 20 == 0 :
         m5mqtt.publish(str('stsmd/'+Device_ID+'/global'),str(data_payload))
         time.sleep(0.2)
  #wait_ms(200)
  time.sleep(0.2)
