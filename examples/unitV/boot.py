# image sensor Example for unit V
# v2.0  press BtnA to enter default QR-Scan, otherwise face detect  2020.4.26
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
#8M SRAM
#onboard 16M flash
#网络模型： TinyYOLOv2 (after pruned)
#深度学习框架： TensorFlow/Keras/Darknet
import sensor, image, time, lcd
import gc
import sys
import machine
#import cpufreq  #bug!!
import os
import uos
from modules import ws2812
from fpioa_manager import *
from Maix import GPIO
import utime


print("UnitV boot.py starts")
utime.localtime()
print("GROVE port GND, VCC_5V, G35,G34 function: UART1")
#UnitV btns
#Btn A, capature image
fm.register(18, fm.fpioa.GPIO1)
Btn_A=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!
#Btn B
fm.register(19, fm.fpioa.GPIO2)
Btn_B = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!


isButtonPressedA = 0
isButtonPressedB = 0

#
GROVE_pin3 = 35 #chip physical port pin number
GROVE_pin4 = 34
fm.unregister(GROVE_pin3, fm.fpioa.GPIOHS13)
fm.unregister(GROVE_pin4, fm.fpioa.GPIOHS14)


# fit to m5stack fire grove C: pin 3 TX, pin4 RX
fm.register(GROVE_pin4,fm.fpioa.UART1_TX)
fm.register(GROVE_pin3,fm.fpioa.UART1_RX)
GROVE_UART = machine.UART(machine.UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)
#GROVE_UART = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)

# fit to m5stack fire grove C: pin 3 TX, pin4 RX
#fm.register(GROVE_pin3, fm.fpioa.UART1_RX ,  GPIO.PULL_UP )
#fm.register(GROVE_pin4, fm.fpioa.UART1_TX ,  GPIO.PULL_UP )
#uart_out = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)

print("##:wait for BtnA pressing for face detect...")
time.sleep(0.5)


if ((Btn_A.value() == 0 )and(Btn_B.value() == 0 )):
   print("-->Both A and B are pressed!")
   print("-->System reset now!")
   time.sleep(0.2)
   machine.reset()



if Btn_A.value() == 0 :
   #import face
   print("-->BtnA is pressed!") # enter boot.py code if BtnB not pressed
   time.sleep(0.2)
elif Btn_B.value() == 0 :
      #import face
      print("-->BtnB is pressed!")
      print("VGA image auto save demo now!")
      time.sleep(0.2)
      import VGA_auto_save
else:
   import face

isButtonPressedA = 0
isButtonPressedB = 0


time.sleep(0.5)
print("VGA sensor QR-image demo starts now(boot.py)!")

version_info = sys.version
QR_detect = False
TF_Card_OK = False
OV77XX_EN = True
# UNIT V RGB Pixel LED
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
def RGB_LED_GREEN():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED = class_ws2812.set_led(0,(0,Color_Green,0))
    PIXEL_LED = class_ws2812.display()
    time.sleep(0.03)
    PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
    PIXEL_LED = class_ws2812.display()
def RGB_LED_BLUE():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
        PIXEL_LED = class_ws2812.set_led(0,(0,0,Color_Blue))
        PIXEL_LED = class_ws2812.display()
        time.sleep(0.05)
        PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
        PIXEL_LED = class_ws2812.display()
def RGB_LED_ORANGE(): # run indication
        #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
        PIXEL_LED = class_ws2812.set_led(0,(Color_Red_max,Color_Green_min,0))
        PIXEL_LED = class_ws2812.display()
        time.sleep(0.1)
        PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
        PIXEL_LED = class_ws2812.display()
def RGB_LED_LightGreen(): # run indication
                #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
                PIXEL_LED = class_ws2812.set_led(0,(Color_Red_half,Color_GREEN,0))
                PIXEL_LED = class_ws2812.display()
                time.sleep(0.2)
                PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
                PIXEL_LED = class_ws2812.display()

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



#kpu_freqlist = cpufreq.set_frequency(cpu = cpu_freq, kpu = kpu_freq)
#cpu_freq: cpu frequency to be set, range [26,400]

#cpu_freq,kpu_freq = cpufreq.get_current_frequencies()
#print("CPU:"+str(cpu_freq)+"MHz, KPU:"+str(kpu_freq))

print("python version:"+version_info)
print(uos.uname())
print(uos.listdir())
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
print("#creat train and valid folder")
try:
    os.mkdir("/sd/train")
except Exception as e:
    pass

