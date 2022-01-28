#include "RD53BConstants.h"

namespace Ph2_HwDescription
{

    namespace RD53BConstants 
    {

        const std::unordered_map<std::string, uint16_t> GlobalPulseRoutes = {
            {"ResetChannelSynchronizer", 0},
            {"ResetCommandDecoder", 1},
            {"ResetGlobalConfiguration", 2},
            {"ResetServiceData", 3},
            {"ResetAurora", 4},
            {"ResetSerializers", 5},
            {"ResetADC", 6},
            {"ResetDataMerging", 7},
            {"ResetEfuses", 8},
            {"ResetTriggerTable", 9},
            {"ResetBCIDCounter", 10},
            {"SendCalResetPulse", 11},
            {"ADCStartOfConversion", 12},
            {"StartRingOscillatorsA", 13},
            {"StartRingOscillatorsB", 14},
            {"StartEfusesProgrammer", 15}
        };

        const std::unordered_map<std::string, uint16_t> ATLAS_IMUX = {
            {"IREF_4UA", 0},
            {"CDR_VCO", 1},
            {"CDR_VCOBUFF", 2},
            {"CDR_CP", 3},
            {"CDR_CPFD", 4},
            {"CDR_CPBUFF", 5},
            {"CML_BIAS_2", 6},
            {"CML_BIAS_1", 7},
            {"CML_BIAS_0", 8},
            {"I_NTC", 9},
            {"INJ_CAP", 10},
            {"INJ_CAP_PAR", 11},
            {"I_PREAMP_M", 12},
            {"I_PRECOMPARATOR", 13},
            {"I_COMPARATOR", 14},
            {"I_VTH2", 15},
            {"I_VTH1_M", 16},
            {"LEAKAGE_CURRENT_COMP", 17},
            {"I_FEEDBACK ", 18},
            {"I_PREAMP_L", 19},
            {"I_VTH1_L", 20},
            {"I_PREAMP_R", 21},
            {"I_PREAMP_TL", 22},
            {"I_VTH1_R", 23},
            {"I_PREAMP TOP PIXELS", 24},
            {"I_PREAMP TOP-RIGHT PIXELS", 25},
            {"IINA", 28},
            {"ISHUNTA", 29},
            {"IIND", 30},
            {"ISHUNTD", 31}
        };

        const std::unordered_map<std::string, uint16_t> ATLAS_VMUX = {
            {"VREF_ADC", 0},
            {"IMUX_OUT", 1},
            {"NTC", 2},
            {"VREF_VCAL_DAC", 3},
            {"VMON_HALFVDDA_INJCAP", 4},
            {"RPOLYTSENS_TOP", 5},
            {"RPOLYTSENS_BOTTOM", 6},
            {"VCAL_HI_GBL", 7},
            {"VCAL_MI_GBL", 8},
            {"VTH2G", 9},
            {"VTH1G_M ", 10},
            {"VTH1G_L ", 11},
            {"VTH1G_R ", 12},
            {"RADSENS_SLDOA", 13},
            {"TEMPSENS_SLDOA", 14},
            {"RADSENS_SLDOD", 15},
            {"TEMPSENS_SLDOD", 16},
            {"RADSENS_ACB", 17},
            {"TEMPSENS_ACB", 18},
            {"GNDA", 19},
            {"VMON_VREF_CORE", 31},
            {"VMON_VREF_PRE", 32},
            {"VMON_VINA", 33},
            {"VMON_VDDA", 34},
            {"VMON_VREFA", 35},
            {"VMON_VOFS", 36},
            {"VMON_VIND", 37},
            {"VMON_VDDD", 38},
            {"VMON_VREFD", 39}
        };

        const std::unordered_map<std::string, uint16_t> CMS_IMUX = {
            {"IREF_4UA", 0},
            {"CDR_VCO", 1},
            {"CDR_VCOBUFF", 2},
            {"CDR_CP", 3},
            {"CDR_CPFD", 4},
            {"CDR_CPBUFF", 5},
            {"CML_BIAS_2", 6},
            {"CML_BIAS_1", 7},
            {"CML_BIAS_0", 8},
            {"I_NTC", 9},
            {"INJ_CAP", 10},
            {"INJ_CAP_PAR", 11},
            {"I_PREAMP_M", 12},
            {"I_COMPARATOR_STAR", 13},
            {"I_COMPARATOR", 14},
            {"I_LDAC", 15},
            {"I_FC", 16},
            {"I_KRUMMENAKER", 17},
            {"I_PREAMP_L", 19},
            {"I_PREAMP_R", 21},
            {"I_PREAMP_TL", 22},
            {"I_PREAMP_T", 24},
            {"I_PREAMP_TR", 25},
            {"IINA", 28},
            {"ISHUNTA", 29},
            {"IIND", 30},
            {"ISHUNTD", 31}
        };

        const std::unordered_map<std::string, uint16_t> CMS_VMUX = {
            {"VREF_ADC", 0},
            {"IMUX_OUT", 1},
            {"NTC", 2},
            {"VREF_VCAL_DAC", 3},
            {"VMON_HALFVDDA_INJCAP", 4},
            {"RPOLYTSENS_TOP", 5},
            {"RPOLYTSENS_BOTTOM", 6},
            {"CAL_HI_GBL", 7},
            {"CAL_MI_GBL", 8},
            {"VREF_KRUM", 9},
            {"GDAC_M", 10},
            {"GDAC_L", 11},
            {"GDAC_R", 12},
            {"RADSENS_SLDOA", 13},
            {"TEMPSENS_SLDOA", 14},
            {"RADSENS_SLDOD", 15},
            {"TEMPSENS_SLDOD", 16},
            {"RADSENS_ACB", 17},
            {"TEMPSENS_ACB", 18},
            {"GNDA", 19},
            {"VMON_VREF_CORE", 31},
            {"VMON_VREF_PRE", 32},
            {"VMON_VINA", 33},
            {"VMON_VDDA", 34},
            {"VMON_VREFA", 35},
            {"VMON_VOFS", 36},
            {"VMON_VIND", 37},
            {"VMON_VDDD", 38},
            {"VMON_VREFD", 39}
        };

    } // namespace RD53BConstants

} // namespace Ph2_HwDescription