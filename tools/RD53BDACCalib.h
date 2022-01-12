#ifndef RD53BDACCalib_H
#define RD53BDACCalib_H

#include <array>
#include <cmath>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
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

namespace RD53BTools {

template <class>
class RD53BDACCalib;

template <class F>
const auto ToolParameters<RD53BDACCalib<F>> = make_named_tuple(
    std::make_pair("dacCodes"_s, 3),
    std::make_pair("voltmeterId"_s, std::string("dacCalib")),
    std::make_pair("voltmeterChannelId"_s, std::string("dacCalib")),
    std::make_pair("voltmeterSamples"_s, 2),
    std::make_pair("dacRegisters"_s, std::vector<std::string>()),
    std::make_pair("muxInput"_s, std::vector<std::string>()),
    std::make_pair("dacMin"_s, std::vector<uint16_t>()),
    std::make_pair("dacMax"_s, std::vector<uint16_t>()),
    std::make_pair("currentMuxResistance"_s, -1.0f),
    std::make_pair("fitThresholdV"_s, INFINITY)
);

template <class F>
class RD53BDACCalib : public RD53BTool<RD53BDACCalib, F> {
  public:
    struct DACData { std::vector<float> code, volt, voltErr; };

    using Base = RD53BTool<RD53BDACCalib, F>;
    using Base::Base;
    using Base::param;
    using Results = ChipDataMap<std::map<RD53BConstants::Register, DACData>>;

    void init() {
        nCod = param("dacCodes"_s);
        nSamp = param("voltmeterSamples"_s);
        vmetId = param("voltmeterId"_s);
        vmetChId = param("voltmeterChannelId"_s);
        min = param("dacMin"_s);
        max = param("dacMax"_s);
        imuxR = param("currentMuxResistance"_s);

        for (const std::string& regName : param("dacRegisters"_s)) {
            regs.push_back(RD53B<F>::vRegs.at(regName)[0].reg);
        }

        const std::vector<std::string> &muxInputs = param("muxInput"_s);

        if (regs.size() != muxInputs.size()) {
            throw std::logic_error("The number of entries in the list of mux inputs"
                                   " does not match the number of DACs to calibrate.");
        }

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
                    isCurr[regs[i]] = true;
                    selec = (it->second << 6) | vmux.at("I_mux");
                    currDet = true;
                }
            } else { // is a voltage DAC
                selec = (imux.at("high_Z") << 6) | it->second;
                isCurr[regs[i]] = false;
            }

