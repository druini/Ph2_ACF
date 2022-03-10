from instrument_control import PowerSupplyController
import xml.etree.ElementTree as ET
import toml
import itertools
import subprocess
from datetime import datetime
import time
import csv 

config = [
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
    }
    # ,
    # {
    #     "name": "IVConfigured",
    #     "type": "IV",
    #     "configFile": "CROC2.xml",
    #     "startingCurrent" : 0.8,
    #     "finalCurrent" : 2.5,
    #     "currentStep" : 0.1
    # },
    # {
    #     "name": "IVDefault",
    #     "type": "IV",
    #     "startingCurrent" : 0.1,
    #     "finalCurrent" : 2.5,
    #     "currentStep" : 0.1 
    # }
]

powerSupplyResource = "/dev/ttyUSB0"
powerSupplyVoltage = 1.8
powerSupplyCurrent = 2

logFile = "log.csv"

fmt = "%Y %m %d-%H:%M:%S"

def getTomlFile(xmlConfig):
    tree = ET.parse(xmlConfig)
    root = tree.getroot()

    return next(root.iter("CROC")).attrib["configfile"]

def run_Ph2_ACF(task, paramsForLog=[]):
    for i in range(task["maxAttempts"]):
        p = subprocess.Popen(["RD53BminiDAQ", "-f", task["configFile"], "-t", "RD53BTools.toml", "-h", "-s", *task["tools"]])
        try:
            returncode = p.wait(timeout=task['timeout'])
        except:
            p.terminate()
            returncode = -1
        add_log_entry(task["name"], returncode, i, paramsForLog)
        if returncode == 0:
            break
        else:
            time.sleep(1)
            
    else:
        return False
    return True

def add_log_entry(name, returncode, attempt_id, paramsForLog):
    with open(logFile, 'a+') as f:
        write = csv.writer(f)
        write.writerow([datetime.now().strftime(fmt), name, returncode, attempt_id, *paramsForLog])


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
    if "configFile" in task:
        while True:
            p = subprocess.Popen(["RD53BminiDAQ", "-f", task["configFile"]])
            returncode = p.wait(timeout=5)
            if returncode == 0:
                break


def main():
    # powerSupply = PowerSupplyController(powerSupplyResource, 2)
    for task in config:
        # set power supply voltage/current
        # powerSupply.set_voltage(1, powerSupplyVoltage)
        # powerSupply.set_voltage(2, powerSupplyVoltage)
        # powerSupply.set_current(1, powerSupplyCurrent)
        # powerSupply.set_current(2, powerSupplyCurrent)

        if task["type"] == "Ph2_ACF":
            Ph2_ACF_Task(task)
            
        elif task["type"] == "IV":
            IV_Task(task)

main()