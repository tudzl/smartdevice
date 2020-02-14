# Hello World sensor Example
#
# Welcome to the MaixPy IDE!
# 1. Conenct board to computer
# 2. Select board at the top of MaixPy IDE: `tools->Select Board`
# 3. Click the connect buttion below to connect board
# 4. Click on the green run arrow button below to run the script!

import sensor, image, time, lcd

#lcd.init(freq=15000000)
sensor.reset()                      # Reset and initialize the sensor. It will
                                    # run automatically, call sensor.run(0) to stop
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
#sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_hmirror(1)
sensor.set_vflip(1)
sensor.set_framesize(sensor.VGA)
sensor.skip_frames(time = 1000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.
Zeit_end = 60*1000
condition = True
Zeit_Anfang = time.ticks_ms()
run_cnt = 0 ;
while(condition):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    #lcd.display(img)               # Display on LCD
    print(clock.fps())              # Note: MaixPy's Cam runs about half as fast when connected
    run_cnt = run_cnt+1                               # to the IDE. The FPS should increase once disconnected.
    Zeit_jetzt=time.ticks_ms()      # get millisecond counter
    print("Time passed:"+str(Zeit_jetzt/1000)+"s, run:"+str(run_cnt))
    if (Zeit_jetzt-Zeit_Anfang > Zeit_end):
       print("Test ends now!")
       condition = False
