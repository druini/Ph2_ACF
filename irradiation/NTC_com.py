import numpy as np
import time, logging, sys
from icicle.double_relay_board import DoubleRelayBoard
from icicle.keithley2000 import Keithley2000

DATEFMT = '%Y/%m/%d-%H:%M:%S'
LOGFILE = 'NTC.log'

def tempCelsius(res):
    # based on https://en.wikipedia.org/wiki/Thermistor#B_or_%CE%B2_parameter_equation
    B  = 3435
    R0 = 1e4
    T0 = 25+273
    rinf = R0*np.exp(-B/T0)
    T = B/np.log( res/rinf )
    return T - 273

def main():
    logging.basicConfig(
            format='%(asctime)s [%(levelname)s] %(message)s',
            datefmt=DATEFMT,
            level=logging.INFO,
            handlers=[
                logging.FileHandler(LOGFILE),
                #logging.StreamHandler(sys.stdout)
                ]
            )

    #logging.info('Starting NTC control script')

    multimeter = Keithley2000(resource='ASRL/dev/ttyUSB5::INSTR', reset_on_init=False)
    drb        = DoubleRelayBoard(resource='ASRL/dev/ttyUSB1::INSTR', resource2='ASRL/dev/ttyUSB3::INSTR')

    with drb:
        drb.set_pin('NTC')
        with multimeter:
            multimeter.set('CONFIGURE', 'RES')
            while True:
                res = multimeter.measure('RES')[0]
                temp = tempCelsius(res)
                logging.info(f'{res}, {temp:.2f}')
                time.sleep(1)

if __name__=='__main__':
    main()
