# image sensor Example for StickV
# V1.2 2020.11.03  zell
# v1.1 2020.11.02  zell
# test sensor for Zeit_end = 60*1000, 60 seconds
# Welcome to the MaixPy IDE!
# 1. Conenct board to computer
# 2. Select board at the top of MaixPy IDE: `tools->Select Board`
# 3. Click the connect buttion below to connect board
# 4. Click on the green run arrow button below to run the script!
## import ***.py to run  the code, and delete file :os.remove('testScript.py')
#FAT memory cards will have a mount point of /sd, while SPIFFS cards will appear as /flash.
#28nM  400MHz dual core 64bit  Neural Network Processor(KPU) / 0.8Tops
#32K*2+32K*2 I,D cache
#8M SRAM
#onboard 16M flash
#网络模型： TinyYOLOv2 (after pruned)
#深度学习框架： TensorFlow/Keras/Darknet
# help('modules')   import Image_auto_save.py
# MicroPython v0.5.0-42-g458ed4e1a-dirty on 2020-04-23; M5 StickV / UnitV with kendryte-k210
import sensor, image, time
import gc
import sys
import machine
#import cpufreq  #bug!!
import os
import lcd
import pmu
#import uos
#import neopixel
from modules import ws2812 #v0.4.x firmware
#new FW 0.5.0: cannot import name ws2812
from fpioa_manager import *
from Maix import GPIO

#PMU
filler = "          "
axp = pmu.axp192()
axp.enableADCs(True)
axp.enablePMICSleepMode(True)

def Foto_save(EN):
    global save_cnt
    Zeit_anfang = time.ticks_ms()
    if EN:
            print("---------------------------")
            print("#:raw image "+str(save_cnt)+" saved to SD card!")

            gc.collect()

            img.save("/sd/DCIM_StickV/" +"OV7740_"+str(save_cnt)+ "_"+str(Zeit_anfang/1000) + ".bmp")
            #time.sleep(0.2)
            save_cnt = save_cnt+1

    Zeit_passed = time.ticks_ms()-Zeit_anfang
    print("#:Save time cost: "+str(Zeit_passed)+"ms!")
    print("---------------------------")
    return


def show_akku_status():
    #lcd.draw_string(0, 0, "Battery Info Develop", lcd.WHITE, lcd.BLACK)
    #lcd.draw_string(230, 0, "*", lcd.BLUE, lcd.BLACK)
    val = axp.getVbatVoltage()
    #lcd.draw_string(0, 75, "Battery Voltage:" + str(val) + filler, lcd.RED, lcd.BLACK)
    lcd.draw_string(0, 75, "Battery Voltage:" + str(val/1000)+ "v", lcd.WHITE)
    print("Battery Voltage:" + str(val/1000) + "v")

    val = axp.getBatteryDischargeCurrent()
    print("Battery DischargeCurrent:" + str(val) + "mA")
    lcd.draw_string(0, 90, "DischargeCurrent:" + str(val) +  "mA", lcd.GREEN, lcd.BLACK)
    val = axp.getUSBInputCurrent()
    lcd.draw_string(0, 105, "USB InputCurrent:" + str(val)+  "mA", lcd.ORANGE, lcd.BLACK)
    val = axp.getTemperature()
    lcd.draw_string(0, 119, "Temperature:" + str(val) + "C", lcd.YELLOW, lcd.BLACK)
    print("PMU Temperature:" + str(val) + "C")

    val = axp.getUSBVoltage()
    #lcd.draw_string(0, 30, "USB Voltage:" + str(val) + filler, lcd.WHITE, lcd.BLACK)
    print("USB Voltage:" + str(val/1000) + "v")
    val = axp.getUSBInputCurrent()
    print("USB InputCurrent:" + str(val) + "mA")
    #lcd.draw_string(0, 45, "USB InputCurrent:" + str(val) + filler, lcd.RED, lcd.BLACK)
    return

def sys_info_display():
    show_akku_status()
    print("#->Image FPS: "+str(clock.fps()) )              # Note: MaixPy's Cam runs about half as fast when connected
    print("GC free mem: "+ str(gc.mem_free()/1000)+"KB")
    gc.collect()
    print("------------- " )
    lcd.draw_string(10,10, "Image FPS: "+str(clock.fps()), lcd.BLACK,lcd.WHITE)
    lcd.draw_string(10,30, "Image size: "+str(img_w)+" x "+str(img_h), lcd.BLUE,lcd.WHITE)
    #time.sleep(0.5)
    return




fm.register(board_info.BUTTON_A, fm.fpioa.GPIO1)
but_a=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

#if but_a.value() == 0: #If dont want to run the demo
    #sys.exit()

fm.register(board_info.BUTTON_B, fm.fpioa.GPIO2)
but_b = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

fm.register(board_info.LED_W, fm.fpioa.GPIO3)
led_w = GPIO(GPIO.GPIO3, GPIO.OUT)
led_w.value(1) #RGBW LEDs are Active Low

fm.register(board_info.LED_R, fm.fpioa.GPIO4)
led_r = GPIO(GPIO.GPIO4, GPIO.OUT)
led_r.value(1) #RGBW LEDs are Active Low

