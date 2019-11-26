import sys 
sys.path.append('flowlib/lib')
from m5ui import *
import machine, time, m5base, uiflow
from m5stack import *
__VERSION__ = m5base.get_version()

lcd.image(lcd.CENTER, lcd.CENTER, 'res/logo.jpg')
lcd.setColor(0xCCCCCC, 0)
lcd.print('UPLOAD', 40, 225)
lcd.print('APP.LIST', 130, 225)
lcd.print('SETUP', 233, 225)
lcd.print(__VERSION__, 260, 5)
lcd.setCursor(0, 0)

#Vb values
label_akku = M5TextBox(10, 5, "Text", lcd.FONT_Default,0xFFFFAA, rotate=0)
adc = machine.ADC(35) 
#------------ADC V akku G35
adc.atten(adc.ATTN_11DB)
Vb_data = adc.readraw()
ratio =2
Vb= Vb_data/4096*1.05*3.55*ratio
Vb="%.3f" % Vb
label_akku.setText("Akku: "+str(Vb)+"V")

cnt_down = time.ticks_ms() + 2000
while time.ticks_ms() < cnt_down:
    if btnA.wasPressed():
        uiflow.start('flow')
    elif btnB.wasPressed():
        from app_manage import file_choose
        file_choose() 
        uiflow.start('app')
    elif btnC.wasPressed():
        import flowSetup
        flowSetup.start()


# 0:app, 1:flow_internet, 2:debug, 3:flow use 
start = uiflow.cfgRead('start')
if start == 'app':
    m5base.app_start(0)
elif start == 'flow':
    if uiflow.cfgRead('mode') == 'usb':
        m5base.app_start(3)
    else:
        m5base.app_start(1)
else:
    m5base.app_start(2)

Vb_data = adc.readraw()
ratio =2
Vb= Vb_data/4096*1.05*3.55*ratio
Vb="%.3f" % Vb
label_akku.setText("Akku: "+str(Vb)+"V")
del start
del cnt_down