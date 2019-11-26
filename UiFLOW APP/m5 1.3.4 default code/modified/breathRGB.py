from m5stack import *
from m5ui import *
from uiflow import *

setScreenColor(0x111111)

LCD_EN =20

circle0 = M5Circle(94, 29, 15, 0xFFFFFF, 0xFFFFFF)
circle1 = M5Circle(219, 29, 15, 0xFFFFFF, 0xFFFFFF)
lcd.image(70, 60, 'res/person.jpg', type=lcd.JPG)
import random

R = None
G = None
B = None
i = None

lcd.setBrightness(LCD_EN)

while True:
  R = random.randint(0, 255)
  G = random.randint(0, 255)
  B = random.randint(0, 255)
  for i in range(0, 256, 10):
    rgb.setColorFrom(6 , 10 ,(R << 16) | (G << 8) | B)
    rgb.setColorFrom(1 , 5 ,(G << 16) | (R << 8) | B)
    rgb.setBrightness(i)
    wait_ms(2)
  for i in range(255, -1, -10):
    rgb.setColorFrom(6 , 10 ,(R << 16) | (G << 8) | B)
    rgb.setColorFrom(1 , 5 ,(G << 16) | (R << 8) | B)
    rgb.setBrightness(i)
    wait_ms(10)
  wait_ms(2)
