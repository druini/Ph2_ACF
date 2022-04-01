config_preIrradiation = [
    {
        "name": "VrefTrimming",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["VrefTrimming"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [16]
            },
        ]
    },
    {
        "name": "vmonitor",
        "type": "Vmonitor",
    },
    ##{
    ##    'name': 'current_vs_PA_IN_BIAS_LIN',
    ##    'type': 'curr_vs_DAC',
    ##    'configFile': 'CROC.xml',
    ##    'PSchannel': 2,
    ##    'params': [
    ##        {
    ##            'table': 'Registers',
    ##            'keys' : ['DAC_PREAMP_L_LIN', 'DAC_PREAMP_R_LIN', 'DAC_PREAMP_TL_LIN', 'DAC_PREAMP_TR_LIN', 'DAC_PREAMP_T_LIN', 'DAC_PREAMP_M_LIN'],
    ##            'values' : [0, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000]
    ##        },
    ##    ]
    ##},
    {
        "name": "MaskStuckUntuned",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : [1]
            },
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [16]
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
                "values" : [470]
            },
        ]
    },
    {
        "name": "MaskNoisyUntuned",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "GlobalThresholdTuning3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["GlobalThresholdTuning3000"],
        "updateConfig" : True,
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [16]
            },
        ]
    },
    {
        "name": "MaskStuck3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : [1]
            },
        ]
    },
    {
        "name": "MaskNoisy3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "ThresholdScan3000Single",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["ThresholdScanSparse"],
        "updateConfig" : False,
    },
    {
        "name": "BasicScans3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["AnalogScan", "DigitalScan", "RingOsc", "ADCScan", "DACScan"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [16]
            },
        ]
    },
    {
        "name": "ThresholdScan3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
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
        "name": "ThresholdTuning1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdEqualization3000", "GlobalThresholdTuning1000", "ThresholdEqualization1000", "GlobalThresholdTuning1000", "ThresholdScanLow"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : [1]
            },
        ]
    },
    {
        "name": "GainTuning1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GainTuning"],
    },
    {
        "name": "ToTMeasurement1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["AnalogScan"],
        "params": [
            {
                "table" : "Registers",
                "keys" : ["VCAL_HIGH"],
                "values" : [1391]
            },
        ]
    },
    {
        "name": "MaskStuck1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan"]
    },
    {
        "name": "MaskNoisy1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "NoiseScan1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["NoiseScan"],
    },
    {
        "name": "DigitalScan1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["DigitalScan"]
    },
    {
        "name": "AnalogScan1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["AnalogScan"],
    },
    {
        "name": "TimeWalk1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["TimeWalk"],
    },
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
        "updateConfig" : False,
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
        "updateConfig" : True,
        "tools": ["VrefTrimming"],
    },
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
    #},
    {
        "name": "ThresholdScan_previousTDACs",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["ThresholdScanLow"]
    },
    {
        "name": "ThresholdScan_preIrradTDACs",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["ThresholdScanLow"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : ["tdac_preIrradiation.csv"]
            }
        ],
    },
    {
        "name": "ThresholdScan_newTuning",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdEqualization1000", "GlobalThresholdTuning1000", "ThresholdScanLow"]
    },
    {
        "name": "GainTuning1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GainTuning"],
    },
    {
        "name": "ToTMeasurement1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["AnalogScan"],
        "params": [
            {
                "table" : "Registers",
                "keys" : ["VCAL_HIGH"],
                "values" : [1391]
            },
        ]
    },
    {
        "name": "BasicScans1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["AnalogScan", "DigitalScan", "RingOsc", "ADCScan", "DACScan"],
    },
    {
        "name": "NoiseScan1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["NoiseScan"],
    },
    {
        "name": "TimeWalk1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["TimeWalk"],
    }
]
