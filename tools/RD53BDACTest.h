#ifndef RD53BDACTest_H
#define RD53BDACTest_H

#include <array>
#include <chrono>
#include <cmath>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "pugixml.hpp"

#include "RD53B.h"
#include "RD53BCommands.h"
#include "RD53BInterface.h"
#include "RD53BRegister.h"
#include "RD53BATLASRegisters.h"
#include "RD53BCMSRegisters.h"
#include "RD53BTool.h"

#include "TAxis.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TGraphErrors.h"

namespace RD53BTools {

template <class>
class RD53BDACTest;

template <class F>
const auto ToolParameters<RD53BDACTest<F>> = make_named_tuple(
    std::make_pair("voltmeterId"_s, std::string("dacCalib")),
    std::make_pair("voltmeterChannelId"_s, std::string("dacCalib")),
    std::make_pair("voltmeterSamples"_s, 2),
    std::make_pair("dacRegisters"_s, std::vector<std::string>()),
    std::make_pair("muxInput"_s, std::vector<std::string>()),
    std::make_pair("currentMuxResistance"_s, -1.0f),
    std::make_pair("vcalDelay"_s, 0.0f),
    std::make_pair("keithSpeed"_s, 0.01f),
    std::make_pair("fitThresholdV"_s, INFINITY)
);

template <class F>
class RD53BDACTest : public RD53BTool<RD53BDACTest, F> {
  public:
    struct DACData {
        std::vector<float> code, volt, voltErr;
        RD53BConstants::Register reg;
        bool isCurr;
    };

    using Base = RD53BTool<RD53BDACTest, F>;
    using Base::Base;
    using Base::param;
    using Results = ChipDataMap<std::vector<DACData>>;

    void init() {
        nCod = 0;
        nSamp = param("voltmeterSamples"_s);
        vmetId = param("voltmeterId"_s);
        vmetChId = param("voltmeterChannelId"_s);
        imuxR = param("currentMuxResistance"_s);

        for (const std::string& regName : param("dacRegisters"_s)) {
            const std::vector<RD53BConstants::RegisterField>& vReg = RD53B<F>::vRegs.at(regName);
            if(vReg.size() != 1 || vReg.front().offset != 0) {
                throw std::logic_error("Register " + regName + " is not supported by this routine.");
            }
            regs.push_back(vReg.front());
            if(regs.back().size * 2 > nCod) nCod = regs.back().size * 2;
            masks.push_back(~(0xffffu >> regs.back().size << regs.back().size));
        }
        const std::vector<std::string> &muxInputs = param("muxInput"_s);

        if (regs.size() != muxInputs.size()) {
            throw std::logic_error("The number of entries in the list of mux inputs"
                                   " does not match the number of DACs to calibrate.");
        }

        muxSelecs.resize(regs.size());
        isCurr.resize(regs.size());
        masks.resize(regs.size());

        bool currDet = false;
        for (size_t i = 0; i < regs.size(); ++i) {
            auto &imux = RD53B<F>::IMUX;
            auto &vmux = RD53B<F>::VMUX;

            uint16_t selec = 0;

            auto it = vmux.find(muxInputs[i]);
            if (it == vmux.end()) {
                auto it = imux.find(muxInputs[i]);
                if (it == imux.end()) {
                    throw std::logic_error(muxInputs[i] + "is not a valid name for a multiplexer input.");
                } else { // is a current DAC
                    isCurr[i] = true;
                    selec = (it->second << 6) | vmux.at("I_mux");
                    currDet = true;
                }
            } else { // is a voltage DAC
                selec = (imux.at("high_Z") << 6) | it->second;
                isCurr[i] = false;
            }

            muxSelecs[i] = selec;
        }

        if (currDet && imuxR < 0.0f) {
            throw std::logic_error("The value for the resistance at the output of the current"
                                   " multiplexer is invalid or was not set at all.");
        }
    }

