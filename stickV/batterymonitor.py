# Untitled - By: ling - Tue Oct 20 2020

import lcd
import utime
import sys
import pmu
from Maix import GPIO
from fpioa_manager import *

#press BtnB (small one located on the side) to hold/unhold readings

#press BtnA to exit!


def display_hold(button):
    hold_status = False
    print("BtnB value(0 pressed):"+str(button.value()))
    if ((button.value() == 0)):
        hold_status = True

    while(hold_status):
        lcd.draw_string(0, 119, "Hold!", lcd.RED, lcd.BLACK)
        utime.sleep(1);
        lcd.draw_string(0, 119, "Hold!", lcd.BLACK, lcd.RED)
        utime.sleep(1);
        if (button.value() == 0):
            lcd.draw_string(0, 119, "     ", lcd.RED, lcd.BLACK)
            hold_status = False
            break

def button_function(button, y):
    lcd.draw_string(0, y, "function" + str(button.value()), lcd.BLUE, lcd.BLACK)
    return


filler = "          "

axp = pmu.axp192()

axp.enableADCs(True)

lcd.init()
lcd.draw_string(0, 0, "Battery Info Develop", lcd.WHITE, lcd.BLACK)
lcd.draw_string(230, 0, "*", lcd.BLUE, lcd.BLACK)

# init button
fm.register(board_info.BUTTON_A, fm.fpioa.GPIO1)
fm.register(board_info.BUTTON_B, fm.fpioa.GPIO2)

button_a = GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!
button_b = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

try:
  while(True):
    val = axp.getVbatVoltage()
    lcd.draw_string(0, 15, "Battery Voltage:" + str(val) + filler, lcd.RED, lcd.BLACK)

    val = axp.getUSBVoltage()
    lcd.draw_string(0, 30, "USB Voltage:" + str(val) + filler, lcd.WHITE, lcd.BLACK)

    val = axp.getUSBInputCurrent()
    lcd.draw_string(0, 45, "USB InputCurrent:" + str(val) + filler, lcd.RED, lcd.BLACK)

    val = axp.getBatteryDischargeCurrent()
    lcd.draw_string(0, 60, "DischargeCurrent:" + str(val) + filler, lcd.GREEN, lcd.BLACK)

    val = axp.getBatteryInstantWatts()
    lcd.draw_string(0, 75, "Instant Watts:" + str(val) + filler, lcd.BLUE, lcd.BLACK)

    val = axp.getTemperature()
    lcd.draw_string(0, 90, "Temperature:" + str(val) + filler, lcd.BLUE, lcd.BLACK)

    lcd.draw_string(80, 105, "Press Button B:Hold", lcd.RED, lcd.BLACK)
    lcd.draw_string(80, 119, "Press Button A:Exit", lcd.RED, lcd.BLACK)

    display_hold(button_b)
    if (button_a.value() == 0):
        print("BtnA pressed!")
        break
    utime.sleep(1)

except Exception as e:
    sys.print_exception(e)

finally:
  lcd.draw_string(230, 0, " ", lcd.BLUE, lcd.BLACK)
  print("Finished")
  sys.exit()
