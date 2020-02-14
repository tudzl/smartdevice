#find color ball test demo by zell
#v1.1, 2020.2.14
# for Unit V
import sensor
import image
import lcd
import time
import utime
from machine import UART
from Maix import GPIO
from fpioa_manager import *
#import cpufreq

fm.register(34,fm.fpioa.UART1_TX)
fm.register(35,fm.fpioa.UART1_RX)
uart_out = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_hmirror(1)#for unit V
sensor.set_vflip(1)#for unit V
sensor.set_framesize(sensor.QVGA)
#sensor.set_auto_whitebal(False)#关闭白平衡。白平衡是默认开启的，在颜色识别中，需要关闭白平衡
sensor.skip_frames(30)

clock = time.clock()




sensor.run(1)

while False:
    uart_out.write('TEST\n')
    utime.sleep_ms(100)

target_blue_ballthreshold =(39,   56,  -25,   -3,   -42,   -16)##Lab颜色空间中，L亮度；a的正数代表红色，负端代表绿色；b的正数代表黄色，负端代表兰色
target_lab_threshold = (45,   70,  -60,   -30,   0,   40) ##?
tag_cnt = 0

while True:
    clock.tick()

    img=sensor.snapshot()
    tag_cnt = 0
    #blobs = img.find_blobs([red_threshold_01],   roi = roi2,   area_threshold=1000)
    blobs = img.find_blobs([target_blue_ballthreshold], x_stride = 8, y_stride = 8,area_threshold=100, pixels_threshold = 100, merge = True, margin = 20)
    if blobs:
        max_area = 4
        target = blobs[0]
        for b in blobs:
            if b.area() > max_area:
                max_area = b.area()
                target = b
                tag_cnt = tag_cnt+1
        #uart function?
        if uart_out.read(4096):
            area = target.area()
            dx = 120 - target[6]
            hexlist = [(dx >> 8) & 0xFF, dx & 0xFF, (area >> 16) & 0xFF, (area >> 8) & 0xFF, area & 0xFF]
            uart_out.write(bytes(hexlist))
        else:
            pass
        print("target area:"+ str(target.area())+" pixels" )
        print("target count:"+ str(tag_cnt) )
        tmp=img.draw_rectangle(target[0:4])# Draw a rect around the blob.
        tmp=img.draw_cross(target[5], target[6])#在目标颜色区域的中心画十字形标记
        c=img.get_pixel(target[5], target[6])# center pixel?

    else:
        if uart_out.read(4096):
            hexlist = [0x80, 0x00, 0x00, 0x00, 0x00]
            uart_out.write(bytes(hexlist))
        else:
            pass

    print("FPS: ",str(clock.fps()) )
