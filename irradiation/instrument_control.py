'''
Instrument control based on the icicle library: https://gitlab.cern.ch/ethz-phase2-pixels/icicle
'''

from icicle.tti import TTI
from icicle.keithley2000 import Keithley2000
import time

class PowerSupplyController:
    def __init__(self, resource, outputs):
        self.tti = TTI(resource=resource, outputs=outputs)

    def power_on(self, channel):
        with self.tti:
            return self.tti.on(channel)

    def power_off(self, channel):
        with self.tti:
            return self.tti.off(channel)

    def set_voltage(self, channel, value):
        with self.tti:
            return self.tti.set_channel('VOLTAGE', channel, value)

    def read_voltage(self, channel):
        with self.tti:
            return self.tti.measure(channel)[0]

    def set_current(self, channel, value):
        with self.tti:
            return self.tti.set_channel('CURRENT', channel, value)

    def read_current(self, channel):
        with self.tti:
            return self.tti.measure(channel)[1]

    def power_cycle(channel='ALL'):
        with self.tti:
            self.tti.off(channel)
            time.sleep(.5)
            return self.tti.on(channel)

class MultimeterController:
    def __init__(self, resource):
        self.keithley2000 = Keithley2000(resource=resource)
        with self.keithley2000:
            self.keithley2000.set_all_line_integrations(1)

    def measure_voltage(self):
        with self.keithley2000 as multimeter:
            if (multimeter.query('CONFIGURE', no_lock=True).strip('"') != 'VOLT:DC' or int(multimeter.query('INITIALISE_CONTINUOUS', no_lock=True)) == 1):
                multimeter.set('CONFIGURE', 'VOLT:DC', no_lock=True)
            return next(multimeter.measure('VOLT:DC'))
