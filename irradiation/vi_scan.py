import time, csv, os, toml
import numpy as np
from icicle.double_relay_board import DoubleRelayBoard
from icicle.keithley2000 import Keithley2000
from icicle.tti import TTI
from scan_routine import configureCROC, getTomlFile

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
        'NTC': 'NTC',
        }

def measure_all_pins(pin_dict=PIN_DICT):
    multimeter = Keithley2000(resource='ASRL/dev/ttyUSB5::INSTR', reset_on_init=False)
    drb        = DoubleRelayBoard(resource='ASRL/dev/ttyUSB1::INSTR', resource2='ASRL/dev/ttyUSB3::INSTR')

    measDict = {}
    with multimeter:
        with drb:
            for pin,pinName in PIN_DICT.items():
                #logger.info(f'== Now connecting: {pin} ({pinName})')
                # Connect pin
                drb.set_pin(pin)
                if pin=='NTC':
                    multimeter.set('CONFIGURE', 'RES')
                    measDict[pinName] = multimeter.measure('RES')[0]
                    multimeter.set('CONFIGURE', 'VOLT:DC')
                else:
                    measDict[pinName] = multimeter.measure('VOLT:DC', cycles=1)[0]
    for pinName in measDict:
        if pinName.endswith('A'):
            measDict[pinName] -= measDict['GNDA_ref']
        elif pinName.endswith('D'):
            measDict[pinName] -= measDict['GNDD_ref']
    return measDict

def vi_curves(outdir, startingCurrent, finalCurrent, currentStep):
    multimeter  = Keithley2000(resource='ASRL/dev/ttyUSB5::INSTR')
    lv          = TTI(resource='ASRL/dev/ttyACM0::INSTR', outputs=2)
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

    if startingCurrent>finalCurrent and currentStep>0:
        currentStep = -currentStep
    currents = numpy.arange(startingCurrent, finalCurrent, currentStep)
    for i in currents:
        with lv:
            for ch in (1,2):
                lv.set_channel('CURRENT', ch, i)
        measDict = measure_all_pins()
        measurements = [i] + list(measDict.values())
        with open(outfile, 'a+') as f:
            csv.writer(f).writerow(measurements)
    with lv:
        for ch in (1,2):
            lv.set_channel('VOLTAGE', ch, oldVoltages[ch-1])
            lv.set_channel('CURRENT', ch, oldCurrents[ch-1])
    return True

def vmonitor(outdir):
    if not os.path.exists(outdir):
        os.makedirs(outdir)
    outfile = os.path.join(outdir,f'croc_vmonitor.csv')
    if not os.path.isfile(outfile):
        with open(outfile, 'a+') as f:
            csv.writer(f).writerow( ['time']+list(PIN_DICT.values()) )

    multimeter = Keithley2000(resource='ASRL/dev/ttyUSB5::INSTR')
    with multimeter:
        multimeter.set('CONFIGURE', 'VOLT:DC')
        multimeter.set_all_line_integrations(1)

    measDict = measure_all_pins()
    measurements = [time.strftime("%Y-%m-%d %H:%M:%S")] + list(measDict.values())
    with open(outfile, 'a+') as f:
        csv.writer(f).writerow(measurements)
    return True

def I_vs_VrefTrim(outdir, xmlConfig):
    vtrimA = 0b1010
    vtrimD = 0b1111 # from separate VrefTrimming
    multimeter = Keithley2000(resource='ASRL/dev/ttyUSB5::INSTR', reset_on_init=False)
    drb        = DoubleRelayBoard(resource='ASRL/dev/ttyUSB1::INSTR', resource2='ASRL/dev/ttyUSB3::INSTR')
    lv         = TTI(resource='ASRL/dev/ttyACM0::INSTR', outputs=2, reset_on_init=False)

    if not os.path.exists(outdir):
        os.makedirs(outdir)
    outfile = os.path.join(outdir,f'croc_Iana_vs_VrefTrim.csv')
    if not os.path.isfile(outfile):
        with open(outfile, 'a+') as f:
            csv.writer(f).writerow(['VOLTAGE_TRIM', 'TRIM_VREFA', 'TRIM_VREFD', 'VddA', 'VddD', 'Iana', 'Idig'])

    tomlFile = getTomlFile(xmlConfig)
    tomlData = toml.load(tomlFile)

    with multimeter:
        multimeter.set('CONFIGURE', 'VOLT:DC')
        multimeter.set_all_line_integrations(1)
        with drb:
            with lv:
                for vA in range(16):
                    voltage_trim = (vA<<4) + (vtrimD)
                    tomlData['Registers']['VOLTAGE_TRIM'] = voltage_trim
                    with open(tomlFile, "w") as f:
                        toml.dump(tomlData, f)
                    configureCROC(xmlConfig)
                    drb.set_pin('ac')
                    vdda = multimeter.measure('VOLT:DC', cycles=1)[0]
                    drb.set_pin('ad')
                    vddd = multimeter.measure('VOLT:DC', cycles=1)[0]
                    iana = lv.measure(2)[1]
                    idig = lv.measure(1)[1]
                    with open(outfile, 'a+') as f:
                        csv.writer(f).writerow([voltage_trim, vA, vtrimD, vdda, vddd, iana, idig])
                for vD in range(16):
                    voltage_trim = (vtrimA<<4) + (vD)
                    tomlData['Registers']['VOLTAGE_TRIM'] = voltage_trim
                    with open(tomlFile, "w") as f:
                        toml.dump(tomlData, f)
                    configureCROC(xmlConfig)
                    drb.set_pin('ac')
                    vdda = multimeter.measure('VOLT:DC', cycles=1)[0]
                    drb.set_pin('ad')
                    vddd = multimeter.measure('VOLT:DC', cycles=1)[0]
                    iana = lv.measure(2)[1]
                    idig = lv.measure(1)[1]
                    with open(outfile, 'a+') as f:
                        csv.writer(f).writerow([voltage_trim, vtrimA, vD, vdda, vddd, iana, idig])
    voltage_trim = (vtrimA<<4) + (vtrimD)
    tomlData['Registers']['VOLTAGE_TRIM'] = voltage_trim
    with open(tomlFile, "w") as f:
        toml.dump(tomlData, f)
