#UART and  timer test

from m5stack import *
from m5ui import *
from uiflow import *
import imu
import unit
import machine



def handleInterrupt(timer):
    global interruptCounter
    interruptCounter = interruptCounter+1
    rgb.setColorAll(0xcc33cc+run_cnt)


setScreenColor(0x222222)
title = M5Title(title="  ESP32 UART debug v1.2", x=3 , fgcolor=0xff99aa, bgcolor=0x1F1F1F)
#light0 = unit.get(unit.LIGHT, unit.PORTB)
#neopixel0 = unit.get(unit.NEOPIXEL, unit.PORTB, 10)
#rfid0 = unit.get(unit.RFID, unit.PORTA)
label_cnt = M5TextBox(220, 225, "CNT: ", lcd.FONT_Default,0xFFEEBB, rotate=0)  # run count


#imu0 = imu.IMU()
label_info = M5TextBox(48, 35, "UART RX:", lcd.FONT_DejaVu24,0xFFFFFF, rotate=0)
label_RX = M5TextBox(48, 60, "UART RX:", lcd.FONT_DejaVu24,0xFFFFFF, rotate=0)
interruptCounter = 0
totalInterruptsCounter = 0
 
timer1 = machine.Timer(0)  
  
timer1.init(period=1000, mode=machine.Timer.PERIODIC, callback=handleInterrupt)


uart = machine.UART(1, tx=17, rx=16)
uart.init(9600, bits=8, parity=None, stop=1)
UART_RX_buf = ""
UART_RX_msg = ""
run_cnt =0
#timerSch.run('timer1', 1000, 0x01)
rgb.setColorAll(0x009900) #green
while True:
  run_cnt = run_cnt+1  # cost about 10 us
  rgb.setColorAll(0x009900) #green
  if interruptCounter>0:
    """
    if we detect the interrupt, we will need to decrement the interruptCounter variable to signal that we will handle it. Since this variable is shared with the ISR and to avoid racing conditions, this decrement needs to be performed in a critical section, which we will implement by simply disabling the interrupts.
    """
    state = machine.disable_irq()
    interruptCounter = interruptCounter-1
    machine.enable_irq(state)  
    totalInterruptsCounter = totalInterruptsCounter+1
    UART_RX_buf=str(uart.readline())
    if len(UART_RX_buf) > 1:
      UART_RX_msg = UART_RX_buf
    label_cnt.setText("run: "+str(run_cnt) )
    label_info.setText("Total INT:"+str(totalInterruptsCounter) )
    label_RX.setText("RX:"+str(UART_RX_msg) )
    uart.write("Total INT:"+str(totalInterruptsCounter))
    
  #UART_RX_buf=str(uart.read())
  #print(UART_RX_buf)
  
  #label0.setText(UART_RX_buf)
  #label_cnt.setText("Run: "+str(run_cnt) )
  wait_ms(10)
