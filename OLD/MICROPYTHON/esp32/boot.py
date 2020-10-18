# This file is executed on every boot (including wake-boot from deepsleep)
import network
# wifi passwords
from wpw import wpws
import webrepl
from time import sleep

wlan = None


def start_sta(ssid):
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
start_sta('')
# start_ap()