fm.register(board_info.LED_G, fm.fpioa.GPIO5)
led_g = GPIO(GPIO.GPIO5, GPIO.OUT)
led_g.value(1) #RGBW LEDs are Active Low

fm.register(board_info.LED_B, fm.fpioa.GPIO6)
led_b = GPIO(GPIO.GPIO6, GPIO.OUT)
led_b.value(1) #RGBW LEDs are Active Low








version_info = sys.version
auto_save = False
save_cnt = 0
debug_print_EN = False
QR_detect = False
TF_Card_OK = False
OV77XX_EN = True


print("python version:"+version_info)
print(os.uname())
print(os.listdir())
#f=open("boot.py", "r")
#f_contents = f.open()
#print(f_contents)
#print(boot.py)
#lcd.init(freq=15000000)

# chdir to "/sd" or "/flash"
devices = os.listdir("/")
if "sd" in devices:
    os.chdir("/sd")
    sys.path.append('/sd')
    print("------------------")
    print("Micro SD detected!")
    print("------------------")
    print(os.listdir())
    TF_Card_OK =True
else:
    os.chdir("/flash")
    print("chdir to /flash")
sys.path.append('/flash')






if OV77XX_EN:
    #sensor.reset(freq=20000000, set_regs=True, dual_buff=False) #OV7740  Loop Time :155ms, run fps:6.451613
    #sensor.reset(freq=20000000, set_regs=True, dual_buff=True) #OV7740  Loop Time :91ms, run fps:10.98901
    sensor.reset()
else:
    sensor.reset()                  # OV2640 Reset and initialize the sensor. It will
                                   # run automatically, call sensor.run(0) to stop
sensor_ID = sensor.get_id()
if (sensor_ID == 30530):
    sensor_ID_str = 'OV7740'
#sensor.shutdown(enable)
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.VGA)  #strerror(ENOMEM) = "Alloc memory fail"
#sensor.set_hmirror(1)#for unit V  #bugs??
#sensor.set_vflip(1)#for unit V
img_w = sensor.width()
img_h = sensor.height()

print("image sensor is "+str(sensor_ID_str)+", with size "+str(img_w)+" x "+str(img_h))
sensor.skip_frames(time = 1000)     # Wait for settings take effect.
#sensor.skip_frames(30)             #Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.
gc.collect()
Zeit_end = 600*1000
condition = True
Zeit_Anfang = time.ticks_ms()
run_cnt = 0
loop_time = 1
fps=0
Zeit_interval = 10*1000 #10s
Zeit_passed = Zeit_Anfang
autosave_cnt =0

btnA_status = 1
btnB_status = 1

lcd.init()
lcd.rotation(2) #Rotate the lcd 180deg
lcd.draw_string(5, 5, "K210 VGA image auto save v1.1" , lcd.GREEN, lcd.BLACK)
Zeit_base = time.ticks_ms()

try:
    while(condition):
        clock.tick()                    # Update the FPS clock.
        #Zeit_jetzt=time.ticks_ms()      # get millisecond counter
        img = sensor.snapshot()         # Take a picture and return the image.
        lcd.display(img)



        if but_a.value() == 0 and btnA_status == 1:
            if led_w.value() == 1:
                led_w.value(0)
                auto_save = True
                lcd.draw_string(5, 5, "Auto save enabled!" , lcd.RED, lcd.BLACK)
            else:
                led_w.value(1)
                auto_save = False
                lcd.draw_string(5, 5, "Auto save disabled!" , lcd.ORANGE, lcd.BLACK)


            btnA_status = 0
        if but_a.value() == 1 and btnA_status == 0:
            btnA_status = 1



        if but_b.value() == 0 and btnB_status == 1:
            led_r.value(0)
            #led_w.value(1)
            #save image
            Foto_save(TF_Card_OK)
            lcd.draw_string(80,40,"Foto Saved :)",lcd.RED,lcd.WHITE)
            time.sleep(0.1)
            btnB_status = 0
        if but_b.value() == 1 and btnB_status == 0:
            btnB_status = 1
            led_r.value(1)






        if auto_save:
           Zeit_tmp = time.ticks_ms()- Zeit_passed
           if (Zeit_tmp > Zeit_interval):
             if TF_Card_OK:
                print("---------------------------")
                print("#:auto write raw: Image "+str(autosave_cnt)+" saved to SD card!")
                print("---------------------------")
                gc.collect()
                led_b.value(0)
                img.save("/sd/rawautosave/" +str(autosave_cnt)+ str(Zeit_jetzt/1000) + ".bmp")
                #time.sleep(0.2)
                autosave_cnt = autosave_cnt+1

                Zeit_passed = time.ticks_ms()
                led_b.value(1)

        Zeit_jetzt=time.ticks_ms()-Zeit_base
        if(Zeit_jetzt > Zeit_interval):
           print("Image FPS: "+str(clock.fps()) )              # Note: MaixPy's Cam runs about half as fast when connected
           sys_info_display()
           time.sleep(0.5)
           Zeit_base = time.ticks_ms()

        #fps =clock.fps()


except KeyboardInterrupt:
    print("#->:KeyboardInterrupt!")
    sys.exit()
