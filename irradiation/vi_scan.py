import time, csv, os
import numpy as np
from icicle.double_relay_board import DoubleRelayBoard
from icicle.keithley2000 import Keithley2000
from icicle.tti import TTI

def vi_curves(outdir, startingCurrent, finalCurrent, currentStep):
    drb         = DoubleRelayBoard(resource='ASRL/dev/ttyUSB1::INSTR', resource2='ASRL/dev/ttyUSB3::INSTR')
    multimeter  = Keithley2000(resource='ASRL/dev/ttyUSB5::INSTR')
    lv          = TTI(resource='ASRL/dev/ttyACM0::INSTR', outputs=2)
    PIN_DICT    = {
            'aa' : 'VinA',
            'ab' : 'VinD',
            'ac' : 'VddA',
            'ad' : 'VddD',
            'ae' : 'GNDA_ref',
            'af' : 'GNDD_ref',
            'ag' : 'VrefA',
            'ah' : 'VrefD',
            'b'  : 'VrextA',
            'c'  : 'VrextD',
            'd'  : 'Vofs',
            'e'  : 'Vref_ADC',
            #'f'  : 'Vmux',
            }
    if not os.path.exists(outdir):
        os.makedirs(outdir)
    outfile = os.path.join(outdir,f'croc_vi_curves_{time.strftime("%Y%m%d-%H%M%S")}.csv')
    if not os.path.isfile(outfile):
        with open(outfile, 'a+') as f:
            csv.writer(f).writerow( ['Iin']+list(PIN_DICT.values()) )
    
    with lv:
        oldVoltages = [ lv.query_channel('VOLTAGE', ch) for ch in (1,2) ]
        oldCurrents = [ lv.query_channel('CURRENT', ch) for ch in (1,2) ]
        for ch in (1,2):
            lv.set_channel('CURRENT', ch, startingCurrent)
            lv.set_channel('VOLTAGE', ch, 2.5)
    with multimeter:
        multimeter.set('CONFIGURE', 'VOLT:DC')
        multimeter.set_all_line_integrations(1)
    time.sleep(.2)

    if startingCurrent>finalCurrent and currentStep>0:
        currentStep = -currentStep
    currents = numpy.arange(startingCurrent, finalCurrent, currentStep)
    for i in currents:
        with lv:
            for ch in (1,2):
                lv.set_channel('CURRENT', ch, i)
        measDict = {}
        for pin,pinName in PIN_DICT.items():
            #logger.info(f'== Now connecting: {pin} ({pinName})')
            # Connect pin
            with drb: drb.set_pin(pin)
            #time.sleep(.1)
            with multimeter:
                measDict[pinName] = multimeter.measure('VOLT:DC', cycles=line_integration_cycles)[0]
        for pinName in measDict:
            if pinName.endswith('A'):
                measDict[pinName] -= measDict['GNDA_ref']
            elif pinName.endswith('D'):
                measDict[pinName] -= measDict['GNDD_ref']
        measurements = [i] + list(measDict.values())
        with open(outfile, 'a+') as f:
            csv.writer(f).writerow(measurements)
    with drb: drb.set_pin('OFF')
    with lv:
        for ch in (1,2):
            lv.set_channel('VOLTAGE', ch, oldVoltages[ch-1])
            lv.set_channel('CURRENT', ch, oldCurrents[ch-1])
