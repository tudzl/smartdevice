# Untitled - By: ling - Tue Oct 20 2020

import lcd
import utime
import sys
from Maix import GPIO
from fpioa_manager import *

def button_function(button, y):
    lcd.draw_string(0, y, "function" + str(button.value()), lcd.BLUE, lcd.BLACK)
    return


lcd.init()
lcd.draw_string(0, 0, "Button Sample", lcd.WHITE, lcd.BLACK)

# init button
fm.register(board_info.BUTTON_A, fm.fpioa.GPIO1)
fm.register(board_info.BUTTON_B, fm.fpioa.GPIO2)

button_a = GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!
button_b = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

try:
  while(True):
    lcd.draw_string(0, 15, "Running", lcd.BLACK, lcd.RED)
    lcd.draw_string(0, 30, "ButtonA : " + str(button_a.value()), lcd.WHITE, lcd.BLACK)
    lcd.draw_string(0, 45, "ButtonB : " + str(button_b.value()), lcd.GREEN, lcd.BLACK)

    button_function(button_a, 60)
    button_function(button_b, 75)

    utime.sleep(0.001)

except Exception as e:
    sys.print_exception(e)

finally:
    print("Finished")
    sys.exit()
