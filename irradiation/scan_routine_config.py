config_preIrradiation = [
    #{
    #    "name": "VrefTrimming",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["VrefTrimming"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : [16]
    #        },
    #    ]
    #},
    #{
    #    'name': 'current_vs_PA_IN_BIAS_LIN',
    #    'type': 'curr_vs_DAC',
    #    'configFile': 'CROC.xml',
    #    'PSchannel': 2,
    #    'params': [
    #        {
    #            'table': 'Registers',
    #            'keys' : ['DAC_PREAMP_L_LIN', 'DAC_PREAMP_R_LIN', 'DAC_PREAMP_TL_LIN', 'DAC_PREAMP_TR_LIN', 'DAC_PREAMP_T_LIN', 'DAC_PREAMP_M_LIN'],
    #            'values' : [0, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000]
    #        },
    #    ]
    #},
    ##{
    ##    "name": "ThresholdTuning1000",
    ##    "type": "Ph2_ACF",
    ##    "configFile": "CROC.xml",
    ##    "updateConfig" : True,
    ##    "tools": ["ThresholdEqualization3000", "GlobalThresholdTuning1000", "ThresholdEqualization1000", "ThresholdScanSparse"],
    ##        {
    ##            "table" : "Registers",
    ##            "keys" : ["DAC_PREAMP_L_LIN", "DAC_PREAMP_R_LIN", "DAC_PREAMP_TL_LIN", "DAC_PREAMP_TR_LIN", "DAC_PREAMP_T_LIN", "DAC_PREAMP_M_LIN"],
    ##            "values" : [348]
    ##        },
    ##},
    ##{
    ##    "name": "ThresholdScanSparse",
    ##    "type": "Ph2_ACF",
    ##    "configFile": "CROC.xml",
    ##    "tools": ["ThresholdScanSparse"],
    ##    "updateConfig" : False,
    ##},
    ##{
    ##    "name": "ToT_vs_KRUM_CURR_LIN",
    ##    "type": "Ph2_ACF",
    ##    "configFile": "CROC.xml",
    ##    "updateConfig" : True,
    ##    "tools": ["AnalogScanSparse"],
    ##    'params': [
    ##        {
    ##            'table': 'Registers',
    ##            'keys' : ['VCAL_HIGH'],
    ##            'values' : [1391]
    ##        },
    ##        {
    ##            'table': 'Registers',
    ##            'keys' : ['DAC_KRUM_CURR_LIN'],
    ##            'values' : [0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 190, 195, 200]
    ##        },
    ##    ]
    ##},
    {
        "name": "GlobalThresholdTuning3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["GlobalThresholdTuning3000", "ThresholdScanSparse"],
        "updateConfig" : True,
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [16]
            }
        ]
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
        "tools": ["ThresholdEqualization3000", "GlobalThresholdTuning1000", "ThresholdEqualization1000", "GlobalThresholdTuning1000", "ThresholdScanLow"]
    },
    {
        "name": "StuckPixelScan1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan"]
    },
    {
        "name": "NoiseScan1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan", "NoiseScan"]
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
        "params": [
            {
                "table" : "Registers",
                "keys" : ["VCAL_HIGH"],
                "values" : [500, 1000, 1500]
            },
        ]
    },
    {
        "name": "TimeWalk1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["TimeWalk"]
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
        "tools": ["ThresholdScan"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : ["tdac_preIrradiation.csv"]
            }
        ],
    },
    {
        "name": "AFEScans",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["ThresholdEqualization1000", "ThresholdScan", "DigitalScan", "AnalogScan", "RingOsc", "ADCScan", "DACScan", "TimeWalk", "NoiseScan"],
    }
]
