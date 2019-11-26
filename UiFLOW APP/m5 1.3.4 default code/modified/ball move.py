# -------- PREW_GYRO --------
#FIRE: SHQ200: iic address:0x6c
#BMM150  3-axis digital geomagnetic sensor to be used in compass applications
_pos = [0, 0]

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
            bmm.set_normal_mode()#??
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
                obj['color'] = lcd.BLACK
                lcd.font(lcd.FONT_Default)
                lcd.print("bmm150", 190, 20, 0xFFAAAA)# 2# m5stack stuck in this
        else:
            lcd.font(lcd.FONT_Default)
            lcd.print("IMU:MPU9250", 190, 20, 0xFFAAAA)
            obj['color'] = lcd.BLACK
    elif obj['imu'].address == 0x6c:
        if get_bmm150_status():
            obj['color'] = lcd.BLUE
            lcd.font(lcd.FONT_Default)
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
    ACC_X=val_x/40   #test bz ZL
    ACC_Y=val_y/40   #test bz ZL
    ACC_Z=val_z/40   #test bz ZL

    lcd.print("Acc_X:"+str(ACC_X), 40, 165, 0xFFAAAA)   #test bz ZL
    lcd.print("Acc_y:"+str(ACC_Y), 40, 180, 0xFFAAAA)   #test bz ZL
    lcd.print("Acc_z:"+str(ACC_Z), 40, 195, 0xFFAAAA)   #test bz ZL
    buffer.pop()
    buffer.insert(0, [int(val_x//4), int(val_y//4)])
    val_x = 0
    val_y = 0
    #7 times sum for the ball move value?
    for i in range(0, 6):
        val_x += buffer[i][0]
        val_y += buffer[i][1]
    ball_move(val_x, -val_y, obj['color'])
    obj['buf'] = buffer
    time.sleep_ms(10)

def gyro_end(obj):
    obj = {}
    unit.deinit()

prewstate.register("PREW_GYRO", MState(start=gyro_start, loop=gyro_loop, end=gyro_end))