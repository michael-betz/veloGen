# This file is executed on every boot (including wake-boot from deepsleep)
import network
from machine import Pin, SoftI2C
from ssd1306 import SSD1306_I2C
from random import randint
from time import sleep
from ina219 import Ina219
# wifi passwords
from wpw import wpws
import webrepl

wlan = None


def start_sta(ssid='***REMOVED***'):
    global wlan
    oled_print(ssid + '...')
    wlan = network.WLAN(network.STA_IF)
    if not wlan.active() or not wlan.isconnected():
        wlan.active(True)
        print('connecting to:', ssid)
        wlan.connect(ssid, wpws[ssid])


def start_ap():
    oled_print('starting AP ...')
    global wlan
    wlan = network.WLAN(network.AP_IF)  # create access-point interface
    wlan.config(essid='velogen')  # set the ESSID of the access point
    wlan.config(max_clients=10)  # set how many clients can connect
    wlan.active(True)  # activate the interface


def hd(buf):
    for i, b in enumerate(buf):
        if (i % 0x10) == 0:
            print('\n{:4x}: '.format(i), end='')
        print('{:02x} '.format(b), end='')
    print()


def oled_print(*args, y=0):
    oled.fill(0)
    for i, s in enumerate(args):
        oled.text(s, 0, y + i * 8)
    oled.show()


def screen():
    i = 0
    while True:
        if (i % 64) == 0:
            y = randint(0, 64 - 8 * 3)
        try:
            ssid = wlan.config('essid')
            ip = wlan.ifconfig()[0]
        except Exception:
            ssid = ''
            ip = ''
        oled_print(
            ssid,
            ip,
            '{} mV, {} mA'.format(
                ina.getV(),
                ina.getI()
            ),
            y=y
        )
        i += 1
        sleep(0.5)



EN_DYN = Pin(16, Pin.OUT)
EN_DYN.value(0)
SPEED_PULSE = Pin(15, Pin.IN, pull=None)

i2c = SoftI2C(scl=Pin(14), sda=Pin(12))
oled = SSD1306_I2C(128, 64, i2c)

ina = Ina219(i2c, r_shunt=50)
ina.pga = 0  # maximum sensitivity
ina.avg = 7  # maximum averaging

webrepl.start()
# start_sta('***REMOVED***')
start_ap()

screen()
