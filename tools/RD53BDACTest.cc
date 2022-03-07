#include "RD53BDACTest.h"

#include <array>
#include <map>
#include <set>
#include <string>

#include "RD53BATLASRegisters.h"
#include "RD53BCMSRegisters.h"
#include "RD53BConstants.h"

using Ph2_HwDescription::RD53BConstants::Register;
using Ph2_HwDescription::RD53BFlavor::ATLAS;
using Ph2_HwDescription::RD53BFlavor::CMS;
using RD53BTools::RD53BDACTest;

template<class F> void RD53BDACTest<F>::draw(const Results& results) const {
    std::ofstream json(Base::getResultPath(".json"));
    TFile f(Base::getResultPath(".root").c_str(), "RECREATE");

    TF1 line("line", "[offset]+[slope]*x");

    const char* str = "{";
    json << std::scientific << "{\"chips\":[";
    for (const std::pair<const ChipLocation, std::map<Register, DACData>>& chipRes : results) {
        const ChipLocation &chip = chipRes.first;

        json << str;
        str = ",{";

        json << "\"board\":" << chip.board_id << ","
             << "\"hybrid\":" << chip.hybrid_id << ","
             << "\"id\":" << chip.chip_id << ",";

        f.mkdir(("board_" + std::to_string(chip.board_id)).c_str(), "", true)
        ->mkdir(("hybrid_" + std::to_string(chip.hybrid_id)).c_str(), "", true)
        ->mkdir(("chip_"+ std::to_string(chip.chip_id)).c_str(), "", true)->cd();

        const char* str2 = "{";
        json << "\"DACs\":[";
        for (const std::pair<const Register, DACData>& p : chipRes.second) {
            const Register& reg = p.first;
            const DACData& data = p.second;
            
            float m = (data.volt.back() - data.volt.front()) / (data.code.back() - data.code.front());
            float q = data.volt.front() - m * data.code.front();

            TGraph* g;
            if(data.voltErr.empty()) g = new TGraph(data.code.size(), data.code.data(), data.volt.data());
            else g = new TGraphErrors(data.code.size(), data.code.data(), data.volt.data(), nullptr, data.voltErr.data());
            g->SetNameTitle(reg.name.c_str());
            g->GetXaxis()->SetTitle("DAC code [DAC LSB]");
            if(isCurr(reg))
            {
                g->GetYaxis()->SetTitle("Output current [A]");
            }
            else
            {
                g->GetYaxis()->SetTitle("Output voltage [V]");
            }
            line.SetParameter("offset", q);
            line.SetParameter("slope", m);
            using namespace Ph2_HwDescription::RD53BFlavor;
            using namespace Ph2_HwDescription::RD53BConstants;
            if(p.first == F::Reg::VCAL_HIGH || p.first == F::Reg::VCAL_MED) {
                g->Fit(&line, "Q");
            }
            else {
                bool found = false;
                float min;
                float max;
                float lim = param("fitThresholdV"_s) / (isCurr(reg) ? imuxR : 1);
                for(std::vector<float>::size_type i = 0; i < data.code.size(); ++i) {
                    if(data.volt[i] < lim) {
                        if(!found) {
                            min = data.code[i];
                            max = data.code[i];
                            found = true;
                        }
                        else {
                            if(data.code[i] < min) min = data.code[i];
                            if(data.code[i] > max) max = data.code[i];
                        }
                    }
                }
                if(found) {
                    g->Fit(&line, "Q", "", min, max);
                }
            }
            g->Write();

            for(int i = 0; i < g->GetN(); ++i) {
                g->SetPointY(i, g->GetPointY(i) - line.GetParameter("offset") - line.GetParameter("slope") * g->GetPointX(i));
            }
            g->SetNameTitle((reg.name + "_diff").c_str());
            if(isCurr(reg)) {
                g->GetYaxis()->SetTitle("Residual (output current) [A]");
            }
            else {
                g->GetYaxis()->SetTitle("Residual (output voltage) [V]");
            }
            g->Write();
            delete g;

            json << str2;
            str2 = ",{";
            json << "\"reg\":\"" << reg.name << "\","
                 << "\"fitted_line\":{\"intercept\":" << line.GetParameter("offset") << ",\"slope\":" << line.GetParameter("slope") << "},";
            if(isCurr(reg)) json << "\"currents\":{";
            else json << "\"voltages\":{";
            for (size_t i = 0; i < data.code.size(); ++i) {
                json << "\"" << static_cast<uint16_t>(data.code[i]) << "\":" << data.volt[i];
                if(i < data.code.size() - 1) json << ",";
            }
            json << "}";
            if(!data.voltErr.empty()) {
                json << ",\"errors\":[";
                for (size_t i = 0; i < data.code.size(); ++i) {
                    json << data.voltErr[i];
                    if(i < data.code.size() - 1) json << ",";
                }
                json << "]";
            }
            json << "}";
        }
        json << "]}";
    }
    json << "]}";
}

template<> const std::map<std::string, std::array<uint16_t, 2>> RD53BDACTest<ATLAS>::lim = {};

template<> const std::map<std::string, std::array<uint16_t, 2>> RD53BDACTest<CMS>::lim = {
    { "FC_BIAS_LIN", { 10, 70 } },
    { "Vthreshold_LIN", { 300, 900} },
    { "COMP_LIN", { 70, 250 } },
    { "COMP_STAR_LIN", { 50, 300 } },
    { "LDAC_LIN", { 80, 500 } },
    { "KRUM_CURR_LIN", { 5, 200 } },
    { "PA_IN_BIAS_LIN", { 100, 700 } },
    { "REF_KRUM_LIN", { 300, 450 } }
};

