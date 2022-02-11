#ifndef RD53ADCScan_H
#define RD53ADCScan_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include "../DQMUtils/RD53ADCScanHistograms.h"


namespace RD53BTools {

template <class>
struct RD53ADCScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53ADCScan<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);

template <class Flavor>
struct RD53ADCScan : public RD53BTool<RD53ADCScan, Flavor> {
    using Base = RD53BTool<RD53ADCScan, Flavor>;
    using Base::Base;
	
    std::string readVar[9]  = {"CAL_HI", "CAL_MED", "REF_KRUM_LIN", "Vthreshold_LIN", "VTH_SYNC", "VBL_SYNC", "VREF_KRUM_SYNC", "VTH_HI_DIFF", "VTH_LO_DIFF"};
    std::string writeVar[9] = {"VCAL_HIGH", "VCAL_MED", "REF_KRUM_LIN", "Vthreshold_LIN", "VTH_SYNC", "VBL_SYNC", "VREF_KRUM_SYNC", "VTH1_DIFF", "VTH2_DIFF"};

    struct ChipResults {
        double fitStart[9];
        double fitEnd[9];
        std::vector<std::vector<double>> VMUXvolt;
        std::vector<std::vector<double>> ADCcode;
    };

    auto run(Ph2_System::SystemController& system) const {
        ChipDataMap<ChipResults> results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(system.fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        LOG(INFO) << "[RD53ADCScan] exampleParam = " << Base::param("exampleParam"_s) << RESET;

        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto& ADCcode = results[chip].ADCcode;
            auto& VMUXvolt = results[chip].VMUXvolt;
            auto& fitStart = results[chip].fitStart;
            auto& fitEnd = results[chip].fitEnd;
			int stepSize = 100;
			
			for(int input = 0; input < 4096; input+=stepSize)
			{
				if(input > 4096) continue;
				LOG(INFO) << BOLDBLUE << "i        = " << BOLDYELLOW << input << " " << RESET;
				ADCcode.push_back(std::vector<double>());
				VMUXvolt.push_back(std::vector<double>());
				for(int variable = 0; variable < 1; variable++)
				{
					chipInterface.WriteReg(chip, "MEAS_CAP", 1);
					chipInterface.WriteReg(chip, writeVar[variable], input);
					chipInterface.SendGlobalPulse(chip, {"ResetADC"},1); //Reset ADC
					chipInterface.WriteReg(chip, "MonitorEnable", 1); //Choose MUX entry
					chipInterface.WriteReg(chip, "VMonitor", 0b000111);
					chipInterface.SendGlobalPulse(chip, {"ADCStartOfConversion"}); //ADC start conversion
					
					ADCcode[variable].push_back(chipInterface.ReadReg(chip, "MonitoringDataADC")); //Read ADC code
					VMUXvolt[variable].push_back(dKeithley2410.getVoltage());

					if(input > 1)
					{
						if(((ADCcode[variable].end()[-1] > 0 && ADCcode[variable].end()[-2] == 0) || (ADCcode[variable].end()[-2] > 0 && ADCcode[variable].end()[-1] == 0)) && fitStart[variable] == 0)
							fitStart[variable] = ADCcode[variable].end()[-1]; 
						if(((ADCcode[variable].end()[-1] == 4095 && ADCcode[variable].end()[-2] < 4095) || (ADCcode[variable].end()[-1] < 4095 && ADCcode[variable].end()[-2] == 4095)) && fitEnd[variable] == 0)
							fitEnd[variable] = ADCcode[variable].end()[-1];
						if(fitEnd[variable] == 0 && input >= 4000) fitEnd[variable] = ADCcode[variable].end()[-1];
					}
				}
			}
        });

        return results;
    }

    void draw(const ChipDataMap<ChipResults>& results) const {
        for (const auto& item : results) {
            ADCScanHistograms* histos = new ADCScanHistograms;
            histos->fillADC(
                item.second.fitStart, 
                item.second.fitEnd, 
                item.second.VMUXvolt, 
                item.second.ADCcode, 
                writeVar
            );
        }
    }

};

}

#endif


