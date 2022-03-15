from instrument_control import PowerSupplyController
from vi_scan import vi_curves, vmonitor
import xml.etree.ElementTree as ET
import toml
import itertools
import subprocess
from datetime import datetime
import time
import csv
import sys
import scan_routine_config

powerSupplyResource = "/dev/ttyUSB0"
powerSupplyVoltage = 1.8
powerSupplyCurrent = 2

logFile = "log.csv"

fmt = "%Y %m %d-%H:%M:%S"

def add_log_entry(row):
    with open(logFile, 'a+') as f:
        write = csv.writer(f)
        write.writerow([datetime.now().strftime(fmt), *row])

def getTomlFile(xmlConfig):
    tree = ET.parse(xmlConfig)
    root = tree.getroot()
    return next(root.iter("CROC")).attrib["configfile"]

def configureCROC(configFile):
    while True:
        p = subprocess.Popen(["RD53BminiDAQ", "-f", configFile])
        returncode = p.wait(timeout=5)
        if returncode == 0:
            return True

def run_Ph2_ACF(task, paramsForLog=[]):
    dir_name =  task["name"] + "_" + datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
    for i in range(task["maxAttempts"]):
        if i>1:
            PowerSupplyController.power_cycle()
            time.sleep(.5)
        p = subprocess.Popen(["RD53BminiDAQ", "-f", task["configFile"], "-t", "RD53BTools.toml", "-h", "-s", "-o", dir_name, *task["tools"]])
        try:
            returncode = p.wait(timeout=task['timeout'])
        except:
            p.terminate()
            returncode = -1
        add_log_entry([task["name"], returncode, i, dir_name, *paramsForLog])
        if returncode == 0:
            break
        else:
            time.sleep(1)
    else:
        return False
    return True

def Ph2_ACF_Task(task):
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
            run_Ph2_ACF(task, paramsForLog)
    else:
        run_Ph2_ACF(task)

def IV_Task(task):
    dir_name =  task["name"] + "_" + datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
    if "configFile" in task:
        configureCROC(task['configFile'])
    vi_curves(dir_name, task['startingCurrent'], task['finalCurrent'], task['currentStep'])

def Vmonitor_Task(task):
    dir_name = '.'
    vmonitor()

def curr_vs_DAC_Task(task):
    dir_name =  task["name"] + "_" + datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)
    outfile = os.path.join(dir_name,f'croc_vi_curves_{time.strftime("%Y%m%d-%H%M%S")}.csv')
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
        configureCROC(task['configFile'])
        data = list(values)
        data.append(powerSupply.read_current(task['PSchannel']))
        with open(outfile, 'a+') as f:
            csv.writer(f).writerow(data)

def main(config):
    for task in config:
        time.sleep(.5)

        if task["type"] == "Ph2_ACF":
            Ph2_ACF_Task(task)

        elif task["type"] == "IV":
            IV_Task(task)

        elif task["type"] == "Vmonitor":
            Vmonitor_Task(task)

        elif task['type'] == 'curr_vs_DAC':
            curr_vs_DAC_Task(task)

if __name__=='__main__':
    powerSupply = PowerSupplyController(powerSupplyResource, 2)
    powerSupply.power_off('ALL')
    # set power supply voltage/current
    powerSupply.set_voltage(1, powerSupplyVoltage)
    powerSupply.set_voltage(2, powerSupplyVoltage)
    powerSupply.set_current(1, powerSupplyCurrent)
    powerSupply.set_current(2, powerSupplyCurrent)
    powerSupply.power_on('ALL')

    if sys.argv[1]=='preIrrad':
        config = scan_routine_config.config_preIrradiation
        main(config)
    elif sys.argv[1]=='irrad':
        config = scan_routine_config.config_irradiation
        while True:
            main(config)
    else:
        sys.exit(f'Unknown config: {sys.argv[1]}. Allowed are "preIrrad" and "irrad"')
