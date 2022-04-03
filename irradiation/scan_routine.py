import xml.etree.ElementTree as ET
import toml, itertools, subprocess, signal
import time, csv, sys,os, argparse
from datetime import datetime, timedelta
from tailer import tail
import scan_routine_config
import telegramBot as telegram
import NTC_com as NTC
from pdb import set_trace

powerSupplyResource = "ASRL/dev/ttyACM0::INSTR"
powerSupplyVoltage = 2.5
powerSupplyCurrent = 1.2

baseDir = 'Results' + "_" + datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
logFile = os.path.join(baseDir, "log.csv")
timeout = 600
maxAttempts = 3

fmt = "%Y %m %d-%H:%M:%S"

if not os.path.exists(baseDir):
    os.makedirs(baseDir)

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

def checkReturncode(code, powerSupply, xray):
    if code<0: # Ph2_ACF task cannot complete
        telegram.send_text('Cannot complete Ph2_ACF task')
        add_log_entry(['gave up on this task'])
    elif code==1: # peltier controller unreacheable
        ntcLog = tail(NTC.LOGFILE, 1)[-1].split(' ')
        ntcTime, ntcTemp = ntcLog[0], ntcLog[-1]
        if datetime.now() - datetime.strptime(ntcTime, NTC.DATEFMT) > timedelta(minutes=1):
            telegram.send_text(f'Peltier controller unreacheable and NTC lost since {ntcTime}. I have no clue about the temperature and will shut down...')
            add_log_entry(['lost peltier and ntc info: shutting down'])
            powerSupply.power_off()
            xray.off()
            sys.exit()
        else:
            if ntcTemp > -5: #FIXME check actual temperature
                telegram.send_text(f'Peltier controller unreacheable and NTC at {ntcTemp}! Shutting down.')
                add_log_entry(['lost peltier and ntc is warm: shutting down'])
                sys.exit()
            else:
                telegram.send_text(f'Peltier controller unreacheable but NTC still cold ({ntcTemp})...please check')
    elif code==2: # temperature away from target
        telegram.send_text('Lost control of peltier temperature! Shutting everything off.')
        add_log_entry(['lost control of temperature'])
        powerSupply.power_off()
        xray.off()
        sys.exit()

def run_Ph2_ACF(task, tool, paramsForLog=[], powerSupply=None, dir_name='Results'):
    for i in range(maxAttempts):
        if i>1 and powerSupply is not None:
            powerSupply.power_cycle()
            time.sleep(.5)
        extra_flags = ["-s"] if task["updateConfig"] else []
        p = subprocess.Popen(["RD53BminiDAQ", "-f", task["configFile"], "-t", "RD53BTools.toml", "-h", "-o", dir_name, *extra_flags, tool])
        try:
            returncode = p.wait(timeout=timeout)
        except:
            p.terminate()
            returncode = -1 # cannot complete task
        add_log_entry([task["name"], tool, returncode, i, dir_name, *paramsForLog])
        if returncode == 0:
            break
        else:
            time.sleep(1)
    else:
        add_log_entry([task["name"], tool, 'failed'])
    return returncode

def Ph2_ACF_Task(task, powerSupply):
    dir_name =  os.path.join(
            baseDir,
            task["name"] + "_" + datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
            )
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)
    for tool in task['tools']:
        if "params" in task:
            if powerSupply is not None:
                currConsumption = os.path.join(dir_name, 'currentConsumption.txt')
                if not os.path.isfile(currConsumption):
                    with open(currConsumption, 'w') as f:
                        f.write('params, Iana, Idig\n')
            tomlFile = getTomlFile(task['configFile'])
            params = task['params']

            # store original parameter values if needed
            if not task['updateConfig']:
                tomlData = toml.load(tomlFile)
                original_values = []
                for p in params:
                    original_values.append((p["table"], {key : tomlData[p["table"]].get(key, None) for key in p["keys"]}))

            for values in itertools.product(*[p["values"] for p in params]):
                paramsForLog = []
                tomlData = toml.load(tomlFile)
                for i in range(len(values)):
                    for key in params[i]["keys"]:
                        paramsForLog += [f'{key}:{values[i]}']
                        tomlData[params[i]["table"]][key] = values[i]
                with open(tomlFile, "w") as f:
                    toml.dump(tomlData, f)
                ret = run_Ph2_ACF(task, tool, paramsForLog, powerSupply, dir_name)
                if powerSupply is not None:
                    idig = powerSupply.read_current(1)
                    iana = powerSupply.read_current(2)
                    with open(currConsumption, 'a+') as f:
                        f.write(f'{"_".join(paramsForLog)},{iana},{idig}\n')

            # restore original parameter values
            if not task['updateConfig']:
                tomlData = toml.load(tomlFile)
                for table, data in original_values:
                    for key, value in data.items():
                        if value is not None:
                            tomlData[table][key] = value
                with open(tomlFile, "w") as f:
                    toml.dump(tomlData, f)

        else:
            ret = run_Ph2_ACF(task, tool=tool, paramsForLog=[], powerSupply=powerSupply, dir_name=dir_name)
    return ret

