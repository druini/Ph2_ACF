import pyvisa as visa
import time, logging
import sys

TARGET_TEMP = 20
TIMEOUT     = 10000

logging.basicConfig(
        format='%(asctime)s [%(levelname)s] %(message)s',
        datefmt='%Y/%m/%d-%H:%M:%S',
        level=logging.INFO,
        handlers=[
            logging.FileHandler("peltier.log"),
            #logging.StreamHandler(sys.stdout)
            ]
        )

def acquire_arduino(timeout=TIMEOUT):
    rm = visa.ResourceManager()
    logging.info(f"Detected devices: {rm.list_resources()}")
    board = rm.open_resource('ASRL/dev/ttyUSB2::INSTR', baud_rate=9600)
    board.timeout = timeout
    return board

logging.info('Starting peltier control script')
uno = acquire_arduino()
#cnt = 0
settemp = -273
meastemp = -273
datastring_raw = ''

reestablish = False
cntr_largeTempDiff  = 0

while True:
    try:
        datastring_raw = uno.read_raw().decode().strip()
        logging.debug(f'Raw string: {datastring_raw}')
    except visa.VisaIOError:
        logging.warning('Cannot read from arduino, reestablishing communication...')
        reestablish = True
        
    if reestablish:
        for reconnectAttempt in range(3):
            try:
                uno.close()
                time.sleep(1)
                uno = acquire_arduino()
                reestablish = False
                logging.info(f'Communication to arduino reestablished on attempt nr. {reconnectAttempt+1}.')
                break
            except:
                logging.warning('Cannot reestablish communication, retrying...')
        else:
            logging.error('Could not reestablish communication to arduino, exiting (code -1)')
            sys.exit(1)
        continue

    datastring = datastring_raw.split(' ')
    try:
        datastring = [float(t) for t in datastring]
        assert( len(datastring)==4 )
        logging.debug(f'datastring: {datastring}')
    except:
        logging.info(datastring_raw)
        continue

    settemp = datastring[-2]
    meastemp = datastring[-1]
    logging.info(f'setTemp {settemp}, measuredTemp {meastemp}') 
    #if the temperature is wrong, set it to the correct one
    #if cnt==2:
    if abs(settemp-TARGET_TEMP)>0.01:
        logging.warning(f'Temperature was set to {settemp}!! Resetting it to {TARGET_TEMP}.')
        uno.write( str(TARGET_TEMP) )
    if abs( meastemp-settemp ) > 5:
        logging.error(f'Large temperature difference: set {settemp}, measured {meastemp}!!')
        cntr_largeTempDiff += 1
    else:
        cntr_largeTempDiff = 0
    if cntr_largeTempDiff>4:
        logging.error('Read large temperature difference for 5 times in a row, exiting (code -2)')
        sys.exit(2)
    time.sleep(0.05)
