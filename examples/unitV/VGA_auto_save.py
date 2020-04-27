# image sensor auto save VGA Example for unit V
# V2.3 imagewriter bugs!
# V2.2 added MJPEG mode, need test--> MJPEG gif module not exist!
# V2.1 added bmp mode, test ok
# v1.0 2020.2.12  zell
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
#8M SRAM (2+6)
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
import uos
#import neopixel
from modules import ws2812 #v0.4.x firmware
#new FW 0.5.0: cannot import name ws2812
from fpioa_manager import *
from Maix import GPIO


version_info = sys.version
auto_save = True
debug_print_EN = False
QR_detect = False
TF_Card_OK = False
OV77XX_EN = True



print("GROVE port GND, VCC_5V, G35,G34 function: N.A.")
#UnitV btns
#Btn A, capature image
fm.register(18, fm.fpioa.GPIO1)
Btn_A=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!
#Btn B
fm.register(19, fm.fpioa.GPIO2)
Btn_B = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!


isButtonPressedA = 0
isButtonPressedB = 0


# UNIT V RGB Pixel LED
"""
PIXEL_LED_pin = 8
PIXEL_LED_num = 1
PIXEL_LED = class_ws2812 = ws2812(PIXEL_LED_pin,PIXEL_LED_num)
Color_Green = 0xE0
Color_Green_half =0x80
Color_Green_min =0x40
Color_Red_max = 0xFF
Color_Red = 0xE0
Color_Red_half = 0xA0
Color_Blue = 0xE0







def RGB_LED_RED():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED = class_ws2812.set_led(0,(Color_Red,0,0))
    PIXEL_LED = class_ws2812.display()
    time.sleep(0.01)
    PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
    PIXEL_LED = class_ws2812.display()

"""




#kpu_freqlist = cpufreq.set_frequency(cpu = cpu_freq, kpu = kpu_freq)
#cpu_freq: cpu frequency to be set, range [26,400]

#cpu_freq,kpu_freq = cpufreq.get_current_frequencies()
#print("CPU:"+str(cpu_freq)+"MHz, KPU:"+str(kpu_freq))

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

def findMaxBMP_IDinDir(dirname):
    larNum = -1
    try:
        dirList = uos.listdir(dirname)
        for fileName in dirList:
            currNum = int(fileName.split(".bmp")[0])
            if currNum > larNum:
                larNum = currNum
        return larNum
    except:
        return -1


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





if "sd" not in os.listdir("/"):
    #lcd.draw_string(lcd.width()//2-96,lcd.height()//2-4, "Error: Cannot read SD Card", lcd.WHITE, lcd.RED)
    print("Error: Cannot read SD Card")
else:
    TF_Card_OK = True
    print("TF_Card_OK")
#creat folder
print("#creat /train and /valid folder")
try:
    os.mkdir("/sd/train")
except Exception as e:
    pass

try:
    os.mkdir("/sd/vaild")
    print("mkdir:/sd/vaild")
except Exception as e:
    pass

if auto_save:
    print("#creat /rawautosave folder")
    try:
        os.mkdir("/sd/rawautosave")
    except Exception as e:
        pass

autosave_cnt =0

currentDirectory = 1
#bugs!
if (TF_Card_OK == True):
    try:
         #currentImage = max(findMaxIDinDir("/sd/train/" + str(currentDirectory)), findMaxIDinDir("/sd/vaild/" + str(currentDirectory))) + 1
         currentImage = findMaxIDinDir("/sd/train/")+1
         print("------------------")
         print("##: Current image file index: "+str(currentImage))
         print("------------------")
         time.sleep(0.5)

    except:
         currentImage = 0
         print("Get current image file index failed")
         pass
    try:
              #currentImage = max(findMaxIDinDir("/sd/train/" + str(currentDirectory)), findMaxIDinDir("/sd/vaild/" + str(currentDirectory))) + 1
              autosave_cnt = findMaxBMP_IDinDir("/sd/rawautosave")+1
              print("------------------")
              print("##: Current raw auto save file index: "+str(autosave_cnt))
              print("------------------")
              time.sleep(0.5)

    except:
              currentImage = 0
              print("Get current raw auto save file index failed")
              time.sleep(0.2)
              pass



