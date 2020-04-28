#UnitV boot.py  facedetect demo
#Version 1.2 improved code, 2020.4.26, 32FPS @ serial terminal mode!
#Version 1.1 by Zell  tudzl@hotmail.de
#Image FPS: 19.02748 @ IDE monitor mode with OV7740
#import lcd
import image
import time
import uos
import sys
import gc
from Maix import GPIO
from Maix import freq
from fpioa_manager import *
from modules import ws2812

#lcd.init()
#lcd.rotation(2) #Rotate the lcd 180deg
'''
try:
    img = image.Image("/flash/startup.jpg")
    lcd.display(img)
except:
    lcd.draw_string(lcd.width()//2-100,lcd.height()//2-4, "Error: Cannot find start.jpg", lcd.WHITE, lcd.RED)
'''

#from Maix import I2S, GPIO
#import audio

OV77XX_EN = True
Sensor_img_flip =True

RGB565_Green = 0x07E0
RGB565_Red = 0xF800
#single color for ws2812
Color_Green = 0xE0
Color_Green_half =0x80
Color_Green_min =0x40
Color_Red_max = 0xFF
Color_Red = 0xE0
Color_Red_half = 0xA0
Color_Blue = 0xE0
# UNIT V RGB Pixel LED
PIXEL_LED_pin = 8
PIXEL_LED_num = 1
#PIXEL_LED = class_ws2812 = ws2812(PIXEL_LED_pin,PIXEL_LED_num)
PIXEL_LED = ws2812(PIXEL_LED_pin,PIXEL_LED_num)



def RGB_LED_RED():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED.set_led(0,(Color_Red,0,0))
    PIXEL_LED.display()
    #PIXEL_LED.del()
    time.sleep(0.01)
    #PIXEL_LED.set_led(0,(0,0,0))
    #PIXEL_LED.display()
def RGB_LED_RED_DK():
    PIXEL_LED.set_led(0,(Color_Red>>4,0,0))
    PIXEL_LED.display()
    time.sleep(0.01)


def RGB_LED_GREEN():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED.set_led(0,(0,Color_Green,0))
    PIXEL_LED.display()
    #time.sleep(0.03)
    #PIXEL_LED.set_led(0,(0,0,0))
    #PIXEL_LED.display()
def RGB_LED_PURPLE():
        #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
        PIXEL_LED.set_led(0,(Color_Red,0,Color_Blue))
        PIXEL_LED.display()

def RGB_LED_OFF():
    PIXEL_LED.set_led(0,(0,0,0))
    PIXEL_LED.display()

'''
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
'''
RGB_LED_OFF()
version_info = sys.version
print("python version:"+version_info)
#freq.set(400,400)
print("CPU freq:"+str(freq.get_cpu()))
print("KPU freq:"+str(freq.get_kpu()))
print("UnitV facedetect demo v1.1 by Zell, 27.04.2020")
print("GROVE port GND, VCC_5V, G35,G34 function: NA")
print("UnitV Btn_A and Btn_B init.")
#UnitV btns
#Btn A, capature image
fm.register(18, fm.fpioa.GPIO1)
Btn_A=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!
#Btn B
fm.register(19, fm.fpioa.GPIO2)
Btn_B = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!


isButtonPressedA = 0
isButtonPressedB = 0

if Btn_B.value() == 0: #If dont want to run the demo
    sys.exit()




time.sleep(0.5) # Delay for few seconds to see the start-up screen :p

import sensor
import KPU as kpu

err_counter = 0
imgsave_cnt = 0
while 1:
    try:
        #sensor.reset() #Reset sensor may failed, let's try sometimes
        if OV77XX_EN:
            #sensor.reset(freq=20000000, set_regs=True, dual_buff=False) #OV7740
            sensor.reset(freq=20000000, set_regs=True, dual_buff=True) #OV7740
            RGB_LED_RED_DK()
            #time.sleep(0.1)
        else:
            sensor.reset()                  # OV2640 Reset and initialize the sensor. It will
        break
    except:
        err_counter = err_counter + 1
        if err_counter == 20:
            print("Error: Sensor Init Failed")
            #lcd.draw_string(lcd.width()//2-100,lcd.height()//2-4, "Error: Sensor Init Failed", lcd.WHITE, lcd.RED)
        time.sleep(0.1)
        continue

