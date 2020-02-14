#V-traning 用的那个拍摄照片程序
#Button A trigger image capature and write to SD-card
# modified by zell, 2020.1.7
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
from modules import ws2812

#class ws2812(led_pin=-1,led_num=-1,i2s_num=I2S_DEVICE_2,i2s_chn=I2S_CHANNEL_3,i2s_dma_chn=DMAC_CHANNEL1)
#i2s_num`: 该对象使用哪个 `I2S` 设备进行驱动，默认为 `I2S_DEVICE_2`,取值范围为 `0-2`
#i2s_chn`: 该对象使用哪个 `I2S` 通道，默认为 `I2S_CHANNEL_3`，取值范围为 `0-3`
PIXEL_LED_pin = 8
PIXEL_LED_num = 1
PIXEL_LED = class_ws2812 = ws2812(PIXEL_LED_pin,PIXEL_LED_num)
Color_Green = 0xE0
BRIGHTNESS_G = 0x10
Color_Red = 0xD0
Color_Blue = 0xA0

#
# initialize
#
#lcd.init()

fm.register(18, fm.fpioa.GPIO1)
but_a=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

fm.register(19, fm.fpioa.GPIO2)
but_b = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

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
        return 0


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
    sensor.set_hmirror(1)
    sensor.set_vflip(1)
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA) #QVGA=320x240, VGA cause out of memory!!!
    sensor.run(1)


def RGB_LED():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED = class_ws2812.set_led(0,(0,BRIGHTNESS,0))
    PIXEL_LED = class_ws2812.display()
    time.sleep(0.5)
    PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
    PIXEL_LED = class_ws2812.display()

def RGB_LED_RED():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED = class_ws2812.set_led(0,(Color_Red,0,0))
    PIXEL_LED = class_ws2812.display()
    time.sleep(0.05)
    PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
    PIXEL_LED = class_ws2812.display()
def RGB_LED_GREEN():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED = class_ws2812.set_led(0,(0,Color_Green,0))
    PIXEL_LED = class_ws2812.display()
    time.sleep(0.005)
    PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
    PIXEL_LED = class_ws2812.display()

def RGB_LED_BLUE():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED = class_ws2812.set_led(0,(0,0,Color_Blue))
    PIXEL_LED = class_ws2812.display()
    time.sleep(0.005)
    PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
    PIXEL_LED = class_ws2812.display()
def RGB_LED_ORANGE(): # run indication
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED = class_ws2812.set_led(0,(Color_Green,Color_Red,0))
    PIXEL_LED = class_ws2812.display()
    time.sleep(0.005)
    PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
    PIXEL_LED = class_ws2812.display()




initialize_camera()

currentDirectory = 1

if "sd" not in os.listdir("/"):
    #lcd.draw_string(lcd.width()//2-96,lcd.height()//2-4, "Error: Cannot read SD Card", lcd.WHITE, lcd.RED)
    print("Error: Cannot init SD Card")
try:
    os.mkdir("/sd/train")
    print("os.mkdir: /sd/train")
except Exception as e:
    pass

try:
    os.mkdir("/sd/vaild")
    print("os.mkdir: /sd/vaild")
except Exception as e:
    pass

try:
    currentImage = max(findMaxIDinDir("/sd/train/" + str(currentDirectory)), findMaxIDinDir("/sd/vaild/" + str(currentDirectory))) + 1
except:
    currentImage = 0
    pass

isButtonPressedA = 0
isButtonPressedB = 0


run_cnt = 0
clock = time.clock()

try:
    while(True):
        clock.tick()  # Update the FPS clock.
        run_cnt = run_cnt+1
        img = sensor.snapshot()
        disp_img=img.copy()
        #temp = disp_img.draw_rectangle(0,60,320,1,color=(0,144,255),thickness=10)
        #temp = disp_img.draw_string(50,55,"Train:%03d/35   Class:%02d/10"%(currentImage,currentDirectory),color=(255,255,255),scale=1)
        #print("class%02d"%(currentDirectory))

        if but_a.value() == 0 and isButtonPressedA == 0:
            #BtnA is pressed
            RGB_LED_GREEN()
            if currentImage <= 30 or currentImage > 35:
                try:
                    if str(currentDirectory) not in os.listdir("/sd/train"):
                        try:
                            os.mkdir("/sd/train/" + str(currentDirectory))
                        except:
                            pass
                    photo = img.save("/sd/train/" + str(currentDirectory) + "/" + str(currentImage) + ".jpg", quality=95)
                    print("ok Class" + str(currentDirectory) + " -> " + str(currentImage) + "/35")
                    RGB_LED()
                except:
                    print("Error: Cannot Write to SD Card")
                    RGB_LED_RED()
                    time.sleep(0.5)
            else:
                try:
                    if str(currentDirectory) not in os.listdir("/sd/vaild"):
                        try:
                            os.mkdir("/sd/vaild/" + str(currentDirectory))
                        except:
                            pass
                    photo = img.save("/sd/vaild/" + str(currentDirectory) + "/" + str(currentImage) + ".jpg", quality=95)
                    print("ok Class" + str(currentDirectory) + " -> " + str(currentImage) + "/35")
                    RGB_LED()
                except:
                    print("Error: Cannot Write to SD Card")
                    RGB_LED_RED()
                    time.sleep(1)
            currentImage = currentImage + 1
            isButtonPressedA = 1

        if but_a.value() == 1:
            isButtonPressedA = 0

        if but_b.value() == 0 and isButtonPressedB == 0:
            #BtnB is pressed
            RGB_LED_BLUE()
            currentDirectory = currentDirectory + 1
            if currentDirectory == 11:
                currentDirectory = 1
            currentImage = max(findMaxIDinDir("/sd/train/" + str(currentDirectory)), findMaxIDinDir("/sd/vaild/" + str(currentDirectory))) + 1
            print("ok Class" + str(currentDirectory) + " -> " + str(currentImage) + "/35")
            isButtonPressedB = 1

        if but_b.value() == 1:
            #RGB_LED_BLUE()
            isButtonPressedB = 0
        RGB_LED_ORANGE()#run indication
        print("Run_cnt:"+str(run_cnt) )
        print(str(clock.fps())+"FPS")
except KeyboardInterrupt:
    pass
