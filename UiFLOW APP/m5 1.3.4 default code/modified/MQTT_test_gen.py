from m5stack import *
from m5ui import *
from uiflow import *
from m5mqtt import M5mqtt

setScreenColor(0x222222)

m5mqtt = M5mqtt('M5_SMD01', 'test.mosquitto.org', 1883, '', '', 300)

m5mqtt = M5mqtt('M5_SMD01', 'test.mosquitto.org', 1883, '', '', 300)
import wifiCfg

wifiCfg.screenShow()
wifiCfg.autoConnect(lcdShow=True)

label1 = M5TextBox(15, 69, "Text", lcd.FONT_Default,0xFFFFFF, rotate=0)

def fun_test_m5stack_zell_(topic_data):
  # global params
  label1.setText(str(topic_data))
  if topic_data == 'RED':
    rgb.setColorAll(0xff0000)
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