    Results run(Ph2_System::SystemController& sysCtrl) {
        Results res;

        sysCtrl.fPowerSupplyClient->sendAndReceivePacket(
            "K2410:SetSpeed"
            ",PowerSupplyId:" + vmetId +
            ",ChannelId:" + vmetChId +
            ",IntegrationTime:" + std::to_string(param("keithSpeed"_s))
        );
        for_each_device<Chip>(sysCtrl, [this, &sysCtrl, &res] (Chip* chip) {
            this->calibChip(chip, sysCtrl, res[chip]);
        });

        return res;
    }

    void draw(const Results& results) const {
        TFile f(Base::getResultPath(".root").c_str(), "RECREATE");

        pugi::xml_document doc;
        pugi::xml_node root = doc.append_child("daccalib");

        TF1 line("line", "[offset]+[slope]*x");
        for (const std::pair<const ChipLocation, std::vector<DACData>>& chipRes : results) {
            const ChipLocation &chip = chipRes.first;

            f.mkdir(("board_" + std::to_string(chip.board_id)).c_str(), "", true)
            ->mkdir(("hybrid_" + std::to_string(chip.hybrid_id)).c_str(), "", true)
            ->mkdir(("chip_"+ std::to_string(chip.chip_id)).c_str(), "", true)->cd();

            pugi::xml_node dev = root.append_child("device");
            dev.append_attribute("board") = chip.board_id;
            dev.append_attribute("hybrid") = chip.hybrid_id;
            dev.append_attribute("chip") = chip.chip_id;

            for (const DACData& data : chipRes.second) {
                const RD53BConstants::Register& reg = data.reg;
                float m = (data.volt.back() - data.volt.front()) / (data.code.back() - data.code.front());
                float q = data.volt.front() - m * data.code.front();

                TGraph* g;
                if(data.voltErr.empty()) g = new TGraph(data.code.size(), data.code.data(), data.volt.data());
                else g = new TGraphErrors(data.code.size(), data.code.data(), data.volt.data(), nullptr, data.voltErr.data());
                g->SetNameTitle(reg.name.c_str());
                g->GetXaxis()->SetTitle("DAC code [DAC LSB]");
                if(data.isCurr)
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
                if(
                    (F::flavor == Flavor::CMS && (data.reg == CMSRegisters::VCAL_HIGH || data.reg == CMSRegisters::VCAL_MED)) ||
                    (F::flavor == Flavor::ATLAS && (data.reg == ATLASRegisters::VCAL_HIGH || data.reg == ATLASRegisters::VCAL_MED))
                ) {
                    g->Fit(&line, "Q");
                }
                else {
                    bool found = false;
                    float min;
                    float max;
                    float lim = param("fitThresholdV"_s) / (data.isCurr ? imuxR : 1);
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
                LOG(INFO) << "Chip: " << chip.board_id << "/" << chip.hybrid_id << "/" << chip.chip_id
                          << "; DAC: " << reg.name
                          << std::scientific
                          << "; slope = " << line.GetParameter("slope")
                          << "; offset = " << line.GetParameter("offset") << RESET;
                for(int i = 0; i < g->GetN(); ++i) {
                    g->SetPointY(i, g->GetPointY(i) - line.GetParameter("offset") - line.GetParameter("slope") * g->GetPointX(i));
                }
                g->SetNameTitle((reg.name + "_diff").c_str());
                if(data.isCurr) {
                    g->GetYaxis()->SetTitle("Residual (output current) [A]");
                }
                else {
                    g->GetYaxis()->SetTitle("Residual (output voltage) [V]");
                }
                g->Write();

                pugi::xml_node dac = dev.append_child("dac");
                dac.append_attribute("offset") = line.GetParameter("offset");
                dac.append_attribute("slope") = line.GetParameter("slope");
                dac.append_attribute("name") = reg.name.c_str();
                for (size_t i = 0; i < data.code.size(); ++i) {
                    pugi::xml_node pt = dac.append_child("point");
                    pt.append_attribute("x") = data.code[i];
                    pt.append_attribute("y") = data.volt[i];
                    if(!data.voltErr.empty()) pt.append_attribute("yuncert") = data.voltErr[i];
                }
                delete g;
            }
            doc.save_file(Base::getResultPath(".xml").c_str());
        }
    };

  private:
    size_t nCod, nSamp;
    float imuxR;
    std::string vmetId, vmetChId;
    std::vector<uint16_t> muxSelecs;
    std::vector<bool> isCurr;
    std::vector<RD53BConstants::RegisterField> regs;
    std::vector<uint16_t> masks;

    void calibChip(Chip* chip,
                   Ph2_System::SystemController& sysCtrl,
                   std::vector<DACData>& res) {
        auto &chipIface = *static_cast<RD53BInterface<F>*>(sysCtrl.fReadoutChipInterface);
        TCPClient &psIface = *sysCtrl.fPowerSupplyClient;

        std::ostringstream psReq("ReadVoltmeter,VoltmeterId:", std::ostringstream::ate);
        psReq << vmetId << ",ChannelId:" << vmetChId;
        if(nSamp > 1) psReq << ",Stats,N:" << nSamp;

        psIface.sendAndReceivePacket("VoltmeterSetRange"
                                      ",VoltmeterId:" + vmetId +
                                      ",ChannelId:" + vmetChId +
                                      ",Value:1.3");

        BitVector<uint16_t> cmdQ;
        std::vector<uint16_t> codes(regs.size());
        res.resize(regs.size());
        for(size_t i = 0; i < regs.size(); ++i) {
            res[i].reg = regs[i].reg;
            res[i].isCurr = isCurr[i];
        }
        for(size_t i = 0; i < nCod; ++i) {
            // setting all the DACs
            cmdQ.clear();
            for(size_t j = 0; j < codes.size(); ++j) {
                chipIface.template SerializeCommand<RD53BCmd::Sync>(chip, cmdQ);
                chipIface.template SerializeCommand<RD53BCmd::WrReg>(chip, cmdQ, regs[j].reg.address, codes[j]);
            }
            chipIface.SendCommandStream(chip, cmdQ);
            auto start = std::chrono::steady_clock::now();
            // all DACs set
            // measuring all DAC outputs
            for (size_t j = 0; j < regs.size(); ++j) {
                if(codes[j] > masks[j]) continue;
                chipIface.WriteChipReg(chip, "MonitorConfig", muxSelecs[j]);
                std::chrono::duration<float> delay = std::chrono::steady_clock::now() - start;
                if((regs[j].reg == F::Reg::VCAL_HIGH || regs[j].reg == F::Reg::VCAL_MED) && delay.count() > param("vcalDelay"_s)) {
                    std::this_thread::sleep_until(start + std::chrono::duration<float>(param("vcalDelay"_s)));
                }
                std::istringstream psResp(psIface.sendAndReceivePacket(psReq.str()));
                float a, b;
                psResp >> a;
                if(nSamp > 1) psResp >> b;
                DACData& data = res[j];
                if (psResp) { // a proper voltage was returned
                    if (isCurr[j]) {
                        a /= imuxR;
                        if(nSamp > 1) b /= imuxR * std::sqrt(nSamp);
                    }
                    data.code.push_back(codes[j]);
                    data.volt.push_back(a);
                    if(nSamp > 1) data.voltErr.push_back(b);
                }
            }
            // all DAC outputs measured for a single configuration
            for (size_t j = 0; j < regs.size(); ++j) {
                // note that if codes[j] == masks[j] == 0xffff the calibration for this chip is over
                if(codes[j] >= masks[j]) {
                    codes[j] = 0xffffu;
                    continue;
                }
                if (codes[j] == 0) codes[j] = 1;
                else if ((codes[j] ^ masks[j]) > codes[j]) codes[j] <<= 1;
                else codes[j] = (~((codes[j] ^ masks[j]) >> 1)) & masks[j];
            }

        }
        // all codes tested for all DACs
    }
};

}

#endif
