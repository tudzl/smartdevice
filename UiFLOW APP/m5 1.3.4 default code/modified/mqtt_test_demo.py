from m5stack import *
from m5ui import *
from uiflow import *
# demo code from uiflow,  only for reference, not working!!!
setScreenColor(0x222222)


LCD_EN =20
lcd.setBrightness(LCD_EN)

import wifiCfg

wifiCfg.screenShow()
wifiCfg.autoConnect(lcdShow=True)



wifiCfg.doConnect('TC', 'sthz@2020')





m5mqtt = M5mqtt('M5', 'test.mosquitto.com', 8080, 'zell', '123456', 300)

#m5mqtt = M5mqtt('M5', 'test.mosquitto.com', 8080, 'zell', '123456', 300)
import wifiCfg

wifiCfg.screenShow()
wifiCfg.autoConnect(lcdShow=True)

emoji0 = Emoji(7, 7, 15, 9)


wifiCfg.doConnect('TC', 'sthz@2020')
if wifiCfg.wlan_sta.isconnected():
  adc0 = machine.ADC(36)
  emoji0.show_map([[0,0,0,0,0,0,0],[0,0,1,0,1,0,0],[0,1,0,1,0,1,0],[0,0,1,0,1,0,0],[0,0,0,1,0,0,0],[0,0,0,0,0,0,0],[0,0,0,0,0,0,0]], 0x33cc00)
  rgb.setColorAll(0x009900)
  m5mqtt.start()
  m5mqtt.publish(str('test'),str('H=50'))
else:
  emoji0.show_map([[0,0,0,0,0,0,0],[0,1,1,1,1,1,0],[0,0,1,1,1,0,0],[0,0,0,1,0,0,0],[0,0,1,1,1,0,0],[0,1,1,1,1,1,0],[0,0,0,0,0,0,0]], 0xff6600)
  rgb.setColorAll(0xff6600)
  wifiCfg.reconnect()