def IV_Task(task, powerSupply):
    dir_name = os.path.join(
            baseDir,
            task["name"]
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
    return 0

def launchScanRoutine(config, peltierControl, ntcControl, powerSupply):
    psOFF = False
    wrongTcounter = 0
    peltierUnreacheableCounter = 0
    for task in config:
        print(f'Starting task {task["name"]} of type {task["type"]}\n')
        if peltierControl is not None:
            while True:
                peltierState = peltierControl.poll()
                if peltierState == 1: #lost communication to arduino
                    if peltierUnreacheableCounter>2:
                        return 1
                    time.sleep(1)
                    peltierControl = subprocess.Popen(['python', os.path.join(os.environ['PH2ACF_BASE_DIR'],'irradiation','peltier_com.py')])
                    time.sleep(3)
                    peltierUnreacheableCounter += 1
                    continue
                elif peltierState == 2: #temperature is more than 5degs from target temperature
                    powerSupply.power_off('ALL')
                    if wrongTcounter > 2:
                        return 2
                    psOFF = True
                    #tempControl = subprocess.Popen(['python', 'peltier_com.py'])
                    peltierControl = subprocess.Popen(['python', os.path.join(os.environ['PH2ACF_BASE_DIR'],'irradiation','peltier_com.py')])
                    time.sleep(60)
                    wrongTcounter += 1
                    continue
                elif peltierState is None:
                    wrongTcounter = 0
                    peltierUnreacheableCounter = 0
                    if psOFF:
                        powerSupply.power_on('ALL')
                        psOFF = False
                    break
            time.sleep(.5)

        add_log_entry([task['name'], 'starting'])
        if task["type"] == "Ph2_ACF":
            ret = Ph2_ACF_Task(task, powerSupply)

        elif task["type"] == "IV":
            if ntcControl is not None: ntcControl.send_signal(signal.SIGSTOP)
            ret = IV_Task(task, powerSupply)
            powerSupply.power_cycle()
            if ntcControl is not None: ntcControl.send_signal(signal.SIGCONT)

        elif task["type"] == "Vmonitor":
            if ntcControl is not None: ntcControl.send_signal(signal.SIGSTOP)
            ret = Vmonitor_Task(task)
            if ntcControl is not None: ntcControl.send_signal(signal.SIGCONT)

        elif task['type'] == 'curr_vs_DAC':
            if powerSupply is None:
                continue
            ret = curr_vs_DAC_Task(task, powerSupply)
        add_log_entry([task['name'], 'completed'])
        checkReturncode( ret, None, None )

if __name__=='__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--no-instruments', action='store_true', help='Flag to not use the instrument control library')
    parser.add_argument('-c', '--config', action='store', choices=['preIrrad','irrad'], help='Selects the scan sequence in scan_routine_config.py')
    args = parser.parse_args()

    if args.no_instruments:
        peltierControl = None
        ntcControl     = None
        powerSupply    = None
    else:
        from instrument_control import PowerSupplyController, XrayController
        from vi_scan import vi_curves, vmonitor
        peltierControl = subprocess.Popen(['python', os.path.join(os.environ['PH2ACF_BASE_DIR'],'irradiation','peltier_com.py')])
        ntcControl     = subprocess.Popen(['python', os.path.join(os.environ['PH2ACF_BASE_DIR'],'irradiation','NTC_com.py')])
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
        launchScanRoutine(config, peltierControl, ntcControl, powerSupply)
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
            if ntcControl is not None:
                if ntcControl.poll() is not None:
                    ntcControl.terminate()
                    ntcControl = subprocess.Popen(['python', os.path.join(os.environ['PH2ACF_BASE_DIR'],'irradiation','NTC_com.py')])
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
            scanRoutineReturn = launchScanRoutine(configBase, peltierControl, ntcControl, powerSupply)
            checkReturncode( scanRoutineReturn, powerSupply, xray )
            if mainScanRepetitions < 10:
                deltaHours = 1
            elif mainScanRepetitions < 100:
                deltaHours = 10
            else:
                deltaHours = 50
            if datetime.now() - lastMainScan > timedelta(hours=deltaHours):
                if xray is not None: xray.off()
                scanRoutineReturn = launchScanRoutine(configMain, peltierControl, ntcControl, powerSupply)
                checkReturncode( scanRoutineReturn, powerSupply, xray )
                mainScanRepetitions += 1
                lastMainScan = datetime.now()
                if xray is not None:
                    xray.on()
                    xray.open_shutter()
    if peltierControl is not None:
        peltierControl.terminate()
    if ntcControl is not None:
        ntcControl.terminate()
    if powerSupply is not None:
        powerSupply.power_off('ALL')
