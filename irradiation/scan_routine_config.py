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
    #            "keys" : ["enable"],
    #            "values" : [1]
    #        },
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : [16]
    #        },
    #        {
    #            "table" : "Registers",
    #            "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
    #            "values" : [470]
    #        },
    #    ]
    #},
    #{
    #    "name": "vmonitor",
    #    "type": "Vmonitor",
    #},
    #{
    #    "name": "IVConfigured",
    #    "type": "IV",
    #    "configFile": "CROC.xml",
    #    "startingCurrent" : 1.,
    #    "finalCurrent" : 2.5,
    #    "currentStep" : 0.1
    #},
    #{
    #    "name": "IVDefault",
    #    "type": "IV",
    #    "startingCurrent" : 0.1,
    #    "finalCurrent" : 2.5,
    #    "currentStep" : 0.1
    #},
    ##{
    ##    'name': 'current_vs_PA_IN_BIAS_LIN',
    ##    'type': 'curr_vs_DAC',
    ##    'configFile': 'CROC.xml',
    ##    'PSchannel': 2,
    ##    'params': [
    ##        {
    ##            'table': 'Registers',
    ##            'keys' : ['DAC_PREAMP_L_LIN', 'DAC_PREAMP_R_LIN', 'DAC_PREAMP_TL_LIN', 'DAC_PREAMP_TR_LIN', 'DAC_PREAMP_T_LIN', 'DAC_PREAMP_M_LIN'],
    ##            'values' : [0, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800]
    ##        },
    ##    ]
    ##},
    #{
    #    "name": "MaskStuckUntuned",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["StuckPixelScan"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : [1]
    #        },
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : [16]
    #        },
    #        {
    #            "table" : "Registers",
    #            "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
    #            "values" : [470]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskNoisyUntuned",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["NoiseScan"]
    #},
    #{
    #    "name": "GlobalThresholdTuning3000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "tools": ["GlobalThresholdTuning3000"],
    #    "updateConfig" : True,
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : [16]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskStuck3000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["StuckPixelScan"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : [1]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskNoisy3000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["NoiseScan"]
    #},
    {
        "name": "ThresholdScan3000Single",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["ThresholdScanSparse"],
        "updateConfig" : False,
    },
    #{
    #    "name": "BasicScans3000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : False,
    #    "tools": ["AnalogScan", "DigitalScan", "RingOsc", "ADCScan", "DACScan"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : [16]
    #        },
    #    ]
    #},
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
                "values" : [130, 140, 150, 170, 180, 190, 200, 220, 240]
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
        "tools": ["ThresholdEqualization3000", "GlobalThresholdTuning1000", "GainTuning", "GlobalThresholdTuning1000", "ThresholdEqualization1000", "GlobalThresholdTuning1000", "ThresholdScanLow"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : [1]
            },
        ]
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
]

config_irradiationBase = [
    {
        "name": "ShortRingOsc_Mux",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["ShortRingOsc", "MuxScan"],
    },
    #{
    #    "name": "vmonitor",
    #    "type": "Vmonitor",
    #}
]

