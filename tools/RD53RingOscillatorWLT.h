#ifndef RD53RingOscillatorWLT_H
#define RD53RingOscillatorWLT_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"
#include "../DQMUtils/RD53RingOscillatorHistograms.h"
#include "../Utils/Bits/BitVector.hpp"

#include <cmath>
#include <string>

#include "pugixml.hpp"

#include "TFile.h"

namespace RD53BTools {
template <class>
struct RD53RingOscillatorWLT;

template <class Flavor>
const auto ToolParameters<RD53RingOscillatorWLT<Flavor>> = make_named_tuple(
    std::make_pair("vmeter"_s, std::string("TestKeithley")),
    std::make_pair("vmeterCH"_s, std::string("Front")),
    std::make_pair("gateWidth"_s, uint16_t{100}),
    std::make_pair("minVDDD"_s, 0),
    std::make_pair("maxVDDD"_s, 15),
    std::make_pair("abortVDDD"_s, INFINITY)
);

template <class Flavor>
struct RD53RingOscillatorWLT : public RD53BTool<RD53RingOscillatorWLT, Flavor> {
    using Base = RD53BTool<RD53RingOscillatorWLT, Flavor>;
    using Base::Base;
    using Base::param;


public:
    struct ChipResults {
        double trimOscCounts[42][16];
        double trimOscFrequency[42][16];
        double trimVoltage[16];
        int n;

        ChipResults() : n{0} {}
    };

private:
    uint16_t pulseRoute, pulseWidth, nBX;

public:
    void init() {
        pulseRoute = 1u << RD53B<Flavor>::GlobalPulseRoutes.at("StartRingOscillatorsA") | 1u << RD53B<Flavor>::GlobalPulseRoutes.at("StartRingOscillatorsB");
        pulseWidth = param("gateWidth"_s) / Flavor::globalPulseUnit;
        if(pulseWidth == 0) pulseWidth = 1;
        nBX = pulseWidth * Flavor::globalPulseUnit;
        if(param("gateWidth"_s) % Flavor::globalPulseUnit != 0) {
            LOG(WARNING) << "Requested gate width not compatible with this chip" << RESET;
        }
    }

    auto run(Ph2_System::SystemController& system) const {
        ChipDataMap<ChipResults> results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        system.fPowerSupplyClient->sendAndReceivePacket("K2410:SetSpeed,PowerSupplyId:" + param("vmeter"_s) + ",ChannelId:" + param("vmeterCH"_s) + ",IntegrationTime:1");
        system.fPowerSupplyClient->sendAndReceivePacket("VoltmeterSetRange,VoltmeterId:" + param("vmeter"_s) + ",ChannelId:" + param("vmeterCH"_s) + ",Value:1.3");

        for_each_device<Chip>(system, [&](DeviceChain devs) {
            RD53B<Flavor>* chip = static_cast<RD53B<Flavor>*>(devs.chip);

            auto& trimOscCounts = results[chip].trimOscCounts;
            auto& trimOscFrequency = results[chip].trimOscFrequency;
            auto& trimVoltage = results[chip].trimVoltage;
            int& n = results[chip].n;

            uint16_t vTrimOld = chipInterface.ReadReg(chip, Flavor::Reg::VOLTAGE_TRIM);

            BitVector<uint16_t> initQ, sweepQ;

            /* initialization */

            uint16_t monConf = chipInterface.ReadReg(chip, Flavor::Reg::MonitorConfig);
            monConf = (monConf & 0xffc0) | RD53B<Flavor>::VMUX.at("VDDD_half");

            chipInterface.template SerializeCommand<RD53BCmd::WrReg>(chip, initQ, Flavor::Reg::MonitorConfig.address, monConf);
            chipInterface.template SerializeCommand<RD53BCmd::WrReg>(chip, initQ, Flavor::Reg::GlobalPulseWidth.address, pulseWidth);
            chipInterface.template SerializeCommand<RD53BCmd::WrReg>(chip, initQ, Flavor::Reg::GlobalPulseConf.address, pulseRoute);

            chipInterface.SendCommandStream(chip, initQ);

            /* command queue for voltage sweep */
            chipInterface.template SerializeCommand<RD53BCmd::WrReg>(chip, sweepQ, Flavor::Reg::RingOscConfig.address, uint16_t{0xffff}); // clear
            chipInterface.template SerializeCommand<RD53BCmd::WrReg>(chip, sweepQ, Flavor::Reg::RingOscConfig.address, uint16_t{0x3eff}); // hold
            chipInterface.template SerializeCommand<RD53BCmd::GlobalPulse>(chip, sweepQ);

            for(int vTrim = param("minVDDD"_s); vTrim <= param("maxVDDD"_s); vTrim++) {
                //Trim voltages
                chipInterface.WriteReg(chip, "TRIM_VREFD", vTrim);
                double v = 2 * std::stod(system.fPowerSupplyClient->sendAndReceivePacket(
                    "ReadVoltmeter"
                    ",VoltmeterId:" + param("vmeter"_s) +
                    ",ChannelId:" + param("vmeterCH"_s))
                );
                chipInterface.SendCommandStream(chip, sweepQ);

                LOG(INFO) << BOLDBLUE << "VDDD: " << BOLDYELLOW << v << " V" << RESET;
                if(v > param("abortVDDD"_s)) break;

                trimVoltage[n] = v;

                for(int ringOsc = 0; ringOsc < 8; ringOsc++){
                    chipInterface.WriteReg(chip, "RingOscARoute", ringOsc);
                    trimOscCounts[ringOsc][n] = chipInterface.ReadReg(chip, Flavor::Reg::RING_OSC_A_OUT) & 0xfff;
                    trimOscFrequency[ringOsc][n] = trimOscCounts[ringOsc][n] * 40 / nBX;
                }
                for(int ringOsc = 0; ringOsc < 34; ringOsc++){
                    chipInterface.WriteReg(chip, "RingOscBRoute", ringOsc);
                    trimOscCounts[ringOsc + 8][n] = chipInterface.ReadReg(chip, Flavor::Reg::RING_OSC_B_OUT) & 0xfff;
                    trimOscFrequency[ringOsc + 8][n] = trimOscCounts[ringOsc + 8][n] * 40 / nBX;
                }

                ++n;
            }

            chipInterface.WriteReg(chip, Flavor::Reg::VOLTAGE_TRIM, vTrimOld);
        });

        return results;
    }

