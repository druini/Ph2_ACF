#include "RD53BDACTest.h"

#include <map>

#include "RD53BATLASRegisters.h"
#include "RD53BCMSRegisters.h"
#include "RD53BConstants.h"

using Ph2_HwDescription::RD53BConstants::ATLASRegisters;
using Ph2_HwDescription::RD53BConstants::CMSRegisters;
using Ph2_HwDescription::RD53BConstants::Register;
using Ph2_HwDescription::RD53BConstants::GetIMUX;
using Ph2_HwDescription::RD53BConstants::GetVMUX;

template<> const std::map<Register, uint16_t> RD53BTools::RD53BDACTest<Ph2_HwDescription::RD53BFlavor::ATLAS>::muxSett = {
    { ATLASRegisters::DAC_PREAMP_L_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_Preamp_Left"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_PREAMP_R_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_Preamp_Right"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_PREAMP_TL_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_Preamp_Top-Left"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_PREAMP_TR_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_Preamp_Top-Right"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_PREAMP_T_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_Preamp_Top"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_PREAMP_M_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_Preamp_Main_array"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_PRECOMP_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_PreComp"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_COMP_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_Comparator"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_VFF_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_Feedback"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_TH1_L_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_VTH1_Left"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_TH1_R_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_VTH1_Right"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_TH1_M_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_VTH1_Main_array"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_TH2_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_VTH2"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::DAC_LCC_DIFF, bits::pack<6,6>(GetIMUX().at("DIFF_FE_LCC"), GetVMUX().at("I_mux")) },
    { ATLASRegisters::VCAL_HIGH, bits::pack<6,6>(GetIMUX().at("high_Z"), GetVMUX().at("VCAL_HI")) },
    { ATLASRegisters::VCAL_MED, bits::pack<6,6>(GetIMUX().at("high_Z"), GetVMUX().at("VCAL_MED")) }
};

template<> const std::map<Register, uint16_t> RD53BTools::RD53BDACTest<Ph2_HwDescription::RD53BFlavor::CMS>::muxSett = {
    { CMSRegisters::DAC_PREAMP_L_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_Preamp_Left"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_PREAMP_R_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_Preamp_Right"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_PREAMP_TL_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_Preamp_Top-Left"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_PREAMP_TR_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_Preamp_Top-Right"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_PREAMP_T_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_Preamp_Top"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_PREAMP_M_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_Preamp_Main_array"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_FC_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_FC"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_KRUM_CURR_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_KrumCurr"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_REF_KRUM_LIN, bits::pack<6,6>(GetIMUX().at("high_Z"), GetVMUX().at("LIN_FE_Ref_KRUM")) },
    { CMSRegisters::DAC_COMP_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_Comparator"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_COMP_TA_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_CompsTAR"), GetVMUX().at("I_mux")) },
    { CMSRegisters::DAC_GDAC_L_LIN, bits::pack<6,6>(GetIMUX().at("high_Z"), GetVMUX().at("LIN_FE_GDAC_Main_array")) },
    { CMSRegisters::DAC_GDAC_R_LIN, bits::pack<6,6>(GetIMUX().at("high_Z"), GetVMUX().at("LIN_FE_GDAC_Right")) },
    { CMSRegisters::DAC_GDAC_M_LIN, bits::pack<6,6>(GetIMUX().at("high_Z"), GetVMUX().at("LIN_FE_GDAC_Main_array")) },
    { CMSRegisters::DAC_LDAC_LIN, bits::pack<6,6>(GetIMUX().at("LIN_FE_LDAC"), GetVMUX().at("I_mux")) },
    { CMSRegisters::VCAL_HIGH, bits::pack<6,6>(GetIMUX().at("high_Z"), GetVMUX().at("VCAL_HI")) },
    { CMSRegisters::VCAL_MED, bits::pack<6,6>(GetIMUX().at("high_Z"), GetVMUX().at("VCAL_MED")) }
};