config_irradiationMain = [
    {
        "name": "VrefTrimming",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["VrefTrimming"],
    },
    {
        "name": "IVConfigured",
        "type": "IV",
        "configFile": "CROC.xml",
        "startingCurrent" : 1,
        "finalCurrent" : 2.5,
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
        "name": "ThresholdScan_previousTDACs",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["ThresholdScanLow"],
    },
    {
        "name": "ThresholdScanFixedTrigger_previousTDACs",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["ThresholdScanLowFixedTrigger"],
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
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
                "values" : [409]
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_LDAC_LIN"],
                "values" : [180]
            },
        ],
    },
    {
        "name": "ThresholdScanFixedTrigger_preIrradTDACs",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["ThresholdScanLowFixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : ["tdac_preIrradiation.csv"]
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
                "values" : [409]
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_LDAC_LIN"],
                "values" : [180]
            },
        ],
    },
    #{
    #    "name": "ThresholdTuning",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["GlobalThresholdTuning1000", "ThresholdEqualization1000", "GlobalThresholdTuning1000"],
    #},
    #{
    #    "name": "NoiseScan1000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : False,
    #    "tools": ["NoiseScan"],
    #},
    #{
    #    "name": "ThresholdScan_newTuning",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : False,
    #    "tools": ["ThresholdScanLow"]
    #},
    {
        "name": "MaskStuck",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [16]
            },
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
                "values" : [470]
            },
        ]
    },
    {
        "name": "MaskNoisy",
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
        "name": "MaskStuck3000untuned",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskNoisy3000untuned",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "ThresholdScan3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["ThresholdScanSparse"],
        "updateConfig" : False,
    },
    {
        "name": "ThresholdEqualization3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdEqualization3000"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskStuckNoisy3000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan", "NoiseScan"],
    },
    {
        "name": "GlobalThresholdTuningVcal200",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GlobalThresholdTuningVcal200"],
    },
    {
        "name": "ThresholdEqualizationVcal200",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdEqualizationVcal200"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskStuckNoisyVcal200",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan", "NoiseScan"],
    },
    {
        "name": "FinalTuningVcal200",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GlobalThresholdTuningVcal200"],
    },
    {
        "name": "MaskStuckFinalVcal200",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskNoisyFinalVcal200",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "ThresholdScanVcal200",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdScanSparse"],
    },
    {
        "name": "GlobalThresholdTuning1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GlobalThresholdTuning1000"],
    },
    {
        "name": "ThresholdEqualization1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdEqualization1000"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskStuckNoisy1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan", "NoiseScan"],
    },
    {
        "name": "FinalTuning1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GlobalThresholdTuning1000"],
    },
    {
        "name": "MaskStuckFinal",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScan"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    #{
    #    "name": "MaskNoisyFinal",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["NoiseScan"]
    #},
    {
        "name": "ThresholdScan1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdScanLow"],
    },
    {
        "name": "MaskStuckFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScanFixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [16]
            },
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
                "values" : [470]
            },
        ]
    },
    {
        "name": "MaskNoisyFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "GlobalThresholdTuning3000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["GlobalThresholdTuning3000FixedTrigger"],
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
        "name": "MaskStuck3000untunedFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScanFixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskNoisy3000untunedFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "ThresholdScan3000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["ThresholdScanSparseFixedTrigger"],
        "updateConfig" : False,
    },
    {
        "name": "ThresholdEqualization3000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdEqualization3000FixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskStuckNoisy3000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScanFixedTrigger", "NoiseScan"],
    },
    {
        "name": "GlobalThresholdTuningVcal200FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GlobalThresholdTuningVcal200FixedTrigger"],
    },
    {
        "name": "ThresholdEqualizationVcal200FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdEqualizationVcal200FixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskStuckNoisyVcal200FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScanFixedTrigger", "NoiseScan"],
    },
    {
        "name": "FinalTuningVcal200FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GlobalThresholdTuningVcal200FixedTrigger"],
    },
    {
        "name": "MaskStuckFinalVcal200FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScanFixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskNoisyFinalVcal200FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "ThresholdScanVcal200FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdScanSparseFixedTrigger"],
    },
    {
        "name": "GlobalThresholdTuning1000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GlobalThresholdTuning1000FixedTrigger"],
    },
    {
        "name": "ThresholdEqualization1000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdEqualization1000FixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskStuckNoisy1000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScanFixedTrigger", "NoiseScan"],
    },
    {
        "name": "FinalTuning1000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["GlobalThresholdTuning1000FixedTrigger"],
    },
    {
        "name": "MaskStuckFinalFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScanFixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskNoisyFinalFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "ThresholdScan1000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdScanLowFixedTrigger"],
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
        "name": "TimeWalk1000",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : False,
        "tools": ["TimeWalk"],
    }
]

