UART# GPIO test for Unit V - By: ling - Mon Jan 7 2020
# version 1.1 test ok!
# set grove port to all output mode , set high, then turn GROVE_3 to input, and print out GROVE signals, show green LED,if input high/float@GROVE_3


from fpioa_manager import *
from Maix import GPIO
from machine import I2C
from modules import ws2812
import time

FPS_clock = time.clock()
run_cnt = 0
isButtonPressedA = 0
isButtonPressedB = 0

#fm.register(board_info.LED_R, fm.fpioa.GPIO0)

#UnitV btns
#Btn A, capature image
fm.register(18, fm.fpioa.GPIO1)
Btn_A=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!
#Btn B
fm.register(19, fm.fpioa.GPIO2)
Btn_B = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!



# GROVE pin init as GPIO_out
print("GROVE_pin3 and pin4 now set as output GPIOHS13,14" )
GROVE_pin3 = 35 #physical port pin number
GROVE_pin4 = 34
# unreg
fm.unregister(GROVE_pin3, fm.fpioa.GPIOHS13)
fm.unregister(GROVE_pin4, fm.fpioa.GPIOHS14)

fm.register(GROVE_pin3, fm.fpioa.GPIOHS13)
fm.register(GROVE_pin4, fm.fpioa.GPIOHS14)

GROVE_3=GPIO(GPIO.GPIOHS13,GPIO.OUT)

GROVE_4=GPIO(GPIO.GPIOHS14,GPIO.OUT)
#>>> GROVE_3
#Pin(18)
#GROVE_3.value(0)

GROVE_3.value(1)
GROVE_3.value()

GROVE_4.value(1)
GROVE_4.value() #get 0
print("GROVE_pin:"+ str(GROVE_3.value())+str(GROVE_4.value()) )

time.sleep(3)
# GROVE pin init as GPIO_INPUT
print("GROVE_pin3/G35 is now being set as input GPIO3,4" )
#GROVE_pin3 = 35 #physical port pin number
#GROVE_pin4 = 34
#unreg first
fm.unregister(GROVE_pin3, fm.fpioa.GPIOHS13)
#fm.unregister(GROVE_pin4, fm.fpioa.GPIOHS14)
#reg
fm.register(GROVE_pin3, fm.fpioa.GPIO3)
#fm.register(GROVE_pin4, fm.fpioa.GPIO4)

GROVE_3=GPIO(GPIO.GPIO3,GPIO.IN,GPIO.PULL_NONE )

#GROVE_4=GPIO(GPIO.GPIO4,GPIO.IN,GPIO.PULL_NONE )

GROVE_3.value()  #read 1 if floating?
GROVE_4.value() #read 1 if floating!

print("GROVE_pin:"+ str(GROVE_3.value())+"&"+str(GROVE_4.value()) )

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




#pin = fm.fpioa.get_Pin_num(fm.fpioa.GPIOHS18)
try:
    while(True):
        FPS_clock.tick()
        run_cnt =run_cnt +1
        if (run_cnt % 50 ==0):
            RGB_LED_RED()
        print("(input)GROVE_pin status:"+ str(GROVE_3.value())+"__"+str(GROVE_4.value()) )

        time.sleep(0.03)
        if GROVE_3.value() == 0:
             RGB_LED_GREEN()
             #time.sleep(0.2)

        if Btn_A.value() == 0 and isButtonPressedA == 0:
            RGB_LED_ORANGE()
        if Btn_A.value() == 1:
            isButtonPressedA = 0
        if Btn_B.value() == 0 and isButtonPressedB == 0:
            RGB_LED_BLUE()
        if Btn_B.value() == 1:
            isButtonPressedB = 0

        #print("Run_cnt:"+str(run_cnt) )
        print(str(FPS_clock.fps())+"FPS")

except KeyboardInterrupt:
    pass
