from micropython import const
from uiflow import *
import  machine
from mstate import *
from m5stack import *
from m5ui import *
from uiflow import *
import imu
import unit

setScreenColor(0x222222)
#rfid0 = unit.get(unit.RFID, unit.PORTA)
title = M5Title(title="   ESP32 GYRO demo", x=3 , fgcolor=0xff9900, bgcolor=0x1F1F1F)
_pos = [0, 0]


imu0 = imu.IMU()
label0 = M5TextBox(20, 40, "GRYO status:", lcd.FONT_Default,0xAFFFFF, rotate=0)

  if get_bmm150_status(): 
    label0.setText("GRYO status: OK")
    ACC_X = imu0.acceleration[0]
    ACC_Y = imu0.acceleration[1]
    ACC_Z = imu0.acceleration[2]
    #imu0.gyro[0]
    #imu0.acceleration[0]
  else:
    print(imu0.ypr[1])



while True:
    gyro_start()



def ball_move(x, y, color):
    global _pos
    if x > 42:
        x = 42
    elif x < -42:
        x = -42
    if y > 42:
        y = 42
    elif y < -42:
        y = -42
    x += 93
    y += 93
    if (not x == _pos[0]) or (not y == _pos[1]):
        lcd.rect(_pos[0]-11, _pos[1]-11, 22, 22, lcd.WHITE, lcd.WHITE)  # clean
        lcd.circle(x, y, 10, color, color)  # draw
        _pos[0] = x
        _pos[1] = y

def get_bmm150_status():
    import bmm150
    state = 0
    bmm = bmm150.Bmm150()
    if bmm.available():
        if bmm.readID() == 0x32:
            state = 1
            bmm.set_normal_mode()
            if bmm.readData()[1] == 0:
                time.sleep_ms(200)
                if bmm.readData()[1] == 0:
                    state = 0
    return state

def gyro_start(obj):
    global _pos
    import i2c_bus

    _pos = [0, 0]
    lcd.image(0, 0, '/flash/img/3-3.jpg', type=lcd.JPG)
    obj['color']= lcd.RED
    from lib import imu
    obj['imu'] = imu.IMU()
    if obj['imu'].address == 0x68:
        if obj['imu'].whoami == 0x19:
            if get_bmm150_status():
                obj['color'] = lcd.GREEN
                lcd.print("IMU:MPU9250", 190, 20, 0xFFAAAA)
        else:
            obj['color'] = lcd.BLACK
    elif obj['imu'].address == 0x6c:
        if get_bmm150_status():
            obj['color'] = lcd.BLUE
            lcd.print("IMU:SHQ200", 190, 20, 0xFFAAAA)
    obj['buf'] = [[0, 0] for i in range(0, 6)]
    lcd.rect(65, 65, 60, 60, lcd.WHITE, lcd.WHITE)  # old pic dot clean

def gyro_loop(obj):
    imu = obj['imu']
    buffer = obj['buf']
    val_x = 0
    val_y = 0
    val_z = 0
    ACC_X = 0
    ACC_Y = 0
    ACC_Z = 0
    Mag_X = 0
    Mag_Y = 0
    Mag_Z = 0
    #4 times average sampling
    for i in range(0, 4):
        raw = imu.acceleration
        val_x += (raw[0] ) * 10
        val_y += (raw[1] ) * 10
        val_z += (raw[2] ) * 10
    #ACC_X=val_x/40
    #ACC_Y=val_y/40
    #ACC_Z=val_z/40
    #lcd.print("Acc_XYZ:"+str(ACC_X)+str(ACC_Y)+str(ACC_Z), 160, 180, 0xFFAAAA)
    buffer.pop()
    buffer.insert(0, [int(val_x//4), int(val_y//4)])
    val_x = 0
    val_y = 0
    for i in range(0, 6):
        val_x += buffer[i][0]
        val_y += buffer[i][1]
    ball_move(val_x, -val_y, obj['color'])
    obj['buf'] = buffer
    time.sleep_ms(10)

def gyro_end(obj):
    obj = {}
    unit.deinit()