    void draw(const ChipDataMap<ChipResults>& results) const {
        TFile f(Base::getResultPath(".root").c_str(), "RECREATE");

        pugi::xml_document doc;
        pugi::xml_node root = doc.append_child("ringosc");

        for(const std::pair<const ChipLocation, ChipResults>& item: results) {
            const ChipLocation& loc = item.first;
            const ChipResults&  res = item.second;

            f.mkdir(("board_" + std::to_string(loc.board_id)).c_str(), "", true)
                ->mkdir(("hybrid_" + std::to_string(loc.hybrid_id)).c_str(), "", true)
                ->mkdir(("chip_" + std::to_string(loc.chip_id)).c_str(), "", true)
                ->cd();

            RingOscillatorHistograms histos;
            histos.fillRO(
                item.second.trimOscCounts,
                item.second.trimOscFrequency,
                item.second.trimVoltage,
                item.second.n
            );
            f.Write();

            pugi::xml_node dev = root.append_child("chip");
            dev.append_attribute("id") = loc.chip_id;
            dev.append_attribute("board") = loc.board_id;
            dev.append_attribute("hybrid") = loc.hybrid_id;

            for(int i = 0; i < 8; ++i) {
                pugi::xml_node osc = root.append_child("oscillator");
                osc.append_attribute("bank") = "A";
                osc.append_attribute("number") = i;
                for(int j = 0; j < res.n; ++j) {
                    pugi::xml_node p = osc.append_child("point");
                    p.append_attribute("vddd") = res.trimVoltage[j];
                    p.append_attribute("freq") = res.trimOscFrequency[i][j];
                }
            }

            for(int i = 0; i < 34; ++i) {
                pugi::xml_node osc = root.append_child("oscillator");
                osc.append_attribute("bank") = "B";
                osc.append_attribute("number") = i;
                for(int j = 0; j < res.n; ++j) {
                    pugi::xml_node p = osc.append_child("point");
                    p.append_attribute("vddd") = res.trimVoltage[j];
                    p.append_attribute("freq") = res.trimOscFrequency[8 + i][j];
                }
            }
        }
        doc.save_file(Base::getResultPath(".xml").c_str());
    }


};

}

#endif


