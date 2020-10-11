# This file is executed on every boot (including wake-boot from deepsleep)
import network
# wifi passwords
from wpw import wpws
import webrepl
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


webrepl.start()
start_sta('***REMOVED***')
# start_ap()

from velogen import Velogen
vg = Velogen()

scr = 1


def btn_l():
    global scr
    scr -= 1
    if scr < 0:
        scr = 0


def btn_dyn():
    vg.EN_DYN.value(not vg.EN_DYN.value())


def btn_r():
    global scr
    scr += 1
    if scr > 3:
        scr = 3


vg.btns.cbs[0] = btn_l
vg.btns.cbs[1] = btn_dyn
vg.btns.cbs[3] = btn_r

while True:
    vg.btns.update()
    vg.draw_screen(scr)
    sleep(0.05)
