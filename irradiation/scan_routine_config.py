config_preIrradiation = [
    #{
    #    'name': 'current_vs_PA_IN_BIAS_LIN',
    #    'type': 'curr_vs_DAC',
    #    'configFile': 'CROC.xml',
    #    'timeout': 300,
    #    'maxAttempts': 3,
    #    'PSchannel': 2,
    #    'params': [
    #        {
    #            'table': 'Registers',
    #            'keys' : ['DAC_PREAMP_L_LIN', 'DAC_PREAMP_R_LIN', 'DAC_PREAMP_TL_LIN', 'DAC_PREAMP_TR_LIN', 'DAC_PREAMP_T_LIN', 'DAC_PREAMP_M_LIN'],
    #            'values' : [0, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000]
    #        },
    #    ]
    #},
    # {
    #     "name": "VrefTrimming",
    #     "type": "Ph2_ACF",
    #     "configFile": "CROC.xml",
    #     "timeout" : 600,
    #     "maxAttempts" : 3,
    #     "updateConfig" : True,
    #     "tools": ["VrefTrimming"]
    # },
    # {
    #     "name": "BasicScansUntuned",
    #     "type": "Ph2_ACF",
    #     "configFile": "CROC.xml",
    #     "timeout" : 600,
    #     "maxAttempts" : 3,
    #     "updateConfig" : False,
    #     "tools": ["AnalogScanFast", "DigitalScan", "RingOsc", "ADCScan", "DACScan"]
    # },
    {
        "name": "GlobalThresholdTuning3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "timeout" : 600,
        "maxAttempts" : 3,
        "tools": ["GlobalThresholdTuning3000", "ThresholdScanSparse"],
        "updateConfig" : True
    },
    {
        "name": "ThresholdScan3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "timeout" : 600,
        "maxAttempts" : 3,
        "tools": ["ThresholdScanSparse"],
        "updateConfig" : False,
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
        "updateConfig" : True,
        "tools": ["ThresholdEqualization3000", "GlobalThresholdTuning1000", "ThresholdEqualization1000", "ThresholdScanLow"],
        "timeout" : 600,
        "maxAttempts" : 3
    },
    {
        "name": "MaskStuck1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan"],
        "timeout" : 600,
        "maxAttempts" : 3
    },
    {
        "name": "MaskNoisy1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"],
        "timeout" : 600,
        "maxAttempts" : 3
    },
    {
        "name": "GainTuning1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GainTuning"],
        "timeout" : 600,
        "maxAttempts" : 3
    },
    {
        "name": "NoiseScan1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["NoiseScan"],
        "timeout" : 600,
        "maxAttempts" : 3
    },
    {
        "name": "AnalogScan1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["AnalogScan"],
        "timeout" : 600,
        "maxAttempts" : 3
    },
    {
        "name": "TimeWalk1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["TimeWalk"],
        "timeout" : 600,
        "maxAttempts" : 3
    }
    #{
    #    "name": "IVConfigured",
    #    "type": "IV",
    #    "configFile": "CROC.xml",
    #    "startingCurrent" : 2.5,
    #    "finalCurrent" : .5,
    #    "currentStep" : 0.1
    #},
    #{
    #    "name": "IVDefault",
    #    "type": "IV",
    #    "startingCurrent" : 0.1,
    #    "finalCurrent" : 2.5,
    #    "currentStep" : 0.1
    #}
]

config_irradiationBase = [
    {
        "name": "ShortRingOsc_Mux",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "timeout": 600,
        "maxAttempts": 3,
        "tools": ["ShortRingOsc", "MuxScan"],
    },
    {
        "name": "vmonitor",
        "type": "Vmonitor",
    }
        ]

config_irradiationMain = [
    {
        "name": "VrefTrimming",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "timeout": 600,
        "maxAttempts": 3,
        "tools": ["VrefTrimming"],
    },
    {
        "name": "IVConfigured",
        "type": "IV",
        "configFile": "CROC.xml",
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
    },
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
        "tools": ["ThresholdEqualization1000", "ThresholdScan", "DigitalScan", "AnalogScan", "RingOsc", "ADCScan", "DACScan", "TimeWalk", "NoiseScan"],
    }
]
