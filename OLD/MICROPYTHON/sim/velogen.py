import pygame
from random import randint
from framebuf import FrameBuffer, MONO_HLSB
from time import sleep
import sys
sys.path.append('../')
from gui import Gui


class Ina219(object):
    def __init__(self, i2c, adr=0x40, r_shunt=50):
        pass

    def getI(self):
        ''' current in [mA] '''
        return randint(-100, 500)

    def getV(self):
        ''' voltage in [mV] '''
        return randint(5000, 8400)


class Btns():
    def __init__(self, ios=[0], thr=0.05):
        self.states = [False] * len(ios)
        self.states_ = [False] * len(ios)
        self.cbs = [None] * len(ios)

    def update(self):
        ks = pygame.key.get_pressed()
        self.states = [ks[k] > 0 for k in range(49, 53)]
        for i, (s_, s) in enumerate(zip(self.states_, self.states)):
            if s_ and not s and self.cbs[i] is not None:
                self.cbs[i]()
            self.states_[i] = s
        return self.states


class Pin():
    def __init__(self, gpio, mode=None, pull=None):
        self.val = False

    def on(self):
        self.val = True

    def off(self):
        self.val = False

    def value(self, val=None):
        if val is not None:
            self.val = val
        return self.val


class Velogen():
    def __init__(self):
        self.EN_DYN = Pin(16)
        self.EN_DYN.on()
        self.SPEED_PULSE = Pin(15)

        self.isAP = False

        self.oled = FrameBuffer(
            bytearray(128 * 64 // 8), 128, 64, MONO_HLSB, True
        )

        self.ina = Ina219(None)
        self.btns = Btns([1, 2, 3, 4])

    def getSpeed(self):
        return randint(0, 555) / 10

    def toggleAP(self):
        self.isAP = not self.isAP

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
        for i in pygame.event.get():
            if i.type == pygame.QUIT:
                quit()

        vg.btns.update()
        gui.draw_screen()
        sleep(0.05)


if __name__ == '__main__':
    main()
