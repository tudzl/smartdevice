SSID_NAME = "SSID名"
SSID_PASS = "パスワード"

import utime
import network
# ==== connecti to wifi access point ============================================
def connect_wifi(ssid, passkey, timeout=10):
    wifi= network.WLAN(network.STA_IF)
    if wifi.isconnected() :
        print('already Connected.    connect skip')
        return wifi
    else :
        wifi.active(True)
        wifi.connect(ssid, passkey)
        while not wifi.isconnected() and timeout > 0:
            print('.')
            utime.sleep(1)
            timeout -= 1
    
    if wifi.isconnected():
        print('Connected')
        return wifi
    else:
        print('Connection failed!')
        return null

wifi = connect_wifi(SSID_NAME, SSID_PASS)
if not wifi :
    sys.exit(0)