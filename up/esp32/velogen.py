import sys
sys.path.append('esp32/')
from machine import Pin, SoftI2C
from ssd1306 import SSD1306_I2C
from ina219 import Ina219
from btns import Btns

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