config_postIrradiation = [
    #{
    #    "name": "VrefTrimming",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["VrefTrimming"],
    #},
    #{
    #    "name": "IVConfigured",
    #    "type": "IV",
    #    "configFile": "CROC.xml",
    #    "startingCurrent" : 1,
    #    "finalCurrent" : 2.5,
    #    "currentStep" : 0.1
    #},
    #{
    #    "name": "IVDefault",
    #    "type": "IV",
    #    "startingCurrent" : 0.1,
    #    "finalCurrent" : 2.5,
    #    "currentStep" : 0.1
    #},
    #{
    #    "name": "ThresholdScan_previousTDACs",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : False,
    #    "tools": ["ThresholdScanLow"],
    #},
    #{
    #    "name": "ThresholdScanFixedTrigger_previousTDACs",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : False,
    #    "tools": ["ThresholdScanLowFixedTrigger"],
    #},
    #{
    #    "name": "ThresholdScan_preIrradTDACs",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : False,
    #    "tools": ["ThresholdScanLow"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : ["tdac_preIrradiation.csv"]
    #        },
    #        {
    #            "table" : "Registers",
    #            "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
    #            "values" : [409]
    #        },
    #        {
    #            "table" : "Registers",
    #            "keys" : ["DAC_LDAC_LIN"],
    #            "values" : [180]
    #        },
    #    ],
    #},
    #{
    #    "name": "ThresholdScanFixedTrigger_preIrradTDACs",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : False,
    #    "tools": ["ThresholdScanLowFixedTrigger"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : ["tdac_preIrradiation.csv"]
    #        },
    #        {
    #            "table" : "Registers",
    #            "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
    #            "values" : [409]
    #        },
    #        {
    #            "table" : "Registers",
    #            "keys" : ["DAC_LDAC_LIN"],
    #            "values" : [180]
    #        },
    #    ],
    #},
    #{
    #    "name": "MaskStuck",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["StuckPixelScan"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : [16]
    #        },
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : ["enable.csv"]
    #        },
    #        {
    #            "table" : "Registers",
    #            "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
    #            "values" : [470]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskNoisy",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["NoiseScan"]
    #},
    #{
    #    "name": "GlobalThresholdTuning3000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "tools": ["GlobalThresholdTuning3000"],
    #    "updateConfig" : True,
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : [16]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskStuck3000untuned",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["StuckPixelScan"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : ["enable.csv"]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskNoisy3000untuned",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["NoiseScan"]
    #},
    #{
    #    "name": "ThresholdScan3000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "tools": ["ThresholdScanSparse"],
    #    "updateConfig" : False,
    #},
    #{
    #    "name": "ThresholdScan3000manyCombinations",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "tools": ["ThresholdScanSparse"],
    #    "updateConfig" : False,
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : ["enable.csv"]
    #        },
    #        {
    #            "table" : "Registers",
    #            "keys" : ["DAC_PREAMP_L_LIN", "DAC_PREAMP_R_LIN", "DAC_PREAMP_TL_LIN", "DAC_PREAMP_TR_LIN", "DAC_PREAMP_T_LIN", "DAC_PREAMP_M_LIN"],
    #            "values" : [100, 250, 200, 300, 400]
    #        },
    #        {
    #            "table" : "Registers",
    #            "keys" : ["DAC_LDAC_LIN"],
    #            "values" : [130, 140, 150, 170, 180, 190, 200, 220, 240]
    #        },
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["tdac"],
    #            "values" : [0, 16, 31]
    #        }
    #    ]
    #},
    #{
    #    "name": "ThresholdEqualization3000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["ThresholdEqualization3000"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : ["enable.csv"]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskStuckNoisy3000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["StuckPixelScan", "NoiseScan"],
    #},
    #{
    #    "name": "GlobalThresholdTuningVcal200",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["GlobalThresholdTuningVcal200"],
    #},
    #{
    #    "name": "ThresholdEqualizationVcal200",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["ThresholdEqualizationVcal200"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : ["enable.csv"]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskStuckNoisyVcal200",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["StuckPixelScan", "NoiseScan"],
    #},
    #{
    #    "name": "FinalTuningVcal200",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["GlobalThresholdTuningVcal200"],
    #},
    #{
    #    "name": "MaskStuckFinalVcal200",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["StuckPixelScan"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : ["enable.csv"]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskNoisyFinalVcal200",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["NoiseScan"]
    #},
    #{
    #    "name": "ThresholdScanVcal200",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["ThresholdScanSparse"],
    #},
    #{
    #    "name": "GlobalThresholdTuning1000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["GlobalThresholdTuning1000"],
    #},
    #{
    #    "name": "ThresholdEqualization1000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["ThresholdEqualization1000"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : ["enable.csv"]
    #        },
    #    ]
    #},
    #{
    #    "name": "MaskStuckNoisy1000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["StuckPixelScan", "NoiseScan"],
    #},
    #{
    #    "name": "FinalTuning1000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["GlobalThresholdTuning1000"],
    #},
    #{
    #    "name": "MaskStuckFinal",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["StuckPixelScan"],
    #    "params": [
    #        {
    #            "table" : "Pixels",
    #            "keys" : ["enable"],
    #            "values" : ["enable.csv"]
    #        },
    #    ]
    #},
    ##{
    ##    "name": "MaskNoisyFinal",
    ##    "type": "Ph2_ACF",
    ##    "configFile": "CROC.xml",
    ##    "updateConfig" : True,
    ##    "tools": ["NoiseScan"]
    ##},
    #{
    #    "name": "ThresholdScan1000",
    #    "type": "Ph2_ACF",
    #    "configFile": "CROC.xml",
    #    "updateConfig" : True,
    #    "tools": ["ThresholdScanLow"],
    #},
    {
        "name": "MaskStuckFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScanFixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [16]
            },
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"],
                "values" : [470]
            },
        ]
    },
    {
        "name": "MaskNoisyFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "GlobalThresholdTuning3000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["GlobalThresholdTuning3000FixedTrigger"],
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
        "name": "MaskStuck3000untunedFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["StuckPixelScanFixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "MaskNoisy3000untunedFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["NoiseScan"]
    },
    {
        "name": "ThresholdScanUntuned3000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["ThresholdScanSparseFixedTrigger"],
        "updateConfig" : False,
    },
    {
        "name": "ThresholdScan3000manyCombinationsFixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["ThresholdScanSparseFixedTrigger"],
        "updateConfig" : False,
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_PREAMP_L_LIN", "DAC_PREAMP_R_LIN", "DAC_PREAMP_TL_LIN", "DAC_PREAMP_TR_LIN", "DAC_PREAMP_T_LIN", "DAC_PREAMP_M_LIN"],
                "values" : [100, 250, 200, 300, 400]
            },
            {
                "table" : "Registers",
                "keys" : ["DAC_LDAC_LIN"],
                "values" : [130, 140, 150, 170, 180, 190, 200, 220, 240]
            },
            {
                "table" : "Pixels",
                "keys" : ["tdac"],
                "values" : [0, 16, 31]
            }
        ]
    },
    {
        "name": "ThresholdEqualization3000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "updateConfig" : True,
        "tools": ["ThresholdEqualization3000FixedTrigger"],
        "params": [
            {
                "table" : "Pixels",
                "keys" : ["enable"],
                "values" : ["enable.csv"]
            },
        ]
    },
    {
        "name": "ThresholdScan3000FixedTrigger",
        "type": "Ph2_ACF",
        "configFile": "CROC.xml",
        "tools": ["ThresholdScanSparseFixedTrigger"],
        "updateConfig" : False,
    },
