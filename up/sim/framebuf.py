from math import ceil
from copy import copy
try:
    from pygame import display
except ImportError:
    display = None


MONO_VLSB = 0
MONO_HLSB = 1
MONO_HMSB = 2
RGB565 = 3
GS2_HMSB = 4
GS4_HMSB = 5
GS8 = 6


class FrameBuffer():
    def __init__(self, buffer, width, height, map, pygame=False):
        '''
        simulate the micropython framebuffer class to test the gui library
        with normal python on a PC
        '''
        self.buffer = buffer
        self.width = width
        self.height = height

        self.surf = None
        if pygame and display is not None:
            display.init()
            self.surf = display.set_mode((width, height + 2))

    def scroll(self, x, y):
        cp = copy(self)
        cp.buffer = copy(self.buffer)
        self.blit(cp, x, y)

    def hline(self, x, y, w, c):
        self.fill_rect(x, y, w, 1, c)

    def vline(self, x, y, h, c):
        self.fill_rect(x, y, 1, h, c)

    def rect(self, x, y, w, h, c):
        self.hline(x, y, w, c)
        self.hline(x, y + h - 1, w, c)
        self.vline(x, y, h, c)
        self.vline(x + w - 1, y, h, c)

    def fill_rect(self, x, y, w, h, c):
        for yi in range(h):
            for xi in range(w):
                self.setpixel(x + xi, y + yi, c)

    def fill(self, c):
        self.fill_rect(0, 0, self.width, self.height, c)

    def line(self, x1, y1, x2, y2, c):
        dx = abs(x2 - x1)
        sx = 1 if x1 < x2 else -1
        dy = -abs(y2 - y1)
        sy = 1 if y1 < y2 else -1
        err = dx + dy
        while (True):
            self.setpixel(x1, y1, c)
            if x1 == x2 and y1 == y2:
                break
            e2 = 2 * err
            if e2 >= dy:
                err += dy
                x1 += sx
            if e2 <= dx:
                err += dx
                y1 += sy

    def blit(self, fb, x, y):
        for yi in range(fb.height):
            for xi in range(fb.width):
                self.setpixel(x + xi, y + yi, fb.getpixel(xi, yi))

    def pixel(self, x, y, c=None):
        if c is not None:
            self.setpixel(x, y, c)
        return self.getpixel(x, y)

    def setpixel(self, x, y, col):
        if x >= self.width or y >= self.height:
            return
        byte = (x >> 3) + ceil(self.width / 8) * y
        bit = 7 - (x & 7)
        mask = 1 << bit
        self.buffer[byte] &= ~mask
        if col:
            self.buffer[byte] |= mask

    def getpixel(self, x, y):
        if x >= self.width or y >= self.height:
            return False
        byte = (x >> 3) + ceil(self.width / 8) * y
        bit = 7 - (x & 7)
        mask = 1 << bit
        return (self.buffer[byte] & mask) > 0

    def __str__(self):
        s = ''
        for yi in range(self.height):
            s += '\n'
            for xi in range(self.width):
                if self.getpixel(xi, yi):
                    s += 'o'
                else:
                    s += ' '
        return s

    def show(self):
        if self.surf is None:
            print(chr(27) + "[2J")
            print(self)
        else:
            for yi in range(self.height):
                for xi in range(self.width):
                    col = (0x00, 0x00, 0x00)
                    if self.getpixel(xi, yi):
                        if yi < 16:
                            col = (0xFF, 0x66, 0x00)
                        else:
                            col = (0x00, 0x99, 0xFF)
                    yi_ = yi + 2 if yi >= 16 else yi
                    self.surf.set_at((xi, yi_), col)
            display.flip()
