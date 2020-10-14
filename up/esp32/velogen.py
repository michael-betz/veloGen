# import sys
# sys.path.append('esp32/')
import network
# wifi passwords
from wpw import wpws
from machine import Pin, SoftI2C
from ssd1306 import SSD1306_I2C
from ina219 import Ina219
from btns import Btns
from random import randint
from gui import Gui
from time import sleep

wlan = None


def start_sta(ssid='***REMOVED***'):
    global wlan
    wlan = network.WLAN(network.STA_IF)
    if not wlan.active() or not wlan.isconnected():
        wlan.active(True)
        wlan.connect(ssid, wpws[ssid])


def start_ap():
    global wlan
    wlan = network.WLAN(network.AP_IF)  # create access-point interface
    wlan.config(essid='velogen')  # set the ESSID of the access point
    wlan.config(max_clients=10)  # set how many clients can connect
    wlan.active(True)  # activate the interface


class Velogen():
    def __init__(self):
        self.EN_DYN = Pin(16, Pin.OUT)
        self.EN_DYN.on()
        self.SPEED_PULSE = Pin(15, Pin.IN, pull=None)

        self.i2c = SoftI2C(scl=Pin(14), sda=Pin(12))
        self.oled = SSD1306_I2C(128, 64, self.i2c)

        self.ina = Ina219(self.i2c, r_shunt=50)
        self.ina.pga = 0  # maximum sensitivity
        self.ina.avg = 7  # maximum averaging

        self.btns = Btns([0, 2, 4, 32], 0.05)

    def getSpeed(self):
        return randint(0, 555) / 10

    def toggleAP(self):
        self.isAP = not self.isAP
        wlan.active(False)
        if self.isAP:
            start_ap()
            self.gui.lbl_status.value('AP: velogen')
        else:
            start_sta('***REMOVED***')
            self.gui.lbl_status.value(wlan.config('essid'))

    def toggleDyn(self):
        self.EN_DYN.value(not self.EN_DYN)


def main():
    vg = Velogen()
    gui = Gui(vg)

    def btn_dyn():
        vg.EN_DYN.value(not vg.EN_DYN.value())
        print(vg.EN_DYN.value())

    vg.btns.cbs[0] = gui.left
    vg.btns.cbs[1] = gui.select
    vg.btns.cbs[3] = gui.right

    gui.draw_screen(2)

    while True:
        vg.btns.update()
        gui.draw_screen()
        sleep(0.05)
