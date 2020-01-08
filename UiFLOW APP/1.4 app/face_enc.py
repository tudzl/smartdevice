#etchasketch.py
# https://m5stack.hackster.io/lukasmaximus89/m5stack-faces-encoder-etch-a-sketch-ed542d
# test by zl, 2020.1.8

from m5stack import *
from m5ui import *
from uiflow import *
import face

setScreenColor(0x222222)

faces_encode = face.get(face.ENCODE)
label1 = M5TextBox(28, 213, "Text", lcd.FONT_Default,0xFFFFFF, rotate=0)
label2 = M5TextBox(112, 210, "Text", lcd.FONT_Default,0xFFFFFF, rotate=0)
label3 = M5TextBox(196, 210, "Text", lcd.FONT_Default,0xFFFFFF, rotate=0)

xpos = None
ypos = None

xpos = 160
ypos = 120
faces_encode.clearValue()
while True:
  label1.setText(str(faces_encode.getValue()))
  label2.setText(str(faces_encode.getDir()))
  if (btnA.isPressed()) and (faces_encode.getDir()) == 0:
    xpos = (xpos if isinstance(xpos, int) else 0) + 1
  elif (btnA.isPressed()) and (faces_encode.getDir()) == 1:
    xpos = (xpos if isinstance(xpos, int) else 0) + -1
  elif (btnB.isPressed()) and (faces_encode.getDir()) == 0:
    ypos = (ypos if isinstance(ypos, int) else 0) + 1
  elif (btnB.isPressed()) and (faces_encode.getDir()) == 1:
    ypos = (ypos if isinstance(ypos, int) else 0) + -1
  elif xpos >= 320:
    xpos = 320
  elif xpos <= 0:
    xpos = 0
  elif ypos <= 0:
    ypos = 0
  elif ypos >= 240:
    ypos = 240
  lcd.pixel(xpos, ypos, 0xffffff)
  wait_ms(2)