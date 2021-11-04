#ifndef RD53BRINGOSCILLATOR_H
#define RD53BRINGOSCILLATOR_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#ifdef __USE_ROOT__
#include "../DQMUtils/RD53RingOscillatorHistograms.h"
#endif


namespace RD53BTools {

template <class>
struct RD53BRingOscillator; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BRingOscillator<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);

template <class Flavor>
struct RD53BRingOscillator : public RD53BTool<RD53BRingOscillator<Flavor>> {
    using Base = RD53BTool<RD53BRingOscillator>;
    using Base::Base;

    struct ChipResults {
        double gloPulse[128];
        double oscCounts[34][128];
        double oscFrequency[34][128];
        double trimOscCounts[34][16];
        double trimOscFrequency[34][16];
        double trimVoltage[16];
    };

    auto run(Ph2_System::SystemController& system) const {
        ChipDataMap<ChipResults> results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(system.fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        LOG(INFO) << "[RD53BRingOscillator] exampleParam = " << Base::param("exampleParam"_s) << RESET;

        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto& gloPulse = results[chip].gloPulse;
            auto& oscCounts = results[chip].oscCounts;

            for(int gPulse = 0; gPulse < 10; gPulse++){ //VS PULSE DURATION
                gloPulse[gPulse] = gPulse*10;
                for(int ringOsc = 0; ringOsc < 8; ringOsc++){
                    chipInterface.WriteReg(chip, "RingOscAEnable", 0b11111111);
                    chipInterface.WriteReg(chip, "RingOscAClear", 1);
                    chipInterface.WriteReg(chip, "RingOscAClear", 0);
                    chipInterface.WriteReg(chip, "RingOscARoute", ringOsc);
                    chipInterface.SendGlobalPulse(chip, {"StartRingOscillatorsA"},gPulse*10); //Start Oscillators 
                    oscCounts[ringOsc][gPulse] = chipInterface.ReadReg(chip, "RING_OSC_A_OUT") - 4096;
                    LOG(INFO) << BOLDMAGENTA << "Counts: " << oscCounts[ringOsc][gPulse] << RESET;
                    results[chip].oscFrequency[ringOsc][gPulse] = oscCounts[ringOsc][gPulse]/((2*(gloPulse[gPulse]+1))/40);
                }
            }

            auto& trimOscCounts = results[chip].trimOscCounts;
                
            for(int vTrim = 0; vTrim < 16; vTrim++){ //VS VDDD
                //Trim voltages
                chipInterface.WriteReg(chip, "VOLTAGE_TRIM", vTrim);
                chipInterface.WriteReg(chip, "MonitorEnable", 1);
                chipInterface.WriteReg(chip, "VMonitor", 38);
                results[chip].trimVoltage[vTrim] = dKeithley2410.getVoltage()*2;
                for(int ringOsc = 0; ringOsc < 34; ringOsc++){
                    //Set up oscillators
                    chipInterface.WriteReg(chip, "RingOscBEnBL", 1);
                    chipInterface.WriteReg(chip, "RingOscBEnBR", 1);
                    chipInterface.WriteReg(chip, "RingOscBEnCAPA", 1);
                    chipInterface.WriteReg(chip, "RingOscBEnFF", 1);
                    chipInterface.WriteReg(chip, "RingOscBEnLVT", 1);
                    chipInterface.WriteReg(chip, "RingOscBClear", 1);
                    chipInterface.WriteReg(chip, "RingOscBClear", 0);
                    chipInterface.WriteReg(chip, "RingOscBRoute", ringOsc);
                    chipInterface.SendGlobalPulse(chip, {"StartRingOscillatorsB"},50); //Start Oscillators 
                    trimOscCounts[ringOsc][vTrim] = chipInterface.ReadReg(chip, "RING_OSC_B_OUT") - 4096;
                    LOG(INFO) << BOLDMAGENTA << "Counts: " << trimOscCounts[ringOsc][vTrim] << RESET;
                    results[chip].trimOscFrequency[ringOsc][vTrim] = trimOscCounts[ringOsc][vTrim]/((2*51)/40);
                }
            }
        });

        return results;
    }

#ifdef __USE_ROOT__

    void draw(ChipDataMap<ChipResults>& results) const {
        for (const auto& item : results) {
            RingOscillatorHistograms* histos;
            histos->fillRO(
                item.second.gloPulse, 
                item.second.oscCounts, 
                item.second.oscFrequency, 
                item.second.trimOscCounts, 
                item.second.trimOscFrequency, 
                item.second.trimVoltage
            );
        }
    }

#endif

};

}

#endif


