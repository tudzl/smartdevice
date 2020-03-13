# image sensor Example for unit V
# v1.0 2020.2.12  zell
# test sensor for Zeit_end = 60*1000, 60 seconds
# Welcome to the MaixPy IDE!
# 1. Conenct board to computer
# 2. Select board at the top of MaixPy IDE: `tools->Select Board`
# 3. Click the connect buttion below to connect board
# 4. Click on the green run arrow button below to run the script!
## import ***.py to run  the code, and delete file :os.remove('testScript.py')
#FAT memory cards will have a mount point of /sd, while SPIFFS cards will appear as /flash.
import sensor, image, time, lcd
import gc
import sys
#import cpufreq  #bug!!
import os
import uos
from modules import ws2812
from fpioa_manager import *
from Maix import GPIO


version_info = sys.version
QR_detect = True
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
GROVE_UART = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)

# fit to m5stack fire grove C: pin 3 TX, pin4 RX
#fm.register(GROVE_pin3, fm.fpioa.UART1_RX ,  GPIO.PULL_UP )
#fm.register(GROVE_pin4, fm.fpioa.UART1_TX ,  GPIO.PULL_UP )
#uart_out = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)





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
else:
    os.chdir("/flash")
    print("chdir to /flash")
sys.path.append('/flash')



sensor.reset()                      # Reset and initialize the sensor. It will
                                    # run automatically, call sensor.run(0) to stop
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
#sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_hmirror(1)#for unit V
sensor.set_vflip(1)#for unit V
sensor.set_framesize(sensor.VGA)
img_w = sensor.width()
img_h = sensor.height()
print("image sensor is "+str(img_w)+" x "+str(img_h))
sensor.skip_frames(time = 1000)     # Wait for settings take effect.
#sensor.skip_frames(30)             #Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.
Zeit_end = 180*1000
condition = True
Zeit_Anfang = time.ticks_ms()
run_cnt = 0
loop_time = 1
fps=0
while(condition):
    clock.tick()                    # Update the FPS clock.
    Zeit_jetzt=time.ticks_ms()      # get millisecond counter
    img = sensor.snapshot()         # Take a picture and return the image.

    img.draw_string(2,2, ("%2.2f FPS" %(fps)), color=(192,192,192), scale=2)#draw fps to real time image,scale is the text size
    #lcd.display(img)               # Display on LCD

    run_cnt = run_cnt+1                               # to the IDE. The FPS should increase once disconnected.

    print("GC free mem: "+ str(gc.mem_free()/1000)+"KB")
    print("Time passed:"+str(Zeit_jetzt/1000)+"s, run cnt:"+str(run_cnt))
    print("Loop Time :"+str(loop_time)+"ms, run fps:"+str(1000/loop_time))#loop_time


    # find QR-codes test
    if QR_detect:
        res = img.find_qrcodes()
        if len(res) > 0:
            RGB_LED_GREEN()
            img.draw_string(2,20, res[0].payload(), color=(0,220,0), scale=2)
            print("Found QRcode :"+res[0].payload())
    # find QR-codes test end

    if (run_cnt % 30 ==0):
        RGB_LED_RED()
    if (run_cnt % 100 ==0):
        RGB_LED_BLUE()
    if Btn_A.value() == 0 and isButtonPressedA == 0:
            RGB_LED_ORANGE()
            QR_detect = True
    if Btn_A.value() == 1:
            isButtonPressedA = 0
    if Btn_B.value() == 0 and isButtonPressedB == 0:
            RGB_LED_BLUE()
            QR_detect = False
    if Btn_B.value() == 1:
            isButtonPressedB = 0
    # -------------time out end
    if (Zeit_jetzt-Zeit_Anfang > Zeit_end):
               print("---------------------------")
               print("Image Sensor test ends now!")
               print("---------------------------")
               condition = False

    loop_time = time.ticks_ms() -Zeit_jetzt     # get loop_time
    fps =clock.fps()
    print("Image FPS: "+str(clock.fps()) )              # Note: MaixPy's Cam runs about half as fast when connected