sensor.set_pixformat(sensor.RGB565)
#sensor.set_framesize(sensor.QVGA) #QVGA=320x240
sensor.set_framesize(sensor.QVGA)
if Sensor_img_flip:
   sensor.set_hmirror(1)#for unit V  #bugs??
   sensor.set_vflip(1)#for unit V
img_w = sensor.width()
img_h = sensor.height()
sensor_ID = sensor.get_id()
if (sensor_ID == 30530):
    sensor_ID_str = 'OV7740'
print("image sensor is "+str(sensor_ID_str)+", with size "+str(img_w)+" x "+str(img_h))
sensor.skip_frames(time = 600)
#sensor.run(1)
#https://maixpy.sipeed.com/zh/libs/Maix/kpu.html
task = kpu.load(0x300000) # Load Model File from Flash
#you need put model(face.kfpkg) in flash at address 0x300000
anchor = (1.889, 2.5245, 2.9465, 3.94056, 3.99987, 5.3658, 5.155437, 6.92275, 6.718375, 9.01025)
# Anchor data is for bbox, extracted from the training sets.
kpu.init_yolo2(task, 0.5, 0.3, 5, anchor)# kpu 网络对象,threshold,box_iou 门限,锚点数,锚点参数与模型参数一致
fps=0
but_stu = 1
autosave_cnt =0
face_duration_cnt = 0
clock = time.clock()
gc.collect()
RGB_LED_OFF()
try:
    while(True):
        clock.tick()
        img = sensor.snapshot() # Take an image from sensor
        bbox = kpu.run_yolo2(task, img) # Run the detection routine
        if bbox:
            RGB_LED_RED()
            for i in bbox:
                face_cnt = i.objnum()
                if face_cnt > 1:
                    RGB_LED_GREEN()
                face_value=i.value()
                face_size = i.w()*i.h()
                obj_x = i.x()
                obj_y = i.y()
                print("found face:"+str(face_cnt)+";  value:"+str(face_value)+";  size:"+str(face_size))
                #print(i)

                if Sensor_img_flip:
                     img.draw_rectangle(img_w-obj_x-i.w(),obj_y,i.w(),i.h(),RGB565_Green)
                else:
                     img.draw_rectangle(i.rect())
                #lcd.display(img)
                time.sleep(0.1)
                face_duration_cnt =face_duration_cnt +1
            RGB_LED_OFF()
            if  face_cnt > 2:
                RGB_LED_PURPLE()
                Zeit_jetzt= time.ticks_ms()/1000
                img.draw_string(2,2, ("%2.1f FPS" %(fps)), color=(192,192,192), scale=2)#draw fps to real time image,scale is the text size
                img.save("/sd/autosave/face_" +str(Zeit_jetzt//1000)+"_"+  str(autosave_cnt)+  ".bmp")
                print(">=3 face found, image saved to /sd/autosave/face" +str(autosave_cnt)+ str(Zeit_jetzt/1000) + ".bmp")
                time.sleep(0.2)
            if  face_duration_cnt > 10:
                    RGB_LED_PURPLE()
                    Zeit_jetzt= time.ticks_ms()/1000
                    img.draw_string(2,2, ("%2.1f FPS" %(fps)), color=(192,192,192), scale=2)#draw fps to real time image,scale is the text size
                    img.save("/sd/autosave/face" +str(autosave_cnt)+ str(Zeit_jetzt/1000) + ".bmp")
                    print("Face long time found, image saved to /sd/autosave/face" +str(autosave_cnt)+ str(Zeit_jetzt/1000) + ".bmp")
                    time.sleep(0.2)
                    RGB_LED_GREEN()


        else:
            face_duration_cnt = 0
        gc.collect()
        fps =clock.fps()
        print("Image FPS: "+str(fps) )

except KeyboardInterrupt:
    a = kpu.deinit(task)
    sys.exit()
#-----results--------------------------------
'''
{"x":90, "y":34, "w":82, "h":111, "value":0.663561, "classid":0, "index":0, "objnum":1}
{"w":320, "h":240, "type"="rgb565", "size":153600}
'''
#QVGA  150K
#VGA  600K
