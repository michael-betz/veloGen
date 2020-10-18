from ustruct import pack, unpack

class Ina219(object):
    def __init__(self, i2c, adr=0x40, r_shunt=50):
        """
        voltage / current / power monitor
        r_shunt [mOhm]
        """
        self.i2c = i2c
        self.adr = adr
        self._w(0, 1 << 15)  # reset
        self.cfg = self._r(0)
        self.r_shunt = r_shunt

    def _r(self, reg, signed=False):
        val_ = self.i2c.readfrom_mem(self.adr, reg, 2)
        return unpack('>h' if signed else '>H', val_)[0]

    def _w(self, reg, val):
        ''' always unsigned '''
        val_ = int.to_bytes(val, 2, 'big')
        self.i2c.writeto_mem(self.adr, reg, val_)

    def _setcfg(self, shift, n_bits, val, write=True):
        mask = (1 << n_bits) - 1
        mask <<= shift
        val <<= shift
        self.cfg &= ~mask
        self.cfg |= (val & mask)
        if write:
            self._w(0, self.cfg)

    def off(self):
        self._w(0, self.cfg & 0xFFF8)

    @property
    def bus_32v(self):
        return (self.cfg >> 13) & 0x1

    @bus_32v.setter
    def bus_32v(self, val):
        '''
        bus_32v = True (16 V FSR), False (32 V FSR)
        '''
        self._setcfg(13, 1, val)

    @property
    def pga(self):
        return (self.cfg >> 11) & 0x3

    @pga.setter
    def pga(self, val):
        '''
        set shunt voltage gain:
        val = 0 (+- 40 mV), 1 (+- 80 mV), 2 (+- 160 mV) or 3 (+- 320 mV)
        '''
        self._setcfg(11, 2, val)

    @property
    def avg(self):
        return (self.cfg >> 3) & 0x7

    @avg.setter
    def avg(self, val):
        '''
        averaging factor: 0 - 7
        applies to BUS and SHUNT
        always 12 bit mode
        '''
        val |= 8
        self._setcfg(3, 4, val)
        self._setcfg(7, 4, val)

    def on(self):
        self._w(0, self.cfg)

    def getI(self):
        ''' current in [mA] '''
        return self._r(1, True) * 10 // self.r_shunt

    def getV(self):
        ''' voltage in [mV] '''
        return (self._r(2) & 0xFFF8) >> 1
