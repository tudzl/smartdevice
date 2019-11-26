#Smart device MQTT test code 
# version 1.1, add sensor data throgh Wifi+MQTT OK! , 2019.7.22, freez??
# version 1.0, Wifi+MQTT OK! , 2019.7.3, have bugs run freez??
#written by zl, test ok
from m5stack import *
from m5ui import *
from uiflow import *
from m5mqtt import M5mqtt
import  unit 


setScreenColor(0x222222)

m5mqtt = M5mqtt('M5_SMD01', 'test.mosquitto.org', 1883, '', '', 300)

m5mqtt = M5mqtt('M5_SMD01', 'test.mosquitto.org', 1883, '', '', 300)
import wifiCfg

wifiCfg.screenShow()
wifiCfg.autoConnect(lcdShow=True)
wait(0.1)
title = M5Title(title="  Smart Device ESP32 MQTT demo", x=3 , fgcolor=0xff9900, bgcolor=0x1F1F1F)
label_info = M5TextBox(15, 30, "Broker:test.mosquitto.org:1883 as M5_SMD01", lcd.FONT_Default,0xDDDDDD, rotate=0)
label1 = M5TextBox(15, 50, "Text", lcd.FONT_Default,0xFFFFFF, rotate=0)
label2 = M5TextBox(15, 70, "Text", lcd.FONT_DejaVu24,0xAFFFAF, rotate=0)
#env0 = unit.get(unit.ENV, unit.PORTA)

def fun_test_m5stack_zell_(topic_data):
  # global params
  label2.setText("Msg:"+str(topic_data))
  if topic_data == 'RED':
    rgb.setColorAll(0xBB0000)
  else if topic_data == 'Alarm':
    rgb.setColorAll(0xff0000)
    for i in range(10, 180, 10):
      rgb.setBrightness(i)
      wait_ms(1)
    for i in range(180, 10, -10):
      rgb.setBrightness(i)
      wait_ms(5)
  else:
    rgb.setColorAll(0x993399)
  pass
m5mqtt.subscribe(str('test_m5stack_zell'), fun_test_m5stack_zell_)

def buttonA_wasPressed():
  # global params
  m5mqtt.publish(str('test_m5stack_zell'),str('M5 is online'))
  pass
btnA.wasPressed(buttonA_wasPressed)
def buttonB_wasPressed():
  # global params
  m5mqtt.publish(str('test_m5stack_zell'),str('M5 is running'))
  pass
btnB.wasPressed(buttonB_wasPressed)

wifiCfg.doConnect('TC', 'sthz@2020')
m5mqtt.start()
label1.setText('Hello M5 MQTT start')
m5mqtt.publish(str('test_m5stack_zell'),str('M5 is online'))