template<> 
const std::map<std::reference_wrapper<const Register>, std::string> RD53BDACTest<ATLAS>::dacSett = {
    { ATLAS::Reg::VCAL_HIGH, std::string("VCAL") },
    { ATLAS::Reg::VCAL_MED, std::string("VCAL") }
};

template<> const std::map<std::reference_wrapper<const Register>, std::string> RD53BDACTest<CMS>::dacSett = {
    { CMS::Reg::DAC_PREAMP_L_LIN, "PA_IN_BIAS_LIN" },
    { CMS::Reg::DAC_PREAMP_R_LIN, "PA_IN_BIAS_LIN" },
    { CMS::Reg::DAC_PREAMP_TL_LIN, "PA_IN_BIAS_LIN" },
    { CMS::Reg::DAC_PREAMP_TR_LIN, "PA_IN_BIAS_LIN" },
    { CMS::Reg::DAC_PREAMP_T_LIN, "PA_IN_BIAS_LIN" },
    { CMS::Reg::DAC_PREAMP_M_LIN, "PA_IN_BIAS_LIN" },
    { CMS::Reg::DAC_FC_LIN, "FC_BIAS_LIN" },
    { CMS::Reg::DAC_KRUM_CURR_LIN, "KRUM_CURR_LIN" },
    { CMS::Reg::DAC_REF_KRUM_LIN, "REF_KRUM_LIN" },
    { CMS::Reg::DAC_COMP_LIN, "COMP_LIN" },
    { CMS::Reg::DAC_COMP_TA_LIN, "COMP_STAR_LIN" },
    { CMS::Reg::DAC_GDAC_L_LIN, "Vthreshold_LIN" },
    { CMS::Reg::DAC_GDAC_R_LIN, "Vthreshold_LIN" },
    { CMS::Reg::DAC_GDAC_M_LIN, "Vthreshold_LIN" },
    { CMS::Reg::DAC_LDAC_LIN, "Vthreshold_LIN" },
    { CMS::Reg::VCAL_HIGH, "VCAL" },
    { CMS::Reg::VCAL_MED, "VCAL" }
};

template<> const std::map<std::reference_wrapper<const Register>, uint16_t> RD53BDACTest<ATLAS>::muxSett = {
    { ATLAS::Reg::DAC_PREAMP_L_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_PREAMP_L, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_PREAMP_R_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_PREAMP_R, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_PREAMP_TL_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_PREAMP_TL, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_PREAMP_TR_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_PREAMP_TR, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_PREAMP_T_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_PREAMP_T, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_PREAMP_M_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_PREAMP_M, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_PRECOMP_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_PRECOMPARATOR, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_COMP_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_COMPARATOR, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_VFF_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_FEEDBACK, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_TH1_L_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_VTH1_L, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_TH1_R_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_VTH1_R, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_TH1_M_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_VTH1_M, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_TH2_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::I_VTH2, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::DAC_LCC_DIFF, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::LEAKAGE_CURRENT_COMP, (uint8_t)RD53B<ATLAS>::VMux::IMUX_OUT) },
    { ATLAS::Reg::VCAL_HIGH, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::HIGH_Z, (uint8_t)RD53B<ATLAS>::VMux::VCAL_HIGH) },
    { ATLAS::Reg::VCAL_MED, bits::pack<6, 6>((uint8_t)RD53B<ATLAS>::IMux::HIGH_Z, (uint8_t)RD53B<ATLAS>::VMux::VCAL_MED) }
};

template<> const std::map<std::reference_wrapper<const Register>, uint16_t> RD53BDACTest<CMS>::muxSett = {
    { CMS::Reg::DAC_PREAMP_L_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_PREAMP_L, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_PREAMP_R_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_PREAMP_R, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_PREAMP_TL_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_PREAMP_TL, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_PREAMP_TR_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_PREAMP_TR, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_PREAMP_T_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_PREAMP_T, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_PREAMP_M_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_PREAMP_M, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_FC_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_FC, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_KRUM_CURR_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_KRUMMENAKER, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_REF_KRUM_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::HIGH_Z, (uint8_t)RD53B<CMS>::VMux::VREF_KRUM) },
    { CMS::Reg::DAC_COMP_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_COMPARATOR, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_COMP_TA_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_COMPARATOR_STAR, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::DAC_GDAC_L_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::HIGH_Z, (uint8_t)RD53B<CMS>::VMux::GDAC_L) },
    { CMS::Reg::DAC_GDAC_R_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::HIGH_Z, (uint8_t)RD53B<CMS>::VMux::GDAC_R) },
    { CMS::Reg::DAC_GDAC_M_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::HIGH_Z, (uint8_t)RD53B<CMS>::VMux::GDAC_M) },
    { CMS::Reg::DAC_LDAC_LIN, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::I_LDAC, (uint8_t)RD53B<CMS>::VMux::IMUX_OUT) },
    { CMS::Reg::VCAL_HIGH, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::HIGH_Z, (uint8_t)RD53B<CMS>::VMux::VCAL_HIGH) },
    { CMS::Reg::VCAL_MED, bits::pack<6, 6>((uint8_t)RD53B<CMS>::IMux::HIGH_Z, (uint8_t)RD53B<CMS>::VMux::VCAL_MED) }
};

template class RD53BTools::RD53BDACTest<ATLAS>;
template class RD53BTools::RD53BDACTest<CMS>;
