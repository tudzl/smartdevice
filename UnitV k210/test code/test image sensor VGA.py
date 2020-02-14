# image sensor Example for unit V
# v1.0 2020.2.12  zell
# test sensor for Zeit_end = 60*1000, 60 seconds
# Welcome to the MaixPy IDE!
# 1. Conenct board to computer
# 2. Select board at the top of MaixPy IDE: `tools->Select Board`
# 3. Click the connect buttion below to connect board
# 4. Click on the green run arrow button below to run the script!

import sensor, image, time, lcd
import gc
from modules import ws2812



# UNIT V RGB Pixel LED
PIXEL_LED_pin = 8
PIXEL_LED_num = 1
PIXEL_LED = class_ws2812 = ws2812(PIXEL_LED_pin,PIXEL_LED_num)
Color_Green = 0xF0
Color_Green_half =0x80
Color_Green_min =0x40
Color_Red_max = 0xFF
Color_Red = 0xF0
Color_Red_half = 0xA0
Color_Blue = 0xF0

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
    time.sleep(0.05)
    PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
    PIXEL_LED = class_ws2812.display()
def RGB_LED_BLUE():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
        PIXEL_LED = class_ws2812.set_led(0,(0,0,Color_Blue))
        PIXEL_LED = class_ws2812.display()
        time.sleep(0.05)
        PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
        PIXEL_LED = class_ws2812.display()












#lcd.init(freq=15000000)
sensor.reset()                      # Reset and initialize the sensor. It will
                                    # run automatically, call sensor.run(0) to stop
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
#sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_hmirror(1)#for unit V
sensor.set_vflip(1)#for unit V
sensor.set_framesize(sensor.VGA)
sensor.skip_frames(time = 1000)     # Wait for settings take effect.
#sensor.skip_frames(30)             #Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.
Zeit_end = 180*1000
condition = True
Zeit_Anfang = time.ticks_ms()
run_cnt = 0 ;
while(condition):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    fps =clock.fps()
    img.draw_string(2,2, ("%2.2f FPS" %(fps)), color=(192,192,192), scale=2)#draw fps to real time image,scale is the text size
    #lcd.display(img)               # Display on LCD
    print("Image FPS: "+clock.fps())              # Note: MaixPy's Cam runs about half as fast when connected
    run_cnt = run_cnt+1                               # to the IDE. The FPS should increase once disconnected.
    Zeit_jetzt=time.ticks_ms()      # get millisecond counter
    print("GC free mem: "+ str(gc.mem_free()/1000)+"KB")
    print("Time passed:"+str(Zeit_jetzt/1000)+"s, run cnt:"+str(run_cnt))


    # find QR-codes test
    res = img.find_qrcodes()
    if len(res) > 0:
            RGB_LED_GREEN()
            img.draw_string(2,20, res[0].payload(), color=(0,220,0), scale=2)
            print("Found QRcode :"+res[0].payload())
    if (Zeit_jetzt-Zeit_Anfang > Zeit_end):
       print("---------------------------")
       print("Image Sensor test ends now!")
       print("---------------------------")
       condition = False
    if (run_cnt % 30 ==0):
        RGB_LED_RED()
    if (run_cnt % 100 ==0):
        RGB_LED_BLUE()
