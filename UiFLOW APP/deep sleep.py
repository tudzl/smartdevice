   

# Complete project details at https://RandomNerdTutorials.com

https://github.com/espressif/arduino-esp32/blob/master/package/package_esp32_index.template.json
li.zhou@staubli.com

https://github.com/espressif/arduino-esp32/releases/download/1.0.2/esp32-1.0.2.zip

https://github.com/espressif/arduino-esp32/releases/tag/1.0.3-rc1

import machine

import esp32

from machine import Pin

from time import sleep



wake1 = Pin(14, mode = Pin.IN)



#level parameter can be: esp32.WAKEUP_ANY_HIGH or esp32.WAKEUP_ALL_LOW

esp32.wake_on_ext0(pin = wake1, level = esp32.WAKEUP_ANY_HIGH)



#your main code goes here to perform a task



print('Im awake. Going to sleep in 10 seconds')

sleep(10)

print('Going to sleep now')

machine.deepsleep()