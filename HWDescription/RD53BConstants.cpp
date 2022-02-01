#include "RD53BConstants.h"

namespace Ph2_HwDescription
{

    namespace RD53BConstants 
    {

        const std::unordered_map<std::string, uint8_t> GlobalPulseRoutes = {
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

        const std::unordered_map<std::string, uint8_t> AtlasIMuxMap = {
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
            {"I_PREAMP_T", 24},
            {"I_PREAMP_TR", 25},
            {"IINA", 28},
            {"ISHUNTA", 29},
            {"IIND", 30},
            {"ISHUNTD", 31},
            {"HIGH_Z", 63}
        };

        const std::unordered_map<std::string, uint8_t> AtlasVMuxMap = {
            {"VREF_ADC", 0},
            {"IMUX_OUT", 1},
            {"NTC", 2},
            {"VREF_VCAL_DAC", 3},
            {"INJCAP_VDDA_HALF", 4},
            {"RPOLYTSENS_TOP", 5},
            {"RPOLYTSENS_BOTTOM", 6},
            {"VCAL_HIGH", 7},
            {"VCAL_MED", 8},
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
            {"VREF_CORE", 31},
            {"VREF_PRE", 32},
            {"VINA_QUARTER", 33},
            {"VDDA_HALF", 34},
            {"VREFA", 35},
            {"VOFS_QUARTER", 36},
            {"VIND_QUARTER", 37},
            {"VDDD_HALF", 38},
            {"VREFD", 39},
            {"HIGH_Z", 63}
        };

        const std::unordered_map<std::string, uint8_t> CmsIMuxMap = {
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
            {"ISHUNTD", 31},
            {"HIGH_Z", 63}
        };

        const std::unordered_map<std::string, uint8_t> CmsVMuxMap = {
            {"VREF_ADC", 0},
            {"IMUX_OUT", 1},
            {"NTC", 2},
            {"VREF_VCAL_DAC", 3},
            {"INJCAP_VDDA_HALF", 4},
            {"RPOLYTSENS_TOP", 5},
            {"RPOLYTSENS_BOTTOM", 6},
            {"VCAL_HIGH", 7},
            {"VCAL_MED", 8},
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
            {"VREF_CORE", 31},
            {"VREF_PRE", 32},
            {"VINA_QUARTER", 33},
            {"VDDA_HALF", 34},
            {"VREFA", 35},
            {"VOFS_QUARTER", 36},
            {"VIND_QUARTER", 37},
            {"VDDD_HALF", 38},
            {"VREFD", 39},
            {"HIGH_Z", 63}
        };

    } // namespace RD53BConstants

} // namespace Ph2_HwDescription