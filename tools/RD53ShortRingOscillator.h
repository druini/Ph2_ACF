#ifndef RD53ShortRingOscillator_H
#define RD53ShortRingOscillator_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include "../DQMUtils/RD53ShortRingOscillatorHistograms.h"


namespace RD53BTools {

template <class>
struct RD53ShortRingOscillator; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53ShortRingOscillator<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);

template <class Flavor>
struct RD53ShortRingOscillator : public RD53BTool<RD53ShortRingOscillator, Flavor> {
    using Base = RD53BTool<RD53ShortRingOscillator, Flavor>;
    using Base::Base;

    struct ChipResults {
        double trimOscCounts[42];
        //double trimOscFrequency[42];
    };

    auto run(Ph2_System::SystemController& system) const {
        ChipDataMap<ChipResults> results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(system.fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        LOG(INFO) << "[RD53ShortRingOscillator] exampleParam = " << Base::param("exampleParam"_s) << RESET;

        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto& trimOscCounts = results[chip].trimOscCounts;
			
			for(int ringOsc = 0; ringOsc < 8; ringOsc++){
				//Set up oscillators
				chipInterface.WriteReg(chip, "RingOscAEnable", 0b11111111);
				chipInterface.WriteReg(chip, "RingOscAClear", 1);
				chipInterface.WriteReg(chip, "RingOscAClear", 0);
				chipInterface.WriteReg(chip, "RingOscARoute", ringOsc);
				chipInterface.SendGlobalPulse(chip, {"StartRingOscillatorsA"},50); //Start Oscillators 
				trimOscCounts[ringOsc] = chipInterface.ReadReg(chip, "RING_OSC_A_OUT") - 4096;
				LOG(INFO) << BOLDMAGENTA << "Counts: " << trimOscCounts[ringOsc] << RESET;
				//results[chip].trimOscFrequency[ringOsc] = trimOscCounts[ringOsc]/((2*51)/40);
			}
			chipInterface.WriteReg(chip, "RingOscAEnable", 0b0000000);
			chipInterface.SendGlobalPulse(chip, {"StartRingOscillatorsA"},50); //Stop Oscillators 
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
				trimOscCounts[ringOsc + 8] = chipInterface.ReadReg(chip, "RING_OSC_B_OUT") - 4096;
				//LOG(INFO) << BOLDMAGENTA << "Counts: " << trimOscCounts[ringOsc + 8] << RESET;
				//results[chip].trimOscFrequency[ringOsc + 8] = trimOscCounts[ringOsc + 8]/((2*51)/40);
			}
			chipInterface.WriteReg(chip, "RingOscBEnBL", 0);
			chipInterface.WriteReg(chip, "RingOscBEnBR", 0);
			chipInterface.WriteReg(chip, "RingOscBEnCAPA", 0);
			chipInterface.WriteReg(chip, "RingOscBEnFF", 0);
			chipInterface.WriteReg(chip, "RingOscBEnLVT", 0);
			chipInterface.SendGlobalPulse(chip, {"StartRingOscillatorsB"},50); //Stop Oscillators 
        });

        return results;
    }


    void draw(const ChipDataMap<ChipResults>& results) const {
        for (const auto& item : results) {
            ShortRingOscillatorHistograms* histos = new ShortRingOscillatorHistograms;
            histos->fillSRO(
                item.second.trimOscCounts
                //item.second.trimOscFrequency, 
            );
        }
    }

};

}

#endif


