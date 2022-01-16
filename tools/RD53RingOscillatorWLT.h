#ifndef RD53RingOscillatorWLT_H
#define RD53RingOscillatorWLT_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"
#include "../DQMUtils/RD53RingOscillatorHistograms.h"
#include "../Utils/Bits/BitVector.hpp"

#include <cmath>
#include <fstream>
#include <string>

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
        int countEnableTimeBX;

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

            results[chip].countEnableTimeBX = nBX;

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
        double fitResults[42][2];
        TFile f(Base::getResultPath(".root").c_str(), "RECREATE");

        for(const std::pair<const ChipLocation, ChipResults>& item: results) {
            f.mkdir(("board_" + std::to_string(item.first.board_id)).c_str(), "", true)
                ->mkdir(("hybrid_" + std::to_string(item.first.hybrid_id)).c_str(), "", true)
                ->mkdir(("chip_" + std::to_string(item.first.chip_id)).c_str(), "", true)
                ->cd();

            RingOscillatorHistograms histos;
            histos.fillRO(
                item.second.trimOscCounts,
                item.second.trimOscFrequency,
                item.second.trimVoltage,
                item.second.n,
                fitResults
            );
            f.Write();
        }

        std::ofstream json(Base::getResultPath(".json"));
        const char* str = "{";
        json << std::scientific
             << "{\"chips\":[";
        for(const std::pair<const ChipLocation, ChipResults>& chip: results) {
            json << str;
            str = ",{";
            json << "\"board\":" << chip.first.board_id << ","
                 << "\"hybrid\":" << chip.first.hybrid_id << ","
                 << "\"id\":" << chip.first.chip_id << ","
                 << "\"count_en_t_BX\":" << chip.second.countEnableTimeBX << ","
                 << "\"oscillators\":[";
            for(int i = 0; i < 42; ++i) {
                /* oscillator data begins here */
                json << "{\"bank\":\"" << (i < 8 ? 'A' : 'B') << "\","
                     << "\"number\":" << (i < 8 ? i : i - 8) << ","
                     << "\"fitted_line\":{\"intercept\":" << fitResults[i][0] << ",\"slope\":" << fitResults[i][1] << "},"
                     << "\"points\":[";
                for(int j = 0; j < chip.second.n; ++j) {
                    json << "{\"VDDD\":" << chip.second.trimVoltage[j] << ","
                         << "\"frequency\":" << chip.second.trimOscFrequency[i][j] * 1e6 << "}";
                    if(j < chip.second.n - 1) json << ",";
                }
                json << "]}";
                /* oscillator data ends here */
                if(i < 42 - 1) json << ",";
            }
            json << "]}";
        }
        json << "]}";
    }


};

}

#endif


