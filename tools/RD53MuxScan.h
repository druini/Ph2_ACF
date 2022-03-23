#ifndef RD53MuxScan_H
#define RD53MuxScan_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include "../DQMUtils/RD53MuxScanHistograms.h"

#include <iostream>

namespace RD53BTools {

template <class>
struct RD53MuxScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53MuxScan<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);

template <class Flavor>
struct RD53MuxScan : public RD53BTool<RD53MuxScan, Flavor> {
    using Base = RD53BTool<RD53MuxScan, Flavor>;
    using Base::Base;

    struct ChipResults {
        std::vector<double> VMUXvolt;
        std::vector<double> VMUXvolt_ADC;
        std::vector<double> IMUXvolt;
        std::vector<double> IMUXvolt_ADC;
    };

    auto run() const {
        ChipDataMap<ChipResults> results;
        auto& chipInterface = Base::chipInterface();

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(Base::system().fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        LOG(INFO) << "[RD53MuxScan] exampleParam = " << Base::param("exampleParam"_s) << RESET;

        Base::for_each_chip([&] (Chip* chip) {
		
			for(int VMUXcode=0;VMUXcode<40;VMUXcode++){
				//Set Mux output
				if (VMUXcode == 1){
					for(int IMUXcode=0;IMUXcode<32;IMUXcode++){
						chipInterface.WriteReg(chip, "MonitorEnable", 1); //Choose MUX entry
						chipInterface.WriteReg(chip, "VMonitor", 0b000001);
						chipInterface.WriteReg(chip, "IMonitor", IMUXcode);
						results[chip].IMUXvolt.push_back(dKeithley2410.getVoltage());
						chipInterface.SendGlobalPulse(chip, {"ADCStartOfConversion"}); //ADC start conversion
						results[chip].IMUXvolt_ADC.push_back(chipInterface.ReadReg(chip, "MonitoringDataADC"));
						LOG(INFO) << BOLDBLUE << "IMUX: " << BOLDYELLOW <<  IMUXcode << " " << RESET;
					}
				}else{
					chipInterface.WriteReg(chip, "MonitorEnable", 1); //Choose MUX entry
					chipInterface.WriteReg(chip, "VMonitor", VMUXcode);
					results[chip].VMUXvolt.push_back(dKeithley2410.getVoltage());
					chipInterface.SendGlobalPulse(chip, {"ADCStartOfConversion"}); //ADC start conversion
					results[chip].VMUXvolt_ADC.push_back(chipInterface.ReadReg(chip, "MonitoringDataADC"));
					LOG(INFO) << BOLDBLUE << "VMUX: " << BOLDYELLOW <<  VMUXcode << " " << RESET;
				}
			}
        });

        return results;
    }

    void draw(const ChipDataMap<ChipResults>& results) const {
        for (const auto& item : results) {
			MuxScanHistograms* histos = new MuxScanHistograms;
            histos->fillMUX(
                item.second.VMUXvolt, 
                item.second.VMUXvolt_ADC, 
                item.second.IMUXvolt,
                item.second.IMUXvolt_ADC
            );
        }
    }


};

}

#endif



