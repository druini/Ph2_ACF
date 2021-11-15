#ifndef RD53MuxScan_H
#define RD53MuxScan_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#ifdef __USE_ROOT__
#include "../DQMUtils/RD53MuxScanHistograms.h"
#endif


namespace RD53BTools {

template <class>
struct RD53MuxScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53MuxScan<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);

template <class Flavor>
struct RD53MuxScan : public RD53BTool<RD53MuxScan<Flavor>> {
    using Base = RD53BTool<RD53MuxScan>;
    using Base::Base;

    struct ChipResults {
        double VMUXvolt[41];
        double IMUXvolt[33];
    };

    auto run(Ph2_System::SystemController& system) const {
        ChipDataMap<ChipResults> results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(system.fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        LOG(INFO) << "[RD53MuxScan] exampleParam = " << Base::param("exampleParam"_s) << RESET;

        for_each_device<Chip>(system, [&] (Chip* chip) {
		
			for(int VMUXcode=0;VMUXcode<40;VMUXcode++){
				//Set Mux output
				if (VMUXcode == 1){
					for(int IMUXcode=0;IMUXcode<32;IMUXcode++){
						chipInterface.WriteReg(chip, "MonitorEnable", 1); //Choose MUX entry
						chipInterface.WriteReg(chip, "VMonitor", 0b000001);
						chipInterface.WriteReg(chip, "IMonitor", IMUXcode);
						results[chip].IMUXvolt[IMUXcode] = dKeithley2410.getVoltage();
						LOG(INFO) << BOLDBLUE << "IMUX: " << BOLDYELLOW <<  IMUXcode << " " << RESET;
					}
				}
				chipInterface.WriteReg(chip, "MonitorEnable", 1); //Choose MUX entry
				chipInterface.WriteReg(chip, "VMonitor", VMUXcode);
				results[chip].VMUXvolt[VMUXcode] = dKeithley2410.getVoltage();
				LOG(INFO) << BOLDBLUE << "VMUX: " << BOLDYELLOW <<  VMUXcode << " " << RESET;
			}
        });

        return results;
    }

#ifdef __USE_ROOT__

    void draw(ChipDataMap<ChipResults>& results, int run_counter) const {
        for (const auto& item : results) {
            MuxScanHistograms* histos;
            histos->fillMUX(
                item.second.VMUXvolt, 
                item.second.IMUXvolt,
                0
            );
        }
    }

#endif

};

}

#endif