#    {
#        "name": "MaskStuckNoisy3000FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["StuckPixelScanFixedTrigger", "NoiseScan"],
#    },
#    {
#        "name": "GlobalThresholdTuningVcal200FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["GlobalThresholdTuningVcal200FixedTrigger"],
#    },
#    {
#        "name": "ThresholdEqualizationVcal200FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["ThresholdEqualizationVcal200FixedTrigger"],
#        "params": [
#            {
#                "table" : "Pixels",
#                "keys" : ["enable"],
#                "values" : ["enable.csv"]
#            },
#        ]
#    },
#    {
#        "name": "MaskStuckNoisyVcal200FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["StuckPixelScanFixedTrigger", "NoiseScan"],
#    },
#    {
#        "name": "FinalTuningVcal200FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["GlobalThresholdTuningVcal200FixedTrigger"],
#    },
#    {
#        "name": "MaskStuckFinalVcal200FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["StuckPixelScanFixedTrigger"],
#        "params": [
#            {
#                "table" : "Pixels",
#                "keys" : ["enable"],
#                "values" : ["enable.csv"]
#            },
#        ]
#    },
#    {
#        "name": "MaskNoisyFinalVcal200FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["NoiseScan"]
#    },
#    {
#        "name": "ThresholdScanVcal200FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["ThresholdScanSparseFixedTrigger"],
#    },
#    {
#        "name": "GlobalThresholdTuning1000FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["GlobalThresholdTuning1000FixedTrigger"],
#    },
#    {
#        "name": "ThresholdEqualization1000FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["ThresholdEqualization1000FixedTrigger"],
#        "params": [
#            {
#                "table" : "Pixels",
#                "keys" : ["enable"],
#                "values" : ["enable.csv"]
#            },
#        ]
#    },
#    {
#        "name": "MaskStuckNoisy1000FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["StuckPixelScanFixedTrigger", "NoiseScan"],
#    },
#    {
#        "name": "FinalTuning1000FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["GlobalThresholdTuning1000FixedTrigger"],
#    },
#    {
#        "name": "MaskStuckFinalFixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["StuckPixelScanFixedTrigger"],
#        "params": [
#            {
#                "table" : "Pixels",
#                "keys" : ["enable"],
#                "values" : ["enable.csv"]
#            },
#        ]
#    },
#    {
#        "name": "MaskNoisyFinalFixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["NoiseScan"]
#    },
#    {
#        "name": "ThresholdScan1000FixedTrigger",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : True,
#        "tools": ["ThresholdScanLowFixedTrigger"],
#    },
#    {
#        "name": "ToTMeasurement1000",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : False,
#        "tools": ["AnalogScan"],
#        "params": [
#            {
#                "table" : "Registers",
#                "keys" : ["VCAL_HIGH"],
#                "values" : [1391]
#            },
#        ]
#    },
#    {
#        "name": "BasicScans1000",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : False,
#        "tools": ["AnalogScan", "DigitalScan", "RingOsc", "ADCScan", "DACScan"],
#    },
#    {
#        "name": "TimeWalk1000",
#        "type": "Ph2_ACF",
#        "configFile": "CROC.xml",
#        "updateConfig" : False,
#        "tools": ["TimeWalk"],
#    }
]
