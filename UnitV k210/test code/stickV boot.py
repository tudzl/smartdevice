import audio
import gc
import image
import lcd
import sensor
import sys
import time
import uos
import os
import KPU as kpu
from fpioa_manager import *
from machine import I2C
from Maix import I2S, GPIO

#
# initialize
#
lcd.init()
lcd.rotation(2)

fm.register(board_info.SPK_SD, fm.fpioa.GPIO0)
spk_sd=GPIO(GPIO.GPIO0, GPIO.OUT)
spk_sd.value(1) #Enable the SPK output

fm.register(board_info.SPK_DIN,fm.fpioa.I2S0_OUT_D1)
fm.register(board_info.SPK_BCLK,fm.fpioa.I2S0_SCLK)
fm.register(board_info.SPK_LRCLK,fm.fpioa.I2S0_WS)

wav_dev = I2S(I2S.DEVICE_0)

fm.register(board_info.BUTTON_A, fm.fpioa.GPIO1)
but_a=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

fm.register(board_info.BUTTON_B, fm.fpioa.GPIO2)
but_b = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

def checkForM5StickV():
    i2c = I2C(I2C.I2C0, freq=100000, scl=28, sda=29)
    devices = i2c.scan()
    if devices == [52, 108]:
        return True
    else:
        return False

def findMaxIDinDir(dirname):
    larNum = -1
    try:
        dirList = uos.listdir(dirname)
        for fileName in dirList:
            currNum = int(fileName.split(".jpg")[0])
            if currNum > larNum:
                larNum = currNum
        return larNum
    except:
        return -1


def play_sound(filename):
    try:
        player = audio.Audio(path = filename)
        player.volume(20)
        wav_info = player.play_process(wav_dev)
        wav_dev.channel_config(wav_dev.CHANNEL_1, I2S.TRANSMITTER,resolution = I2S.RESOLUTION_16_BIT, align_mode = I2S.STANDARD_MODE)
        wav_dev.set_sample_rate(wav_info[1])
        spk_sd.value(1)
        while True:
            ret = player.play()
            if ret == None:
                break
            elif ret==0:
                break
        player.finish()
        spk_sd.value(0)
    except:
        pass

def initialize_camera():
    err_counter = 0
    while 1:
        try:
            sensor.reset() #Reset sensor may failed, let's try some times
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

if checkForM5StickV() == False:
    while 1:
        time.sleep(1)

initialize_camera()

currentDirectory = 1

if "sd" not in os.listdir("/"):
    lcd.draw_string(lcd.width()//2-96,lcd.height()//2-4, "Error: Cannot read SD Card", lcd.WHITE, lcd.RED)

try:
    os.mkdir("/sd/train")
except Exception as e:
    pass

try:
    os.mkdir("/sd/vaild")
except Exception as e:
    pass

try:
    currentImage = max(findMaxIDinDir("/sd/train/" + str(currentDirectory)), findMaxIDinDir("/sd/vaild/" + str(currentDirectory))) + 1
except:
    currentImage = 0
    pass

isButtonPressedA = 0
isButtonPressedB = 0

try:
    while(True):
        img = sensor.snapshot()
        disp_img=img.copy()
        disp_img.draw_string(40,55,"%04d"%(currentImage),color=(255,255,255),scale=6)
        img.save("/sd/train/" + str(currentImage) + ".jpg", quality=95)
        currentImage = currentImage + 1
        if but_a.value() == 0 and isButtonPressedA == 0:
            break

        if but_a.value() == 1:
            isButtonPressedA = 0
        time.sleep(0.2)
        lcd.display(disp_img)
        print(currentImage)
except KeyboardInterrupt:
    pass
