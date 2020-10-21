# Untitled - By: ling - Tue Oct 20 2020
# factory default boot.py file modified by ZL

#version 1.1   2020.20.21
import lcd
import image
import time
import uos
import utime
import sys
import gc
import pmu
from Maix import GPIO
from fpioa_manager import *


#global vars
auto_save = False
save_cnt = 0
debug_print_EN = False
QR_detect = False
TF_Card_OK = False
Zeit_interval = 10*1000  # 10s interval for sys info display
#PMU vars
filler = "          "
axp = pmu.axp192()
axp.enableADCs(True)

version_info = sys.version
print("python version:"+version_info)
print(os.uname())
print(os.listdir())


def Foto_save(EN):
    global save_cnt
    Zeit_anfang = time.ticks_ms()
    if EN:
            print("---------------------------")
            print("#:raw image "+str(save_cnt)+" saved to SD card!")

            gc.collect()

            img.save("/sd/DCIM_StickV/" +str(save_cnt)+ str(Zeit_anfang/1000) + ".bmp")
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
    print("------------- "+str(clock.fps()) )
    #time.sleep(0.5)
    return


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

if "sd" not in os.listdir("/"):
    #lcd.draw_string(lcd.width()//2-96,lcd.height()//2-4, "Error: Cannot read SD Card", lcd.WHITE, lcd.RED)
    print("Error: Cannot read SD Card")
else:
    TF_Card_OK = True
    print("TF_Card_OK")
#creat folder
#print("#creat /train and /valid folder")
#try:
    #os.mkdir("/sd/train")
#except Exception as e:
    #pass

#try:
    #os.mkdir("/sd/vaild")
    #print("mkdir:/sd/vaild")
#except Exception as e:
    #pass

print("#creat /DCIM_StickV folder")
try:
    os.mkdir("/sd/DCIM_StickV")
except Exception as e:
    pass

if auto_save:
    print("#creat /rawautosave folder")
    try:
        os.mkdir("/sd/rawautosave")
    except Exception as e:
        pass

if (TF_Card_OK == True):
    try:
         #currentImage = max(findMaxIDinDir("/sd/train/" + str(currentDirectory)), findMaxIDinDir("/sd/vaild/" + str(currentDirectory))) + 1
         currentImage = findMaxIDinDir("/sd/rawautosave/")+1
         print("------------------")
         print("##: Current image file index: "+str(currentImage))
         print("------------------")
         time.sleep(0.5)

    except:
         currentImage = 0
         print("Get current image file index failed")
         pass







lcd.init()
lcd.rotation(2) #Rotate the lcd 180deg

try:
    img = image.Image("/flash/startup.jpg")
    #img2 = image.Image("/flash/logo.jpg")
    lcd.display(img)
except:
    lcd.draw_string(lcd.width()//2-100,lcd.height()//2-4, "Error: Cannot find start.jpg", lcd.WHITE, lcd.RED)

from Maix import I2S, GPIO
import audio

fm.register(board_info.SPK_SD, fm.fpioa.GPIO0)
spk_sd=GPIO(GPIO.GPIO0, GPIO.OUT)
spk_sd.value(1) #Enable the SPK output

fm.register(board_info.SPK_DIN,fm.fpioa.I2S0_OUT_D1)
fm.register(board_info.SPK_BCLK,fm.fpioa.I2S0_SCLK)
fm.register(board_info.SPK_LRCLK,fm.fpioa.I2S0_WS)

wav_dev = I2S(I2S.DEVICE_0)

try:
    player = audio.Audio(path = "/flash/ding.wav")
    player.volume(100)
    wav_info = player.play_process(wav_dev)
    wav_dev.channel_config(wav_dev.CHANNEL_1, I2S.TRANSMITTER,resolution = I2S.RESOLUTION_16_BIT, align_mode = I2S.STANDARD_MODE)
    wav_dev.set_sample_rate(wav_info[1])
    while True:
        ret = player.play()
        if ret == None:
            break
        elif ret==0:
            break
    player.finish()
except:
    pass

fm.register(board_info.BUTTON_A, fm.fpioa.GPIO1)
but_a=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

if but_a.value() == 0: #If dont want to run the demo
    sys.exit()

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


time.sleep(0.5) # Delay for few seconds to see the start-up screen :p

try:
    #img = image.Image("/flash/startup.jpg")
    img = image.Image("/flash/logo.jpg")
    lcd.display(img)
except:
    lcd.draw_string(lcd.width()//2-100,lcd.height()//2-4, "Error: Cannot find logo.jpg", lcd.WHITE, lcd.RED)

#time.sleep(0.4)



time.sleep(0.6)


import sensor
import KPU as kpu

err_counter = 0

while 1:
    try:
        sensor.reset() #Reset sensor may failed, let's try sometimes
        break
    except:
        err_counter = err_counter + 1
        if err_counter == 20:
            lcd.draw_string(lcd.width()//2-100,lcd.height()//2-4, "Error: Sensor Init Failed", lcd.WHITE, lcd.RED)
        time.sleep(0.1)
        continue

sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA) #QVGA=320x240
sensor.run(1)

task = kpu.load(0x300000) # Load Model File from Flash
anchor = (1.889, 2.5245, 2.9465, 3.94056, 3.99987, 5.3658, 5.155437, 6.92275, 6.718375, 9.01025)
# Anchor data is for bbox, extracted from the training sets.
kpu.init_yolo2(task, 0.5, 0.3, 5, anchor)

btnA_status = 1
btnB_status = 1
clock = time.clock()
gc.collect()
Zeit_base = time.ticks_ms()
try:
    while(True):
        clock.tick()
        img = sensor.snapshot() # Take an image from sensor
        bbox = kpu.run_yolo2(task, img) # Run the detection routine
        if bbox:
            for i in bbox:
                print(i)
                img.draw_rectangle(i.rect())
            led_g.value(0)
        else:
            led_g.value(1)

        lcd.display(img)

        if but_a.value() == 0 and btnA_status == 1:
            if led_w.value() == 1:
                led_w.value(0)
            else:
                led_w.value(1)
            btnA_status = 0
        if but_a.value() == 1 and btnA_status == 0:
            btnA_status = 1


        if but_b.value() == 0 and btnB_status == 1:
                led_r.value(0)
                #led_w.value(1)
                #save image
                Foto_save(TF_Card_OK)
                btnB_status = 0
        if but_b.value() == 1 and btnB_status == 0:
            btnB_status = 1
            led_r.value(1)

        Zeit_jetzt=time.ticks_ms()-Zeit_base
        if(Zeit_jetzt > Zeit_interval):
           sys_info_display()
           time.sleep(0.5)
           Zeit_base = time.ticks_ms()







except KeyboardInterrupt:
    a = kpu.deinit(task)
    sys.exit()


