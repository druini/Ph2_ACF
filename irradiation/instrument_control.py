from icicle.tti import TTI


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

    def read_current():
        with self.tti:
            return self.tti.measure(channel)[1]