            muxSelecs[regs[i]] = selec;
        }

        if (currDet && imuxR < 0.0f) {
            throw std::logic_error("The value for the resistance at the output of the current"
                                   " multiplexer is invalid or was not set at all.");
        }

        if (regs.size() != min.size()) {
            throw std::logic_error("The number of entries in the list of the minimum values"
                                   " for the DAC codes does not match that of DACs to calibrate.");
        }

        if (regs.size() != max.size()) {
            throw std::logic_error("The number of entries in the list of the maximum values "
                                   " for the DAC codes does not match that of DACs to calibrate.");
        }
    }

    Results run(Ph2_System::SystemController& sysCtrl) {
        Results res;

        sysCtrl.fPowerSupplyClient->sendAndReceivePacket(
            "K2410:SetSpeed"
            ",PowerSupplyId:" + vmetId +
            ",ChannelId:" + vmetChId +
            ",IntegrationTime:0.01"
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
        for (const std::pair<const ChipLocation, std::map<RD53BConstants::Register, DACData>>& chipRes : results) {
            const ChipLocation &chip = chipRes.first;

            f.mkdir(("board_" + std::to_string(chip.board_id)).c_str(), "", true)
            ->mkdir(("hybrid_" + std::to_string(chip.hybrid_id)).c_str(), "", true)
            ->mkdir(("chip_"+ std::to_string(chip.chip_id)).c_str(), "", true)->cd();

            pugi::xml_node dev = root.append_child("device");
            dev.append_attribute("board") = chip.board_id;
            dev.append_attribute("hybrid") = chip.hybrid_id;
            dev.append_attribute("chip") = chip.chip_id;

            for (const std::pair<const RD53BConstants::Register, DACData>& p : chipRes.second) {
                const RD53BConstants::Register& reg = p.first;
                const DACData& data = p.second;
                float m = (data.volt.back() - data.volt.front()) / (data.code.back() - data.code.front());
                float q = data.volt.front() - m * data.code.front();

                TGraphErrors g(data.code.size(), data.code.data(), data.volt.data(), nullptr, data.voltErr.data());
                g.SetNameTitle(reg.name.c_str());
                g.GetXaxis()->SetTitle("DAC code [DAC LSB]");
                if(isCurr.at(reg))
                {
                    g.GetYaxis()->SetTitle("Output current [A]");
                }
                else
                {
                    g.GetYaxis()->SetTitle("Output voltage [V]");
                }
                line.SetParameter("offset", q);
                line.SetParameter("slope", m);
                using namespace Ph2_HwDescription::RD53BFlavor;
                using namespace Ph2_HwDescription::RD53BConstants;
                if(
                    (F::flavor == Flavor::CMS && (p.first == CMSRegisters::VCAL_HIGH || p.first == CMSRegisters::VCAL_MED)) ||
                    (F::flavor == Flavor::ATLAS && (p.first == ATLASRegisters::VCAL_HIGH || p.first == ATLASRegisters::VCAL_MED))
                ) {
                    g.Fit(&line, "Q");
                }
                else {
                    bool found = false;
                    float min;
                    float max;
                    float lim = param("fitThresholdV"_s) / (isCurr.at(p.first) ? imuxR : 1);
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
                        g.Fit(&line, "Q", "", min, max);
                    }
                }
                g.Write();
                LOG(INFO) << "Chip: " << chip.board_id << "/" << chip.hybrid_id << "/" << chip.chip_id
                          << "; DAC: " << reg.name
                          << std::scientific
                          << "; slope = " << line.GetParameter("slope")
                          << "; offset = " << line.GetParameter("offset") << RESET;

                pugi::xml_node dac = dev.append_child("dac");
                dac.append_attribute("offset") = line.GetParameter("offset");
                dac.append_attribute("slope") = line.GetParameter("slope");
                dac.append_attribute("name") = reg.name.c_str();
                for (size_t i = 0; i < data.code.size(); ++i) {
                    pugi::xml_node pt = dac.append_child("point");
                    pt.append_attribute("x") = data.code[i];
                    pt.append_attribute("y") = data.volt[i];
                    pt.append_attribute("yuncert") = data.voltErr[i];
                }
            }
            doc.save_file(Base::getResultPath(".xml").c_str());
        }
    };

  private:
    size_t nCod, nSamp;
    float imuxR;
    std::string vmetId, vmetChId;
    std::map<RD53BConstants::Register, uint16_t> muxSelecs;
    std::map<RD53BConstants::Register, bool> isCurr;
    std::vector<RD53BConstants::Register> regs;
    std::vector<uint16_t> min, max;

    void calibChip(Chip* chip,
                   Ph2_System::SystemController& sysCtrl,
                   std::map<RD53BConstants::Register, DACData>& res) {
        auto &chipIface = *static_cast<RD53BInterface<F>*>(sysCtrl.fReadoutChipInterface);
        TCPClient &psIface = *sysCtrl.fPowerSupplyClient;

        std::ostringstream psReq("ReadVoltmeter,VoltmeterId:", std::ostringstream::ate);
        psReq << vmetId << ",ChannelId:" << vmetChId << ",Stats,N:" << nSamp;

        psIface.sendAndReceivePacket("VoltmeterSetRange"
                                      ",VoltmeterId:" + vmetId +
                                      ",ChannelId:" + vmetChId +
                                      ",Value:1.3");

        BitVector<uint16_t> cmdQ;
        std::vector<uint16_t> codes(regs.size());
        for (size_t i = 0; i < nCod; ++i) {
            // setting all the DACs
            cmdQ.clear();
            codes.clear();
            for (size_t j = 0; j < regs.size(); ++j) {
                uint16_t code = min[j] + std::round(static_cast<double>(i) / (nCod-1) * (max[j]-min[j]));
                chipIface.template SerializeCommand<RD53BCmd::Sync>(chip, cmdQ);
                chipIface.template SerializeCommand<RD53BCmd::WrReg>(chip, cmdQ, regs[j].address, code);
                codes.push_back(code);
            }
            chipIface.SendCommandStream(chip, cmdQ);
            // all DACs set
            // measuring all DAC outputs
            for (size_t j = 0; j < regs.size(); ++j) {
                chipIface.WriteChipReg(chip, "MonitorConfig", muxSelecs[regs[j]]);
                std::istringstream psResp(psIface.sendAndReceivePacket(psReq.str()));
                float a, b;
                psResp >> a >> b;
                DACData& data = res[regs[j]];
                if (psResp) { // a proper voltage was returned
                    if (isCurr[regs[j]]) {
                        a /= imuxR;
                        b /= imuxR;
                    }
                    data.code.push_back(codes[j]);
                    data.volt.push_back(a);
                    data.voltErr.push_back(b);
                }
            }
            // all DAC outputs measured for a single configuration
        }
        // all codes tested for all DACs
    }
};

}

#endif
