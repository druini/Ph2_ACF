#ifndef RD53VrefTrimming_H
#define RD53VrefTrimming_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include "../DQMUtils/RD53VrefTrimmingHistograms.h"


namespace RD53BTools {

template <class>
struct RD53VrefTrimming; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53VrefTrimming<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);

template <class Flavor>
struct RD53VrefTrimming : public RD53BTool<RD53VrefTrimming, Flavor> {
    using Base = RD53BTool<RD53VrefTrimming, Flavor>;
    using Base::Base;

    struct ChipResults {
        double vdddVoltage[16];
        double vddaVoltage[16];
    };

    auto run(Ph2_System::SystemController& system) const {
        ChipDataMap<ChipResults> results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(system.fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        LOG(INFO) << "[RD53VrefTrimming] exampleParam = " << Base::param("exampleParam"_s) << RESET;

        for_each_device<Chip>(system, [&] (Chip* chip) {
            for(int vTrim = 0; vTrim < 16; vTrim++){ //VS VDDD
                //Trim voltages
                chipInterface.WriteReg(chip, "TRIM_VREFD", vTrim);
                chipInterface.WriteReg(chip, "MonitorEnable", 1);
                chipInterface.WriteReg(chip, "VMonitor", 38);
                results[chip].vdddVoltage[vTrim] = dKeithley2410.getVoltage()*2;
				LOG(INFO) << BOLDMAGENTA << "Voltage: " << results[chip].vdddVoltage[vTrim] << RESET;
            }
            
            {
                size_t i_best = 0;
                double best = 999;
                for (int i = 0; i < 16; ++i) {
                    auto value = std::abs(results[chip].vdddVoltage[i] - 1.2);
                    if (value < best) {
                        best = value;
                        i_best = i;
                    }
                }
                chipInterface.WriteReg(chip, "TRIM_VREFD", i_best);
             }

            for(int vTrim = 0; vTrim < 16; vTrim++){ //VS VDDA
                //Trim voltages
                chipInterface.WriteReg(chip, "TRIM_VREFA", vTrim);
                chipInterface.WriteReg(chip, "MonitorEnable", 1);
                chipInterface.WriteReg(chip, "VMonitor", 34);
                results[chip].vddaVoltage[vTrim] = dKeithley2410.getVoltage()*2;
				LOG(INFO) << BOLDMAGENTA << "Voltage: " << results[chip].vddaVoltage[vTrim] << RESET;
            }

            {
                size_t i_best = 0;
                double best = 999;
                for (int i = 0; i < 16; ++i) {
                    auto value = std::abs(results[chip].vddaVoltage[i] - 1.2);
                    if (value < best) {
                        best = value;
                        i_best = i;
                    }
                }
                chipInterface.WriteReg(chip, "TRIM_VREFA", i_best);
            }

            
        });

        

        return results;
    }


    void draw(const ChipDataMap<ChipResults>& results) const {
        for (const auto& item : results) {
            VrefTrimmingHistograms* histos = new VrefTrimmingHistograms;
            histos->fillVT(
                item.second.vdddVoltage,
                item.second.vddaVoltage
            );
        }
    }


};

}

#endif


