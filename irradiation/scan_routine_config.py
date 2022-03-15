config_preIrradiation = [
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
        "tools": ["AnalogScan", "DigitalScan", "RingOsc", "ADCScan", "DACScan", "GlobalThresholdTuning"],
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
        "tools": ["ThresholdEqualization", "GlobalThresholdTuning", "ThresholdEqualization", "ThresholdScan", "DigitalScan", "AnalogScan", "TimeWalk", "Noise"],
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

config_irradiation = [
    {
        "name": "ThresholdScan_existingTDACs",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "timeout": 600,
        "maxAttempts": 3,
        "tools": ["ThresholdScan"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : ["tdac.csv", "tdac_preIrradiation.csv"]
            }
        ],
    },
    {
        "name": "AFEScans",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "timeout": 600,
        "maxAttempts": 3,
        "tools": ["ThresholdEqualization", "ThresholdScan", "DigitalScan", "AnalogScan", "RingOsc", "ADCScan", "DACScan", "TimeWalk", "Noise"],
    }
]