try:
    os.mkdir("/sd/vaild")
    print("mkdir:/sd/vaild")
except Exception as e:
    pass

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









if OV77XX_EN:
    #sensor.reset(freq=20000000, set_regs=True, dual_buff=False) #OV7740  Loop Time :155ms, run fps:6.451613
    sensor.reset(freq=20000000, dual_buff=True) #OV7740  Loop Time :91ms, run fps:10.98901
    #sensor.reset(freq=20000000)
else:
    sensor.reset()                  # OV2640 Reset and initialize the sensor. It will
                                    # run automatically, call sensor.run(0) to stop
#sensor.shutdown(enable)

sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.VGA)
#sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
#sensor.set_auto_whitebal(True)  #OV2640
sensor.set_hmirror(1)#for unit V
sensor.set_vflip(1)#for unit V

img_w = sensor.width()
img_h = sensor.height()
sensor_ID = sensor.get_id()
print("image sensor is "+str(sensor_ID)+", with size "+str(img_w)+" x "+str(img_h))
sensor.run(1)
sensor.skip_frames(time = 1000)     # Wait for settings take effect.
#sensor.skip_frames(30)             #Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.
Zeit_end = 600*1000
condition = True
Zeit_Anfang = time.ticks_ms()
run_cnt = 0
loop_time = 1
fps=0
while(condition):
    clock.tick()                    # Update the FPS clock.
    Zeit_jetzt=time.ticks_ms()      # get millisecond counter
    #sensor.alloc_extra_fb()
    img = sensor.snapshot()         # Take a picture and return the image. store in mpy heap limit to 100kb
    #img_hst =img.get_histogram()
    #img = sensor.snapshot(copy_to_fb=True)
    img.draw_string(2,2, ("%2.1f FPS" %(fps)), color=(192,192,192), scale=2)#draw fps to real time image,scale is the text size
    #lcd.display(img)               # Display on LCD

    # find QR-codes test
    if QR_detect:
        res = img.find_qrcodes()
        if len(res) > 0:
            RGB_LED_GREEN()
            img.draw_string(2,20, res[0].payload(), color=(0,220,0), scale=2)
            print("Found QRcode :"+res[0].payload())
    # find QR-codes test end

    #if (run_cnt % 30 ==0):
        #RGB_LED_RED()
    if (run_cnt % 100 ==0):
        RGB_LED_BLUE()
        gc.collect()
        # -------------time out end
        print("Time passed:"+str(Zeit_jetzt/1000)+"s, run cnt:"+str(run_cnt))
        if (Zeit_jetzt-Zeit_Anfang > Zeit_end):
                   print("---------------------------")
                   print("Image Sensor test ends now!")
                   print("---------------------------")
                   condition = False


    if Btn_A.value() == 0 and isButtonPressedA == 0:
                RGB_LED_ORANGE()
                if TF_Card_OK:
                    img.save("/sd/train/" + str(sensor_ID)+ str(currentImage)+"_s"+str(Zeit_jetzt/1000) + ".bmp", )
                    #img_roi = (80,80,480,320)
                    #img.save("/sd/train/" + str(sensor_ID)+ str(currentImage)+"_s"+str(Zeit_jetzt/1000) + ".bmp")
                    #img.save("/sd/train/" + str(sensor_ID)+ str(currentImage)+"_s"+str(Zeit_jetzt/1000) + ".jpg", roi=img_roi, quality=95)
                    print("---------------------------")
                    print("Image "+str(currentImage)+" saving to SD card now...")
                    print("---------------------------")
                    currentImage = currentImage + 1
                    time.sleep(0.2)
                #QR_detect = True
    if Btn_A.value() == 1:
                isButtonPressedA = 0
    if Btn_B.value() == 0 and isButtonPressedB == 0:
                RGB_LED_RED()
                QR_detect = not QR_detect
                time.sleep(0.5)
                RGB_LED_RED()
    if Btn_B.value() == 1:
                isButtonPressedB = 0

    loop_time = time.ticks_ms() -Zeit_jetzt     # get loop_time
    print("GC free mem: "+ str(gc.mem_free()/1000)+"KB")
    print("Loop Time :"+str(loop_time)+"ms, run fps:"+str(1000/loop_time))#loop_time

    run_cnt = run_cnt+1      # to the IDE. The FPS should increase once disconnected.


    #time.sleep(0.1)
    fps =clock.fps()
    print("Image FPS: "+str(clock.fps()) )              # Note: MaixPy's Cam runs about half as fast when connected
