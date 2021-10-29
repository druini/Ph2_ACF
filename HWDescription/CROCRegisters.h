#ifndef CROCREGISTERS_H
#define CROCREGISTERS_H

#include <numeric>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>

#include "RD53Register.h"

namespace Ph2_HwDescription
{

struct CROCReg {

    static constexpr size_t nRegs = 151;

    using Register = RD53Register;

    static const auto& GetRegisters() {
        static const std::vector<Register> registers{{
            {"PIX_PORTAL", 0, {16}, {0}},
            {"REGION_COL", 1, {8}, {0}},
            {"REGION_ROW", 2, {9}, {0}},
            {"PIX_MODE", 3, {1, 1, 1}, {0, 1, 0}},
            {"PIX_DEFAULT_CONFIG", 4, {16}, {0}},
            {"PIX_DEFAULT_CONFIG_B", 5, {16}, {0}},
            {"GCR_DEFAULT_CONFIG", 6, {16}, {0}},
            {"GCR_DEFAULT_CONFIG_B", 7, {16}, {0}},
            {"DAC_PREAMP_L_DIFF", 8, {10}, {50}},
            {"DAC_PREAMP_R_DIFF", 9, {10}, {50}},
            {"DAC_PREAMP_TL_DIFF", 10, {10}, {50}},
            {"DAC_PREAMP_TR_DIFF", 11, {10}, {50}},
            {"DAC_PREAMP_T_DIFF", 12, {10}, {50}},
            {"DAC_PREAMP_M_DIFF", 13, {10}, {50}},
            {"DAC_PRECOMP_DIFF", 14, {10}, {50}},
            {"DAC_COMP_DIFF", 15, {10}, {50}},
            {"DAC_VFF_DIFF", 16, {10}, {100}},
            {"DAC_TH1_L_DIFF", 17, {10}, {100}},
            {"DAC_TH1_R_DIFF", 18, {10}, {100}},
            {"DAC_TH1_M_DIFF", 19, {10}, {100}},
            {"DAC_TH2_DIFF", 20, {10}, {0}},
            {"DAC_LCC_DIFF", 21, {10}, {100}},
            {"DAC_PREAMP_L_LIN", 22, {10}, {300}},
            {"DAC_PREAMP_R_LIN", 23, {10}, {300}},
            {"DAC_PREAMP_TL_LIN", 24, {10}, {300}},
            {"DAC_PREAMP_TR_LIN", 25, {10}, {300}},
            {"DAC_PREAMP_T_LIN", 26, {10}, {300}},
            {"DAC_PREAMP_M_LIN", 27, {10}, {300}},
            {"DAC_FC_LIN", 28, {10}, {20}},
            {"DAC_KRUM_CURR_LIN", 29, {10}, {50}},
            {"DAC_REF_KRUM_LIN", 30, {10}, {300}},
            {"DAC_COMP_LIN", 31, {10}, {110}},
            {"DAC_COMP_TA_LIN", 32, {10}, {110}},
            {"DAC_GDAC_L_LIN", 33, {10}, {408}},
            {"DAC_GDAC_R_LIN", 34, {10}, {408}},
            {"DAC_GDAC_M_LIN", 35, {10}, {408}},
            {"DAC_LDAC_LIN", 36, {10}, {100}},
            {"LEACKAGE_FEEDBACK", 37, {1, 1}, {0, 0}},
            {"VOLTAGE_TRIM", 38, {1, 1, 4, 4}, {0, 0, 8, 8}},
            {"EnCoreCol_3", 39, {1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0}},
            {"EnCoreCol_2", 40, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"EnCoreCol_1", 41, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"EnCoreCol_0", 42, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"EnCoreColumnReset_3", 43, {1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0}},
            {"EnCoreColumnReset_2", 44, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"EnCoreColumnReset_1", 45, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"EnCoreColumnReset_0", 46, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"TriggerConfig", 47, {1, 9}, {0, 500}},
            {"SelfTriggerConfig_1", 48, {1, 1, 4}, {0, 1, 1}},
            {"SelfTriggerConfig_0", 49, {10, 5}, {100, 1}},
            {"HitOrPatternLUT", 50, {16}, {0}},
            {"ReadTriggerConfig", 51, {2, 12}, {0, 1000}},
            {"TruncationTimeoutConf", 52, {12}, {0}},
            {"CalibrationConfig", 53, {1, 1, 6}, {0, 0, 0}},
            {"CLK_DATA_FINE_DELAY", 54, {6, 6}, {0, 0}},
            {"VCAL_HIGH", 55, {12}, {500}},
            {"VCAL_MED", 56, {12}, {300}},
            {"MEAS_CAP", 57, {1, 1, 1}, {0, 0, 0}},
            {"CdrConf", 58, {1, 1, 3}, {0, 0, 0}},
            {"CkEnConf", 59, {3, 3, 3, 3}, {7, 7, 7, 7}},
            {"ChSyncConf", 60, {5}, {16}},
            {"GlobalPulseConf", 61, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"GlobalPulseWidth", 62, {8}, {1}},
            {"ServiceDataConf", 63, {1, 8}, {0, 50}},
            {"ToTConfig", 64, {1, 1, 1, 1, 9}, {0, 0, 0, 0, 500}},
            {"PrecisionToTEnable_3", 65, {1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0}},
            {"PrecisionToTEnable_2", 66, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"PrecisionToTEnable_1", 67, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"PrecisionToTEnable_0", 68, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"DataMerging", 69, {4, 1, 1, 1, 4, 1, 1}, {0, 1, 1, 0, 0, 0, 1}},
            {"DataMergingMux", 70, {2, 2, 2, 2, 2, 2, 2, 2}, {3, 2, 1, 0, 3, 2, 1, 0}},
            {"EnCoreColumnCalibration_3", 71, {1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1}},
            {"EnCoreColumnCalibration_2", 72, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
            {"EnCoreColumnCalibration_1", 73, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
            {"EnCoreColumnCalibration_0", 74, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
            {"DataConcentratorConf", 75, {1, 1, 1, 8}, {0, 0, 1, 16}},
            {"CoreColEncoderConf", 76, {1, 1, 4, 3}, {0, 0, 0, 0}},
            {"EnHitsRemoval_3", 77, {16}, {0}},
            {"EnHitsRemoval_2", 78, {16}, {0}},
            {"EnHitsRemoval_1", 79, {16}, {0}},
            {"EnHitsRemoval_0", 80, {6}, {0}},
            {"EnIsolHitsRemoval_3", 81, {16}, {0}},
            {"EnIsolHitsRemoval_2", 82, {16}, {0}},
            {"EnIsolHitsRemoval_1", 83, {16}, {0}},
            {"EnIsolHitsRemoval_0", 84, {6}, {0}},
            {"EvenMask", 85, {16}, {0}},
            {"OddMask", 86, {16}, {0}},
            {"EfusesConfig", 87, {16}, {0}},
            {"EfusesWriteData1", 88, {16}, {0}},
            {"EfusesWriteData0", 89, {16}, {0}},
            {"AuroraConfig", 90, {1, 4, 6, 2}, {0, 1, 25, 3}},
            {"AURORA_CB_CONFIG1", 91, {8}, {255}},
            {"AURORA_CB_CONFIG0", 92, {12, 4}, {4095, 0}},
            {"AURORA_INIT_WAIT", 93, {11}, {32}},
            {"AURORA_AltOutput_1", 94, {4}, {0}},
            {"AURORA_AltOutput_0", 95, {11}, {0}},
            {"OUTPUT_PAD_CONFIG", 96, {4, 1, 1, 4, 3}, {5, 1, 0, 15, 7}},
            {"GP_CMOS_ROUTE", 97, {6}, {34}},
            {"GP_LVDS_ROUTE_1", 98, {6, 6}, {35, 33}},
            {"GP_LVDS_ROUTE_0", 99, {6, 6}, {1, 0}},
            {"DAC_CP_CDR", 100, {10}, {40}},
            {"DAC_CP_FD_CDR", 101, {10}, {400}},
            {"DAC_CP_BUFF_CDR", 102, {10}, {200}},
            {"DAC_VCO_CDR", 103, {10}, {1023}},
            {"DAC_VCOBUFF_CDR", 104, {10}, {500}},
            {"SER_SEL_OUT", 105, {2, 2, 2, 2}, {1, 1, 1, 1}},
            {"CML_CONFIG", 106, {2, 2, 4}, {0, 0, 1}},
            {"DAC_CML_BIAS_2", 107, {10}, {0}},
            {"DAC_CML_BIAS_1", 108, {10}, {0}},
            {"DAC_CML_BIAS_0", 109, {10}, {500}},
            {"MonitorConfig", 110, {1, 6, 6}, {0, 63, 63}},
            {"ErrWngMask", 111, {1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0}},
            {"MON_SENS_SLDO", 112, {1, 4, 1, 1, 4, 1}, {0, 0, 0, 0, 0, 0}},
            {"MON_SENS_ACB", 113, {1, 4, 1}, {0, 0, 0}},
            {"MON_ADC", 114, {1, 1, 1, 6}, {0, 0, 1, 0}},
            {"DAC_NTC", 115, {10}, {100}},
            {"HITOR_MASK_3", 116, {1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0}},
            {"HITOR_MASK_2", 117, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"HITOR_MASK_1", 118, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"HITOR_MASK_0", 119, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"AutoRead0", 120, {9}, {137}},
            {"AutoRead1", 121, {9}, {133}},
            {"AutoRead2", 122, {9}, {121}},
            {"AutoRead3", 123, {9}, {122}},
            {"AutoRead4", 124, {9}, {124}},
            {"AutoRead5", 125, {9}, {127}},
            {"AutoRead6", 126, {9}, {126}},
            {"AutoRead7", 127, {9}, {125}},
            {"RingOscConfig", 128, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}},
            {"RingOscRoute", 129, {3, 6}, {0, 0}},
            {"RING_OSC_A_OUT", 130, {16}, {0}, true},
            {"RING_OSC_B_OUT", 131, {16}, {0}, true},
            {"BCIDCnt", 132, {16}, {0}, true},
            {"TrigCnt", 133, {16}, {0}, true},
            {"ReadTrigCnt", 134, {16}, {0}, true},
            {"LockLossCnt", 135, {16}, {0}, true},
            {"BitFlipWngCnt", 136, {16}, {0}, true},
            {"BitFlipErrCnt", 137, {16}, {0}, true},
            {"CmdErrCnt", 138, {16}, {0}, true},
            {"RdWrFifoErrorCount", 139, {16}, {0}, true},
            {"AI_REGION_ROW", 140, {9}, {0}, true},
            {"HitOr_3_Cnt", 141, {16}, {0}, true},
            {"HitOr_2_Cnt", 142, {16}, {0}, true},
            {"HitOr_1_Cnt", 143, {16}, {0}, true},
            {"HitOr_0_Cnt", 144, {16}, {0}, true},
            {"Pixel_SEU_Cnt", 145, {16}, {0}, true},
            {"GlobalConfig_SEU_Cnt", 146, {16}, {0}, true},
            {"SkippedTriggerCnt", 147, {16}, {0}, true},
            {"EfusesReadData1", 148, {16}, {0}, true},
            {"EfusesReadData0", 149, {16}, {0}, true},
            {"MonitoringDataADC", 150, {12}, {0}, true}
        }};
        return registers;
    }

    
    static const auto& GetRegisterIndexMap() {
        static const std::unordered_map<std::string, uint16_t> map = [] () {
            std::unordered_map<std::string, uint16_t> map;
            for (size_t i = 0; i < GetRegisters().size(); ++i) {
                map.insert({GetRegisters()[i].name.c_str(), i});
            }
            return map;
        } ();
        return map;
    };

    static const std::vector<Register>& Registers;
    
    static const std::unordered_map<std::string, uint16_t>& RegisterIndexMap;

    static const Register& PIX_PORTAL;
    static const Register& REGION_COL;
    static const Register& REGION_ROW;
    static const Register& PIX_MODE;
    static const Register& PIX_DEFAULT_CONFIG;
    static const Register& PIX_DEFAULT_CONFIG_B;
    static const Register& GCR_DEFAULT_CONFIG;
    static const Register& GCR_DEFAULT_CONFIG_B;
    static const Register& DAC_PREAMP_L_DIFF;
    static const Register& DAC_PREAMP_R_DIFF;
    static const Register& DAC_PREAMP_TL_DIFF;
    static const Register& DAC_PREAMP_TR_DIFF;
    static const Register& DAC_PREAMP_T_DIFF;
    static const Register& DAC_PREAMP_M_DIFF;
    static const Register& DAC_PRECOMP_DIFF;
    static const Register& DAC_COMP_DIFF;
    static const Register& DAC_VFF_DIFF;
    static const Register& DAC_TH1_L_DIFF;
    static const Register& DAC_TH1_R_DIFF;
    static const Register& DAC_TH1_M_DIFF;
    static const Register& DAC_TH2_DIFF;
    static const Register& DAC_LCC_DIFF;
    static const Register& DAC_PREAMP_L_LIN;
    static const Register& DAC_PREAMP_R_LIN;
    static const Register& DAC_PREAMP_TL_LIN;
    static const Register& DAC_PREAMP_TR_LIN;
    static const Register& DAC_PREAMP_T_LIN;
    static const Register& DAC_PREAMP_M_LIN;
    static const Register& DAC_FC_LIN;
    static const Register& DAC_KRUM_CURR_LIN;
    static const Register& DAC_REF_KRUM_LIN;
    static const Register& DAC_COMP_LIN;
    static const Register& DAC_COMP_TA_LIN;
    static const Register& DAC_GDAC_L_LIN;
    static const Register& DAC_GDAC_R_LIN;
    static const Register& DAC_GDAC_M_LIN;
    static const Register& DAC_LDAC_LIN;
    static const Register& LEACKAGE_FEEDBACK;
    static const Register& VOLTAGE_TRIM;
    static const Register& EnCoreCol_3;
    static const Register& EnCoreCol_2;
    static const Register& EnCoreCol_1;
    static const Register& EnCoreCol_0;
    static const Register& EnCoreColumnReset_3;
    static const Register& EnCoreColumnReset_2;
    static const Register& EnCoreColumnReset_1;
    static const Register& EnCoreColumnReset_0;
    static const Register& TriggerConfig;
    static const Register& SelfTriggerConfig_1;
    static const Register& SelfTriggerConfig_0;
    static const Register& HitOrPatternLUT;
    static const Register& ReadTriggerConfig;
    static const Register& TruncationTimeoutConf;
    static const Register& CalibrationConfig;
    static const Register& CLK_DATA_FINE_DELAY;
    static const Register& VCAL_HIGH;
    static const Register& VCAL_MED;
    static const Register& MEAS_CAP;
    static const Register& CdrConf;
    static const Register& CkEnConf;
    static const Register& ChSyncConf;
    static const Register& GlobalPulseConf;
    static const Register& GlobalPulseWidth;
    static const Register& ServiceDataConf;
    static const Register& ToTConfig;
    static const Register& PrecisionToTEnable_3;
    static const Register& PrecisionToTEnable_2;
    static const Register& PrecisionToTEnable_1;
    static const Register& PrecisionToTEnable_0;
    static const Register& DataMerging;
    static const Register& DataMergingMux;
    static const Register& EnCoreColumnCalibration_3;
    static const Register& EnCoreColumnCalibration_2;
    static const Register& EnCoreColumnCalibration_1;
    static const Register& EnCoreColumnCalibration_0;
    static const Register& DataConcentratorConf;
    static const Register& CoreColEncoderConf;
    static const Register& EnHitsRemoval_3;
    static const Register& EnHitsRemoval_2;
    static const Register& EnHitsRemoval_1;
    static const Register& EnHitsRemoval_0;
    static const Register& EnIsolHitsRemoval_3;
    static const Register& EnIsolHitsRemoval_2;
    static const Register& EnIsolHitsRemoval_1;
    static const Register& EnIsolHitsRemoval_0;
    static const Register& EvenMask;
    static const Register& OddMask;
    static const Register& EfusesConfig;
    static const Register& EfusesWriteData1;
    static const Register& EfusesWriteData0;
    static const Register& AuroraConfig;
    static const Register& AURORA_CB_CONFIG1;
    static const Register& AURORA_CB_CONFIG0;
    static const Register& AURORA_INIT_WAIT;
    static const Register& AURORA_AltOutput_1;
    static const Register& AURORA_AltOutput_0;
    static const Register& OUTPUT_PAD_CONFIG;
    static const Register& GP_CMOS_ROUTE;
    static const Register& GP_LVDS_ROUTE_1;
    static const Register& GP_LVDS_ROUTE_0;
    static const Register& DAC_CP_CDR;
    static const Register& DAC_CP_FD_CDR;
    static const Register& DAC_CP_BUFF_CDR;
    static const Register& DAC_VCO_CDR;
    static const Register& DAC_VCOBUFF_CDR;
    static const Register& SER_SEL_OUT;
    static const Register& CML_CONFIG;
    static const Register& DAC_CML_BIAS_2;
    static const Register& DAC_CML_BIAS_1;
    static const Register& DAC_CML_BIAS_0;
    static const Register& MonitorConfig;
    static const Register& ErrWngMask;
    static const Register& MON_SENS_SLDO;
    static const Register& MON_SENS_ACB;
    static const Register& MON_ADC;
    static const Register& DAC_NTC;
    static const Register& HITOR_MASK_3;
    static const Register& HITOR_MASK_2;
    static const Register& HITOR_MASK_1;
    static const Register& HITOR_MASK_0;
    static const Register& AutoRead0;
    static const Register& AutoRead1;
    static const Register& AutoRead2;
    static const Register& AutoRead3;
    static const Register& AutoRead4;
    static const Register& AutoRead5;
    static const Register& AutoRead6;
    static const Register& AutoRead7;
    static const Register& RingOscConfig;
    static const Register& RingOscRoute;
    static const Register& RING_OSC_A_OUT;
    static const Register& RING_OSC_B_OUT;
    static const Register& BCIDCnt;
    static const Register& TrigCnt;
    static const Register& ReadTrigCnt;
    static const Register& LockLossCnt;
    static const Register& BitFlipWngCnt;
    static const Register& BitFlipErrCnt;
    static const Register& CmdErrCnt;
    static const Register& RdWrFifoErrorCount;
    static const Register& AI_REGION_ROW;
    static const Register& HitOr_3_Cnt;
    static const Register& HitOr_2_Cnt;
    static const Register& HitOr_1_Cnt;
    static const Register& HitOr_0_Cnt;
    static const Register& Pixel_SEU_Cnt;
    static const Register& GlobalConfig_SEU_Cnt;
    static const Register& SkippedTriggerCnt;
    static const Register& EfusesReadData1;
    static const Register& EfusesReadData0;
    static const Register& MonitoringDataADC;


};

} // namespace Ph2_HwDescription

#endif