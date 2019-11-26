#Smart device MQTT test code 
# version 2.1, add sensors , 2019.7.22, send 3 set of data each time
# version 2.0, Wifi+MQTT OK! , 2019.7.4, send 3 set of data each time
#written by zl, test ok
from m5stack import *
from m5ui import *
from uiflow import *
from m5mqtt import M5mqtt
import  unit 
import  machine
import math

#process MQTT received messages
def fun_test_m5stack_zell_(topic_data):
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

  if topic_data == 'RED':
    Alarm_flag = 0
    #speaker.tone(1000, 120, 1)
    rgb.setColorAll(0xBB0000)
  else:
    Alarm_flag = 0
    #speaker.tone(520, 120, 1)
    rgb.setColorAll(0x993399)
  pass


#  one time online info
def buttonA_wasPressed():
  # global params
  speaker.tone(346, 120, 1)
  m5mqtt.publish(str('test_m5stack_zell'),str('M5 is online'))
  wait(0.1)
  pass

#  toggle MQTT sending  
def buttonB_wasPressed():
  # global params
  global MQTT_data_send_flag
  speaker.tone(446, 120, 1)
  MQTT_data_send_flag = 1-MQTT_data_send_flag
  if MQTT_data_send_flag ==1:
   label1.setText('M5 MQTT start sending data...')
   m5mqtt.publish(str('test_m5stack_zell'), "Smart device ESP32_M5_Zell data:")
   wait_ms(100)
  else:
    m5mqtt.publish(str('test_m5stack_zell'), "Smart device ESP32_M5_Zell Go OFF:")
    label1.setText('M5 MQTT STOP sending data!  ')
    pass
  #MQTT_data_send_flag = 1
  m5mqtt.publish(str('test_m5stack_zell'),str('M5 MQTT is running'))
  wait(0.1)
  pass

#Btn C: DISPLAY BK light control
def buttonC_wasPressed():
  # global params
  global LCD_EN
  speaker.tone(1046, 120, 1)
  LCD_EN = LCD_EN-1
  if LCD_EN <= 0:
     LCD_EN = 20
  wait(0.1)
  pass


#------------------------------script code starts here:-------------
IIC_Address_ENV = 0x5C
setScreenColor(0x222222)
lcd.setBrightness(25)
LCD_EN = 20
res_tmp = 0
m5mqtt = M5mqtt('M5_SMD01', 'test.mosquitto.org', 1883, '', '', 300)

#m5mqtt = M5mqtt('M5_SMD01', 'test.mosquitto.org', 1883, '', '', 300)
import wifiCfg
#global control flags
Alarm_flag = 0
MQTT_data_send_flag = 0
run_cnt = 0
Msg_cnt_R =0
Msg_cnt_T =0
wifiCfg.screenShow()
wifiCfg.autoConnect(lcdShow=True)
wait(0.1)
setScreenColor(0x000000)
title = M5Title(title="  Smart Device ESP32 MQTT demo V2.1", x=3 , fgcolor=0xff9900, bgcolor=0x1F1F1F)
label_info = M5TextBox(15, 30, "test.mosquitto.org:1883 as M5_SMD01", lcd.FONT_Default,0xDDDDDD, rotate=0)
label1 = M5TextBox(15, 50, "Text", lcd.FONT_Default,0xFFFFFF, rotate=0) #status info
label2 = M5TextBox(15, 80, "Text", lcd.FONT_DejaVu24,0xAFFFAF, rotate=0) #MQTT sent msg display
label3 = M5TextBox(15, 100, "Sensor: ", lcd.FONT_DejaVu24,0xCFFF9F, rotate=0) #MQTT sent msg display
label_MQTT_R = M5TextBox(15, 120, "RX_Msg:", lcd.FONT_DejaVu24,0xFFF000, rotate=0) #MQTT received msg display
label_cnt = M5TextBox(220, 225, "CNT: ", lcd.FONT_Default,0xFFEEBB, rotate=0)  # run count
label_LCD = M5TextBox(220, 205, "LCD_BK: ", lcd.FONT_Default,0xAAEEAA, rotate=0)
label_cnt_msg = M5TextBox(20, 205, "MQTT CNT: ", lcd.FONT_Default,0xAAEEAA, rotate=0) 
image_B_icon = M5Img(140,220, "res/bnt_B.jpg", True)
#Vb values
label_akku = M5TextBox(20, 225, "Text", lcd.FONT_Default,0xFFFFAA, rotate=0)
adc = machine.ADC(35) 
ratio =2
#------------ADC V akku G35
adc.atten(adc.ATTN_11DB)

m5mqtt.subscribe(str('test_m5stack_zell'), fun_test_m5stack_zell_)
wifiCfg.doConnect('TC', 'sthz@2020')
m5mqtt.start()
label1.setText('Hello M5 MQTT start')
m5mqtt.publish(str('test_m5stack_zell'),str('M5 is now online'))

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
  if  MQTT_data_send_flag == 1:
      label2.setText("MQTT_send:"+"T.="+str(T))
      m5mqtt.publish(str('test_m5stack_zell'),"T.="+str(T))
      wait_ms(200)
      label2.setText("MQTT_send:"+"A.P.="+str(T))
      m5mqtt.publish(str('test_m5stack_zell'),"A.P.="+str(A))
      wait_ms(200)
      label2.setText("MQTT_send:"+"H.="+str(H))
      m5mqtt.publish(str('test_m5stack_zell'),"H.="+str(H))
      wait_ms(200)
      Msg_cnt_T =Msg_cnt_T+3
      
  if Alarm_flag == 1:
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
  
  label_cnt_msg.setText("MQTT Tx: "+str(Msg_cnt_T)+" Rx: "+str(Msg_cnt_R) )
  label_cnt.setText("Run: "+str(run_cnt) )
  # GUI info display  --- bugs?? 
  run_cnt = run_cnt+1
  #res_tmp= res_tmp +math.floor(run_cnt /10) 
  if run_cnt % 5 == 0 :
    Vb_data = adc.readraw()
    Vb= Vb_data/4096*1.05*3.55*ratio
    Vb="%.3f" % Vb
    #Vb= Vb_data/4096*2.5
    label_akku.setText("Akku: "+str(Vb)+"V")
    label_LCD.setText("LCD_BK: "+str(LCD_EN))
  wait_ms(200)