from machine import Pin, TouchPad


class Btns():
    def __init__(self, ios=[0], thr=0.05):
        self.ts = [TouchPad(Pin(p)) for p in ios]
        # values without touching
        cals = [t.read() for t in self.ts]
        # thresholds (considered as touched when below that value)
        self.thrs = [int(c * (1 - thr)) for c in cals]
        self.states = [False] * len(ios)
        self.states_ = [False] * len(ios)
        self.cbs = [None] * len(ios)

    def update(self):
        self.states = [t.read() < thr for t, thr in zip(self.ts, self.thrs)]
        for i, (s_, s) in enumerate(zip(self.states_, self.states)):
            if s_ and not s and self.cbs[i] is not None:
                self.cbs[i]()
            self.states_[i] = s
        return self.states
