from instrument_control import PowerSupplyController
from vi_scan import vi_curves
import xml.etree.ElementTree as ET
import toml
import itertools
import subprocess
from datetime import datetime
import time
import csv 

config = [
    {
        'name': 'current_vs_PA_IN_BIAS_LIN',
        'type': 'curr_vs_DAC',
        'configFile': 'CROC.xml',
        'timeout': 300,
        'maxAttempts': 3,
        'PSchannel': 2,
        'params': [
            {
                'table': 'Registers',
                'keys' : ['DAC_PREAMP_L_LIN', 'DAC_PREAMP_R_LIN', 'DAC_PREAMP_TL_LIN', 'DAC_PREAMP_TR_LIN', 'DAC_PREAMP_T_LIN', 'DAC_PREAMP_M_LIN'],
                'values' : [0, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000]
            },
        ]
    },
    {
        "name": "GlobalThresholdTuning3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "timeout" : 600,
        "maxAttempts" : 3,
        "tools": ["GlobalThresholdTuning"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [16]
            }
        ]
    },
    {
        "name": "ThresholdScan3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "timeout" : 600,
        "maxAttempts" : 3,
        "tools": ["ThresholdScan"],
        "params": [
            {
                "table" : "Registers", 
                "keys" : ["DAC_PREAMP_L_LIN", "DAC_PREAMP_R_LIN", "DAC_PREAMP_TL_LIN", "DAC_PREAMP_TR_LIN", "DAC_PREAMP_T_LIN", "DAC_PREAMP_M_LIN"],
                "values" : [100, 250, 200, 300, 400]
            },
            {
                "table" : "Registers", 
                "keys" : ["DAC_LDAC_LIN"], 
                "values" : [130, 140, 150, 170, 190]
            },
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [0, 16, 31]
            }
        ]
    },
    {
        "name": "AFEScans1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["ThresholdEqualization", "GlobalThresholdTuning", "ThresholdEqualization", "ThresholdScan", "AnalogScan", "TimeWalk", "Noise"],
        "timeout" : 600,
        "maxAttempts" : 3
    },
    {
        "name": "IVConfigured",
        "type": "IV",
        "configFile": "CROC2.xml",
        "startingCurrent" : 2.5,
        "finalCurrent" : .5,
        "currentStep" : 0.1
    },
    {
        "name": "IVDefault",
        "type": "IV",
        "startingCurrent" : 0.1,
        "finalCurrent" : 2.5,
        "currentStep" : 0.1 
    }
]

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
        data.append(powerSupply.read_voltage(task['PSchannel']))
        with open(outfile, 'a+') as f:
            csv.writer(f).writerow(data)

def main():
    powerSupply = PowerSupplyController(powerSupplyResource, 2)
    powerSupply.power_off('ALL')
    # set power supply voltage/current
    powerSupply.set_voltage(1, powerSupplyVoltage)
    powerSupply.set_voltage(2, powerSupplyVoltage)
    powerSupply.set_current(1, powerSupplyCurrent)
    powerSupply.set_current(2, powerSupplyCurrent)
    powerSupply.power_on('ALL')

    for task in config:
        time.sleep(.5)

        if task["type"] == "Ph2_ACF":
            Ph2_ACF_Task(task)
            
        elif task["type"] == "IV":
            IV_Task(task)

        elif task['type'] == 'curr_vs_DAC':
            curr_vs_DAC_Task(task)

if __name__=='__main__':
    main()
