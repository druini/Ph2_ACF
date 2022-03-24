import xml.etree.ElementTree as ET
import toml
import itertools
import subprocess
from datetime import datetime, timedelta
import time
import csv
import sys,os
import argparse
import scan_routine_config
from pdb import set_trace

powerSupplyResource = "ASRL/dev/ttyACM0::INSTR"
powerSupplyVoltage = 1.8
powerSupplyCurrent = 2

logFile = "log.csv"
baseDir = 'Results'

fmt = "%Y %m %d-%H:%M:%S"

def add_log_entry(row):
    with open(logFile, 'a+') as f:
        write = csv.writer(f)
        write.writerow([datetime.now().strftime(fmt), *row])

def getTomlFile(xmlConfig):
    tree = ET.parse(xmlConfig)
    root = tree.getroot()
    return next(root.iter("CROC")).attrib["configfile"]

def configureCROC(configFile, powerSupply=None):
    for attempt in range(5):
        if attempt>2 and powerSupply is not None:
            powerSupply.power_cycle()
        p = subprocess.Popen(["RD53BminiDAQ", "-f", configFile, "-t", "RD53BTools.toml"])
        returncode = p.wait(timeout=5)
        if returncode == 0:
            return True
    return False

def run_Ph2_ACF(task, tool, paramsForLog=[], powerSupply=None, dir_name='Results'):
    for i in range(task["maxAttempts"]):
        if i>1 and powerSupply is not None:
            powerSupply.power_cycle()
            time.sleep(.5)
        extra_flags = ["-s"] if task["updateConfig"] else []
        p = subprocess.Popen(["RD53BminiDAQ", "-f", task["configFile"], "-t", "RD53BTools.toml", "-h", "-o", dir_name, *extra_flags, tool])
        try:
            returncode = p.wait(timeout=task['timeout'])
        except:
            p.terminate()
            returncode = -1
        add_log_entry([task["name"], tool, returncode, i, dir_name, *paramsForLog])
        if returncode == 0:
            break
        else:
            time.sleep(1)
    else:
        add_log_entry([task["name"], tool, 'failed'])
    return True

def Ph2_ACF_Task(task, powerSupply):
    dir_name =  os.path.join(
            baseDir,
            task["name"] + "_" + datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
            )
    for tool in task['tools']:
        if "params" in task:
            tomlFile = getTomlFile(task['configFile'])
            tomlData = toml.load(tomlFile)
            params = task['params']
            # for p in range(len(task['params'])):
            for values in itertools.product(*[p["values"] for p in params]):
                paramsForLog = []
                for i in range(len(values)):
                    for key in params[i]["keys"]:
                        paramsForLog += [f'{key}:{values[i]}']
                        tomlData[params[i]["table"]][key] = values[i]
                with open(tomlFile, "w") as f:
                    toml.dump(tomlData, f)
                run_Ph2_ACF(task, tool, paramsForLog, powerSupply, dir_name)
        else:
            run_Ph2_ACF(task, tool=tool, paramsForLog=[], powerSupply=powerSupply, dir_name=dir_name)
    return True

def IV_Task(task, powerSupply):
    dir_name = os.path.join(
            baseDir,
            task["name"] + "_" + datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
            )
    if "configFile" in task:
        configured = configureCROC(task['configFile'], powerSupply=powerSupply)
        if not configured:
            return False
    return vi_curves(dir_name, task['startingCurrent'], task['finalCurrent'], task['currentStep'])

def Vmonitor_Task(task):
    dir_name = baseDir
    return vmonitor(dir_name)

def curr_vs_DAC_Task(task, powerSupply):
    dir_name = os.path.join(
            baseDir,
            task["name"] + "_" + datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
            )
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)
    outfile = os.path.join(dir_name,f'croc_{task["name"]}_{time.strftime("%Y%m%d-%H%M%S")}.csv')
    if not os.path.isfile(outfile):
        with open(outfile, 'a+') as f:
            csv.writer(f).writerow(['PA_IN_BIAS_LIN', 'Iana'])
    tomlFile = getTomlFile(task['configFile'])
    tomlData = toml.load(tomlFile)
    params = task['params']
    for values in itertools.product(*[p["values"] for p in params]):
        for i in range(len(values)):
            for key in params[i]["keys"]:
                tomlData[params[i]["table"]][key] = values[i]
        with open(tomlFile, "w") as f:
            toml.dump(tomlData, f)
        configured = configureCROC(task['configFile'], powerSupply)
        if not configured:
            return False
        time.sleep(.5)
        data = list(values)
        data.append(powerSupply.read_current(task['PSchannel']))
        with open(outfile, 'a+') as f:
            csv.writer(f).writerow(data)
    return True

