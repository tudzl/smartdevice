#Smart device RFID and TF write test code author: ling zhou

# version 1.4, added SD mount check, 2019.7.24
# version 1.30, added module insert check, 2019.7.15
# version 1.20, added UOS,GC,RTC, 2019.7.8
from m5stack import *
from m5ui import *
from uiflow import *
import unit
import  machine
import network
import gc
import uos
from machine import RTC
import utime
import esp32
import esp

setScreenColor(0x222222)
title = M5Title(title="     ESP32 RFID demo V1.3  2019.7", x=3 , fgcolor=0xff9900, bgcolor=0x1F1FAF)
IIC_Address_RFID = 0X28


def settime():

   #(2019, 2, 1, 11, 45, 38, 4, 32)
   t=time()
   import utime
   tm = utime.localtime(t)
   return tm




#rfid0 = unit.get(unit.RFID, unit.PORTA)
rtc = machine.RTC()
#save some power
wlan = network.WLAN(network.STA_IF) # create station interface
wlan.active(False) 

#tm = utime.localtime(t)
#(year, month, day[, hour[, minute[, second[, microsecond[, tzinfo]]]]])
#RTC.init(2019, 7, 8[,9[,0[,0[,0[,0]]]]])
RFID_Module_insert =0
rtc.init((2019, 7, 8, 9, 0, 0, 0, 0))
label0 = M5TextBox(20, 40, "NFC status:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label_time = M5TextBox(20, 20, "TIme: ", lcd.FONT_Default,0xDFCFCF, rotate=0)
label_data = M5TextBox(20, 60, "RFID data:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label_data_hall = M5TextBox(20, 100, "Hall sensor:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label_data_T_CPU = M5TextBox(180, 100, "T_CPU:", lcd.FONT_Default,0xAFCFFF, rotate=0)
label_info = M5TextBox(20, 160, "info:", lcd.FONT_Default,0xDFCF1F, rotate=0)
label_info_t = M5TextBox(20, 140, "info:", lcd.FONT_Default,0xFFBF00, rotate=0)
#I find three rtc apis: get_time_since_boot/get_boot_time/set_boot_time.
#I find three rtc apis: get_time_since_boot/get_boot_time/set_boot_time.

try:
            rfid0 = unit.get(unit.RFID, unit.PORTA)
            label0.setText("NFC status: module OK!")

except:
            label0.setText("RFID: N.A. ")
            RFID_Module_insert = 1
            while RFID_Module_insert:
               if i2c.is_ready(IIC_Address_RFID):
                RFID_Module_insert  =0
                rfid0 = unit.get(unit.RFID, unit.PORTA)
                wait_ms(200) 
            pass


#rtc.get_boot_time()
tmp_str= str (esp.flash_size() )
label_info.setText("Flash size: "+tmp_str )
wait(1)
tmp_str= str (esp32.hall_sensor() )
label_data_hall.setText("Hall sensor: "+tmp_str )
tmp_str= str ("%.1f" % ((esp32.raw_temperature() -32)/1.8))
label_data_T_CPU.setText("T_CPU: "+tmp_str )
rtc.datetime()
#rtc.now() 
#label_info_t.setText(str rtc.now() )
# display mpy info
label_info.setText(str(uos.uname()) )
gc.collect()
wait(2)
label_info.setText("Free HEAP: "+str(gc.mem_free())+" Bytes" )


wait(2)
tmp_str = str ( uos.listdir() )
label_info_t.setText("Flash root: ")
label_info.setText(tmp_str)
#uos.listdir() # list all root files


wait(2)
#UOS api  file access
#need code to check if SD is mounted
try:
    os.mountsd()
	SD_mounted_flat = True
except:
    SD_mounted_flat = False
if SD_mounted_flat:
   path = "/sd"
   dirs = str (uos.listdir( path ) )
   #if dirs :
   label_info_t.setText("SD card root: ")
   print(str(dirs))
   label_info.setText(str(dirs) )
#else
#label_info_t.setText("Flash root: ")
   wait(2)

label_info.setText("SD card now write text file as m5stack_data.txt ")
#f = open('/sd/m5stack_data.txt', 'w+') # R+W mode?
f = open('/sd/m5stack_data.txt', 'a') # append mode?
tmp_str = str (rtc.datetime())
f.write("some test data@"+tmp_str)
f.close()
wait(2)
f = open('/sd/m5stack_data.txt', 'r')
label_info_t.setText("Read m5stack_data.txt: ")
label_info.setText(str (f.read()) )
f.close()

while True:
  tmp_str= str (esp32.hall_sensor() )
  label_data_hall.setText("Hall sensor: "+tmp_str )
  tmp_str= str ("%.1f" % ((esp32.raw_temperature() -32)/1.8))
  label_data_T_CPU.setText("T_CPU: "+tmp_str )
  if rfid0.isCardOn():
    label0.setText("NFC status: card available")
    print(rfid0.readUid())
    label_data.setText("Uid: " +str(rfid0.readUid()) )
    print(rfid0.readBlockStr(1))
    
  #f = open('/sd/m5stack_data.txt', 'r')
  #label_info_t.setText("Read m5stack_data.txt: ")
  #label_info.setText(str (f.read()) )
  #f.close()
  label_time.setText("RTC: "+str (rtc.datetime()) )
  wait_ms(200)
    

	