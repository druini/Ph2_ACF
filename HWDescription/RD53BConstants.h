#ifndef RD53BCONSTANTS_H
#define RD53BCONSTANTS_H

#include <unordered_map>
#include <string>

namespace Ph2_HwDescription
{

    namespace RD53BConstants 
    {

        extern const std::unordered_map<std::string, uint8_t> GlobalPulseRoutes;

        extern const std::unordered_map<std::string, uint8_t> AtlasIMuxMap;

        extern const std::unordered_map<std::string, uint8_t> AtlasVMuxMap;

        extern const std::unordered_map<std::string, uint8_t> CmsIMuxMap;

        extern const std::unordered_map<std::string, uint8_t> CmsVMuxMap;

        enum class AtlasIMux : uint8_t {
            IREF_4UA = 0,
            CDR_VCO = 1,
            CDR_VCOBUFF = 2,
            CDR_CP = 3,
            CDR_CPFD = 4,
            CDR_CPBUFF = 5,
            CML_BIAS_2 = 6,
            CML_BIAS_1 = 7,
            CML_BIAS_0 = 8,
            I_NTC = 9,
            INJ_CAP = 10,
            INJ_CAP_PAR = 11,
            I_PREAMP_M = 12,
            I_PRECOMPARATOR = 13,
            I_COMPARATOR = 14,
            I_VTH2 = 15,
            I_VTH1_M = 16,
            LEAKAGE_CURRENT_COMP = 17,
            I_FEEDBACK  = 18,
            I_PREAMP_L = 19,
            I_VTH1_L = 20,
            I_PREAMP_R = 21,
            I_PREAMP_TL = 22,
            I_VTH1_R = 23,
            I_PREAMP_T = 24,
            I_PREAMP_TR = 25,
            IINA = 28,
            ISHUNTA = 29,
            IIND = 30,
            ISHUNTD = 31,
            HIGH_Z = 6
        };

        enum class AtlasVMux : uint8_t {
            VREF_ADC = 0,
            IMUX_OUT = 1,
            NTC = 2,
            VREF_VCAL_DAC = 3,
            INJCAP_VDDA_HALF = 4,
            RPOLYTSENS_TOP = 5,
            RPOLYTSENS_BOTTOM = 6,
            VCAL_HIGH = 7,
            VCAL_MED = 8,
            VTH2G = 9,
            VTH1G_M  = 10,
            VTH1G_L  = 11,
            VTH1G_R  = 12,
            RADSENS_SLDOA = 13,
            TEMPSENS_SLDOA = 14,
            RADSENS_SLDOD = 15,
            TEMPSENS_SLDOD = 16,
            RADSENS_ACB = 17,
            TEMPSENS_ACB = 18,
            GNDA = 19,
            VREF_CORE = 31,
            VREF_PRE = 32,
            VINA_QUARTER = 33,
            VDDA_HALF = 34,
            VREFA = 35,
            VOFS_QUARTER = 36,
            VIND_QUARTER = 37,
            VDDD_HALF = 38,
            VREFD = 39,
            HIGH_Z = 63
        };

        enum class CmsIMux : uint8_t {
            IREF_4UA = 0,
            CDR_VCO = 1,
            CDR_VCOBUFF = 2,
            CDR_CP = 3,
            CDR_CPFD = 4,
            CDR_CPBUFF = 5,
            CML_BIAS_2 = 6,
            CML_BIAS_1 = 7,
            CML_BIAS_0 = 8,
            I_NTC = 9,
            INJ_CAP = 10,
            INJ_CAP_PAR = 11,
            I_PREAMP_M = 12,
            I_COMPARATOR_STAR = 13,
            I_COMPARATOR = 14,
            I_LDAC = 15,
            I_FC = 16,
            I_KRUMMENAKER = 17,
            I_PREAMP_L = 19,
            I_PREAMP_R = 21,
            I_PREAMP_TL = 22,
            I_PREAMP_T = 24,
            I_PREAMP_TR = 25,
            IINA = 28,
            ISHUNTA = 29,
            IIND = 30,
            ISHUNTD = 31,
            HIGH_Z = 63
        };

        enum class CmsVMux : uint8_t {
            VREF_ADC = 0,
            IMUX_OUT = 1,
            NTC = 2,
            VREF_VCAL_DAC = 3,
            INJCAP_VDDA_HALF = 4,
            RPOLYTSENS_TOP = 5,
            RPOLYTSENS_BOTTOM = 6,
            VCAL_HIGH = 7,
            VCAL_MED = 8,
            VREF_KRUM = 9,
            GDAC_M = 10,
            GDAC_L = 11,
            GDAC_R = 12,
            RADSENS_SLDOA = 13,
            TEMPSENS_SLDOA = 14,
            RADSENS_SLDOD = 15,
            TEMPSENS_SLDOD = 16,
            RADSENS_ACB = 17,
            TEMPSENS_ACB = 18,
            GNDA = 19,
            VREF_CORE = 31,
            VREF_PRE = 32,
            VINA_QUARTER = 33,
            VDDA_HALF = 34,
            VREFA = 35,
            VOFS_QUARTER = 36,
            VIND_QUARTER = 37,
            VDDD_HALF = 38,
            VREFD = 39,
            HIGH_Z = 63
        };

    } // namespace RD53BConstants

} // namespace Ph2_HwDescription

#endif