def main(config, tempControl, powerSupply):
    psOFF = False
    wrongTcounter = 0
    for task in config:
        if tempControl is not None:
            while True:
                tempState = tempControl.poll()
                if tempState == -1: #lost communication to arduino
                    time.sleep(1)
                    tempControl = subprocess.Popen(['python', os.path.join(os.environ['PH2ACF_BASE_DIR'],'irradiation','peltier_com.py')])
                    time.sleep(3)
                    continue
                elif tempState == -2: #temperature is more than 5degs from target temperature
                    powerSupply.power_off('ALL')
                    wrongTcounter += 1
                    if wrongTcounter > 2: sys.exit('Lost control over temperature')
                    psOFF = True
                    #tempControl = subprocess.Popen(['python', 'peltier_com.py'])
                    tempControl = subprocess.Popen(['python', os.path.join(os.environ['PH2ACF_BASE_DIR'],'irradiation','peltier_com.py')])
                    time.sleep(60)
                    continue
                elif tempState is None:
                    wrongTcounter = 0
                    if psOFF:
                        powerSupply.power_on('ALL')
                        psOFF = False
                    break
            time.sleep(.5)

        add_log_entry([task['name'], 'starting'])
        if task["type"] == "Ph2_ACF":
            ret = Ph2_ACF_Task(task, powerSupply)

        elif task["type"] == "IV":
            ret = IV_Task(task, powerSupply)

        elif task["type"] == "Vmonitor":
            ret = Vmonitor_Task(task)

        elif task['type'] == 'curr_vs_DAC':
            if powerSupply is None:
                continue
            ret = curr_vs_DAC_Task(task, powerSupply)
        add_log_entry([task['name'], 'completed'])

if __name__=='__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--no-instruments', action='store_true', help='Flag to not use the instrument control library')
    parser.add_argument('-c', '--config', action='store', choices=['preIrrad','irrad'], help='Selects the scan sequence in scan_routine_config.py')
    args = parser.parse_args()

    if args.no_instruments:
        tempControl = None
        powerSupply = None
    else:
        from instrument_control import PowerSupplyController, XrayController
        from vi_scan import vi_curves, vmonitor
        tempControl = subprocess.Popen(['python', os.path.join(os.environ['PH2ACF_BASE_DIR'],'irradiation','peltier_com.py')])
        powerSupply = PowerSupplyController(powerSupplyResource, 2)

    if powerSupply is not None:
        powerSupply.power_off('ALL')
        # set power supply voltage/current
        powerSupply.set_voltage(1, powerSupplyVoltage)
        powerSupply.set_voltage(2, powerSupplyVoltage)
        powerSupply.set_current(1, powerSupplyCurrent)
        powerSupply.set_current(2, powerSupplyCurrent)
        powerSupply.power_on('ALL')

    if args.config=='preIrrad':
        config = scan_routine_config.config_preIrradiation
        main(config, tempControl, powerSupply)
    elif args.config=='irrad':
        configBase = scan_routine_config.config_irradiationBase
        configMain = scan_routine_config.config_irradiationMain
        lastMainScan = datetime.fromisocalendar(1900,1,1)
        mainScanRepetitions = 0

        if args.no_instruments:
            xray = None
        else:
            xray = XrayController(resource='ASRL/dev/ttyID3003::INSTR', logfile='xray.log')
        if xray is not None:
            xray.set_current(30)
            xray.set_voltage(60)
            xray.on()
            xray.open_shutter()

        while True:
            if xray is not None:
                for i in range(3):
                    if xray.verify_parameters():
                        break
                    else:
                        xray.off()
                        xray.on()
                        time.sleep(3)
                        xray.open_shutter()
                else:
                    sys.exit('Xrays are broken :(')
            main(configBase, tempControl, powerSupply)
            if mainScanRepetitions < 10:
                deltaHours = 1
            elif mainScanRepetitions < 100:
                deltaHours = 10
            else:
                deltaHours = 50
            if datetime.now() - lastMainScan > timedelta(hours=deltaHours):
                if xray is not None: xray.off()
                main(configMain, tempControl, powerSupply)
                mainScanRepetitions += 1
                lastMainScan = datetime.now()
                if xray is not None:
                    xray.on()
                    xray.open_shutter()
    if powerSupply is not None:
        powerSupply.power_off('ALL')