print("##: press Btn_A to save img as MJPEG movie")
time.sleep(0.5)
if Btn_A.value() == 0 and isButtonPressedA == 0:
        print("##: MJPG_mode set!")
        MJPG_mode = True
        try:
            os.mkdir("/sd/rawmoviestream")
            print("mkdir: /sd/rawmoviestream")
        except Exception as e:
            pass
else:
        MJPG_mode = False
        print("##: BMP mode !")





if OV77XX_EN:
    #sensor.reset(freq=20000000, set_regs=True, dual_buff=False) #OV7740  Loop Time :155ms, run fps:6.451613
    #sensor.reset(freq=20000000, set_regs=True, dual_buff=True) #OV7740  Loop Time :91ms, run fps:10.98901
    #sensor.reset(freq=20000000)
    OV_err_counter = 0
    while 1:
        try:
                 sensor.reset(freq=20000000, set_regs=True, dual_buff=True) #OV7740  Loop Time :91ms, run fps:10.98901
                 break
        except:
                 OV_err_counter = OV_err_counter + 1
                 print("image sensor reset failed:")
                 if(OV_err_counter == 10):
                    print("image sensor reset failed:"+str(err_counter))
                    time.sleep(5)
                    machine.reset()
                 time.sleep(0.1)
                 continue

else:
    sensor.reset()                  # OV2640 Reset and initialize the sensor. It will
                                   # run automatically, call sensor.run(0) to stop
sensor_ID = sensor.get_id()
if (sensor_ID == 30530):
    sensor_ID_str = 'OV7740'
#sensor.shutdown(enable)
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.VGA)
sensor.set_hmirror(1)#for unit V  #bugs??
sensor.set_vflip(1)#for unit V
img_w = sensor.width()
img_h = sensor.height()

print("image sensor is "+str(sensor_ID_str)+", with size "+str(img_w)+" x "+str(img_h))
sensor.skip_frames(time = 1000)     # Wait for settings take effect.
#sensor.skip_frames(30)             #Wait for settings take effect.

if(MJPG_mode):
    #import gif
    movie = image.ImageWriter("/sd/rawmoviestream/movie_stream.bin")
    #movie = mjpeg.Mjpeg("/sd/rawautosave/example_movie.mjpeg")


clock = time.clock()                # Create a clock object to track the FPS.
gc.collect()
Zeit_end = 600*1000
condition = True
Zeit_Anfang = time.ticks_ms()
run_cnt = 0
loop_time = 1
fps=0
Zeit_interval = 10 #10s
Zeit_passed = Zeit_Anfang
#autosave_cnt =0
print("#->: VGA sensor auto save code starts now! Press Btn_B to exit auto save mode")
time.sleep(0.5)
while(condition):
    clock.tick()                    # Update the FPS clock.
    Zeit_jetzt=time.ticks_ms()      # get millisecond counter
    img = sensor.snapshot()         # Take a picture and return the image.

    if auto_save:
       Zeit_tmp = time.ticks_ms()- Zeit_passed
       if (Zeit_tmp > Zeit_interval*1000):
         if TF_Card_OK:
            print("---------------------------")
            print("#:auto write raw: Image "+str(autosave_cnt)+" saved to SD card!")
            print("---------------------------")
            gc.collect()

            if(MJPG_mode):
                movie.add_frame(img)
            else:
                img.save("/sd/rawautosave/OV7740_" +str(Zeit_jetzt//1000)+"_"+  str(autosave_cnt)+ ".bmp")
            time.sleep(0.2)
            autosave_cnt = autosave_cnt+1

            Zeit_passed = time.ticks_ms()
    #Btn_B to exit auto save mode
    if Btn_B.value() == 0 and isButtonPressedB == 0:
        print("---Btn_B pressed, auto save ends now!------------------------")
        condition = not condition
    if Btn_B.value() == 1:
        isButtonPressedB = 0

    #fps =clock.fps()
    print("Img FPS: "+str(clock.fps()) )              # Note: MaixPy's Cam runs about half as fast when connected

#----ends---------------
if(MJPG_mode):
    movie.close()
    #movie.close(clock.fps())
print("#:Total auto saved images: "+str(autosave_cnt))
print("#:Total running time: "+str((time.ticks_ms()-Zeit_Anfang)/1000   )+" s")
