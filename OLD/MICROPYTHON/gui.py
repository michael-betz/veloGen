from nanogui import Label, LED, refresh
from writer import Writer
from fonts import font6
# import fredoka_30
from fonts import pacifico_35
from fonts import fa_30


class Gui:
    def __init__(self, vg):
        self.vg = vg
        wri_big = Writer(vg.oled, pacifico_35, verbose=False)
        wri_small = Writer(vg.oled, font6, verbose=False)
        wri_fa = Writer(vg.oled, fa_30, verbose=False)
        self.lbl_status = Label(wri_small, 0, 0, 127)

        self.cur_scr = -1
        self.scr_objs = [
            [
                # km/h screen
                # dynamic objects have a call back (cb) lambda
                Label(
                    wri_big, 16, 0, '22.2', right=True,
                    cb=lambda: '{:0.1f}'.format(self.vg.getSpeed())
                ),
                # static objects have None
                Label(wri_small, 37, 90, 'km/h')
            ], [
                # voltage screen
                Label(
                    wri_big, 16, 0, '2222', right=True,
                    cb=lambda: str(self.vg.ina.getV())
                ),
                Label(wri_small, 37, 100, 'mV')
            ], [
                # current screen
                Label(
                    wri_big, 16, 0, '-222', right=True,
                    cb=lambda: '{:4d}'.format(self.vg.ina.getI())
                ),
                Label(wri_small, 37, 100, 'mA')
            ], [
                # power screen
                Label(
                    wri_big, 16, 0, '-2.22', right=True,
                    cb=lambda: '{:0.2f}'.format(
                        self.vg.ina.getV() * self.vg.ina.getI() / 1000000
                    )
                ),
                Label(wri_small, 37, 110, 'W')
            ], [
                # Hotspot on off screen
                Label(
                    wri_fa, 22, 8, '',
                    cb=lambda: '' if self.vg.isAP else ''
                ),
                Label(wri_small, 22, 65, 'AP mode'),
                Label(wri_small, 38, 65, 'velogen')
            ]
        ]

        refresh(self.vg.oled, True)

    def draw_screen(self, n=None):
        if n is None:
            n = self.cur_scr
        sobjs = self.scr_objs[n]
        if n != self.cur_scr:
            # draw static objects only when screen is changed
            refresh(self.vg.oled, True)  # clear screen
            self.lbl_status.show(True)
            for sobj in sobjs:
                sobj.show(True)
            self.cur_scr = n
            return
        # draw dynamic objects every cycle
        for sobj in sobjs:
            sobj.show(False)
        refresh(self.vg.oled)

    def left(self):
        if self.cur_scr > 0:
            self.draw_screen(self.cur_scr - 1)

    def right(self):
        if self.cur_scr < len(self.scr_objs) - 1:
            self.draw_screen(self.cur_scr + 1)

    def select(self):
        if self.cur_scr == 4:
            self.vg.toggleAP()
        else:
            self.vg.toggleDyn()
