#include <unordered_map>

namespace Ph2_HwDescription
{

namespace RD53BConstants 
{

const std::unordered_map<std::string, uint16_t> IMUX = {
    {"IREF", 0},
    {"CDR_VCO_main_bias", 1},
    {"CDR_VCO_buffer_bias", 2},
    {"CDR_CP_current", 3},
    {"CDR_FD_current", 4},
    {"CDR_buffer_bias", 5},
    {"CML_driver_tap_2_bias", 6},
    {"CML_driver_tap_1_bias", 7},
    {"CML_driver_main_bias", 8},
    {"NTC_pad_current", 9},
    {"Capmeasure_circuit", 10},
    {"Capmeasure_parasitic", 11},
    {"DIFF_FE_Preamp_Main_array", 12},
    {"DIFF_FE_PreComp", 13},
    {"DIFF_FE_Comparator", 14},
    {"DIFF_FE_VTH2", 15},
    {"DIFF_FE_VTH1_Main_array", 16},
    {"DIFF_FE_LCC", 17},
    {"DIFF_FE_Feedback", 18},
    {"DIFF_FE_Preamp_Left", 19},
    {"DIFF_FE_VTH1_Left", 20},
    {"DIFF_FE_Preamp_Right", 21},
    {"DIFF_FE Preamp Top-Left", 22},
    {"DIFF_FE VTH1 Right", 23},
    {"DIFF_FE Preamp Top", 24},
    {"DIFF_FE Preamp Top-Right", 25},
    {"Analog_input_current", 28},
    {"Analog_shunt_current", 29},
    {"Digital_input_current", 30},
    {"Digital_shunt_current", 31}
};

const std::unordered_map<std::string, uint16_t> VMUX = {
    {"Vref_ADC", 0},
    {"I_mux", 1},
    {"NTC", 2},
    {"VCAL_DAC_half", 3},
    {"VDDA_half", 4},
    {"Poly_TEMPSENS_top", 5},
    {"Poly_TEMPSENS_bottom", 6},
    {"VCAL_HI", 7},
    {"VCAL_MED", 8},
    {"DIFF_FE_VTH2", 9},
    {"DIFF_FE_VTH1_Main_array", 10},
    {"DIFF_FE_VTH1_Left", 11},
    {"DIFF_FE_VTH1_Right", 12},
    {"RADSENS_Analog_SLDO", 13},
    {"TEMPSENS_Analog_SLDO", 14},
    {"RADSENS_Digital_SLDO", 15},
    {"TEMPSENS_Digital_SLDO", 16},
    {"RADSENS_center", 17},
    {"TEMPSENS_center", 18},
    {"Analog_GND", 19},
    {"Vref_CORE", 31},
    {"Vref_PRE", 32},
    {"VINA_half", 33},
    {"VDDA_half", 34},
    {"VrefA", 35},
    {"VOFS_quarter", 36},
    {"VIND_quarter", 37},
    {"VDDD_half", 38},
    {"VrefD", 39}
};

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

} // namespace RD53BConstants

} // namespace Ph2_HwDescription