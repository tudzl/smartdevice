# IIC scan code - By: ling - Mon Jan 6 2020
# version 1.1 test ok!
#with open("IIC scan.py") as f:
#exec(f.read())



from machine import I2C
from fpioa_manager import *
from Maix import GPIO
from machine import I2C
from modules import ws2812
import time
import gc
from modules import ws2812

FPS_clock = time.clock()

print("listdir:" )
os.listdir()

#fm.register(board_info.LED_R, fm.fpioa.GPIO0)

# GROVE pins init as SDA and SCL
print("GROVE_pin3 and pin4 now set as IIC0 SDA and SCL" )
GROVE_pin3 = 35 #physical port pin number
GROVE_pin4 = 34
# unreg

fm.unregister(GROVE_pin3, fm.fpioa.GPIOHS13)
fm.unregister(GROVE_pin4, fm.fpioa.GPIOHS14)

fm.register(GROVE_pin3, fm.fpioa.I2C0_SDA ,  GPIO.PULL_UP )
fm.register(GROVE_pin4, fm.fpioa.I2C0_SCLK ,  GPIO.PULL_UP )

# fit to m5stack fire grove C: pin 3 TX, pin4 RX
#fm.register(GROVE_pin3, fm.fpioa.UART1_RX ,  GPIO.PULL_UP )
#fm.register(GROVE_pin4, fm.fpioa.UART1_TX ,  GPIO.PULL_UP )
#uart_out = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)


#i2c = I2C(I2C.I2C2, mode=I2C.MODE_MASTER, freq=400000, scl=GROVE_pin3, sda=GROVE_pin4, addr_size=7)
#i2c = I2C(I2C.I2C0, mode=I2C.MODE_MASTER, freq=400000, scl=34, sda=35, addr_size=7)
i2c = I2C(I2C.I2C0, mode=I2C.MODE_MASTER, freq=110000, addr_size=7)
#slave mode
#i2c = I2C(I2C.I2C0, I2C.MODE_SLAVE, freq=400000, addr = 0x99, addr_size=7, on_recieve=None, on_transmit=None, on_event=None)
#i2c = I2C(I2C.I2C0, freq=100000, scl=34, sda=35)

#480960
#使用 pye("hello.py") 可以创建文件并进入编辑模式， 快捷键等使用说明可以在这里
#然后按 Ctrl+S 按 Enter 键保存， 按 Ctrl+Q 退出编辑

# RGB Pixel LED
PIXEL_LED_pin = 8
PIXEL_LED_num = 1
PIXEL_LED = class_ws2812 = ws2812(PIXEL_LED_pin,PIXEL_LED_num)
Color_Green = 0xF0
Color_Red = 0xF0
Color_Blue = 0xF0

run_cnt= 0

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
def RGB_LED_GREEN2():
    #class_ws2812.set_led(num,color), color` : 该灯珠赋值的颜色，为 `tuple` 类型， （R,G,B）
    PIXEL_LED = class_ws2812.set_led(0,(0,Color_Green,0))
    PIXEL_LED = class_ws2812.display()
    time.sleep(0.2)
    PIXEL_LED = class_ws2812.set_led(0,(0,0,0))
    PIXEL_LED = class_ws2812.display()


try:
    while(True):
        run_cnt = run_cnt +1
        FPS_clock.tick()
        #addrList =[0]
        addrList = i2c.scan()
        DEV_num=len(addrList)
        if (DEV_num>0):
             RGB_LED_GREEN2()
             time.sleep(0.2)
        print("IIC scan found devices:"+str(DEV_num))
        print(addrList)
        RGB_LED_RED()
        time.sleep(0.1)
        print("GC free mem: "+ str(gc.mem_free()/1000)+"KB")
        gc.collect()
        print("run cnt:"+str(run_cnt)+" @ "+str(FPS_clock.fps())+" FPS")



except KeyboardInterrupt:
    pass
