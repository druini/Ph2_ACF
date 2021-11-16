#ifndef RD53DACScan_H
#define RD53DACScan_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include "../DQMUtils/RD53DACScanHistograms.h"


namespace RD53BTools {

template <class>
struct RD53DACScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53DACScan<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);

template <class Flavor>
struct RD53DACScan : public RD53BTool<RD53DACScan, Flavor> {
    using Base = RD53BTool<RD53DACScan, Flavor>;
    using Base::Base;
	
    std::string readVar[9]  = {"CAL_HI", "CAL_MED", "REF_KRUM_LIN", "Vthreshold_LIN", "VTH_SYNC", "VBL_SYNC", "VREF_KRUM_SYNC", "VTH_HI_DIFF", "VTH_LO_DIFF"};
    std::string writeVar[9] = {"VCAL_HIGH", "VCAL_MED", "REF_KRUM_LIN", "Vthreshold_LIN", "VTH_SYNC", "VBL_SYNC", "VREF_KRUM_SYNC", "VTH1_DIFF", "VTH2_DIFF"};

    struct ChipResults {
        double fitStart[9];
        double fitEnd[9];
        double VMUXvolt[9][5000];
        double DACcode[9][5000];
    };

    auto run(Ph2_System::SystemController& system) const {
        ChipDataMap<ChipResults> results;
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(system.fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        LOG(INFO) << "[RD53DACScan] exampleParam = " << Base::param("exampleParam"_s) << RESET;

        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto& DACcode = results[chip].DACcode;
            auto& fitStart = results[chip].fitStart;
            auto& fitEnd = results[chip].fitEnd;
			
			for(int input = 0; input < 4096; input+=10)
			{
				LOG(INFO) << BOLDBLUE << "i        = " << BOLDYELLOW << input << " " << RESET;
				for(int variable = 0; variable < 1; variable++)
				{
					if(input > 4096) continue;
					//if(input == 0)
					//{
					//	VMUXvolt[variable] = new double[5000];
					//	DACcode[variable]  = new double[5000];
					//}
					chipInterface.WriteReg(chip, writeVar[variable], input);
					chipInterface.WriteReg(chip, "MonitorEnable", 1); //Choose MUX entry
					chipInterface.WriteReg(chip, "VMonitor", 0b000111);
					
					DACcode[variable][int(input/10)] = input; //Read DAC code
					results[chip].VMUXvolt[variable][int(input/10)] = dKeithley2410.getVoltage();

					if(input > 1)
					{
						if(((DACcode[variable][int(input/10)] > 0 && DACcode[variable][int(input/10) - 1] == 0) || (DACcode[variable][int(input/10) - 1] > 0 && DACcode[variable][int(input/10)] == 0)) &&
						   fitStart[variable] == 0)
						{ fitStart[variable] = DACcode[variable][int(input/10)]; }
						if(((DACcode[variable][int(input/10)] == 4095 && DACcode[variable][int(input/10) - 1] < 4095) || (DACcode[variable][int(input/10)] < 4095 && DACcode[variable][int(input/10) - 1] == 4095)) &&
						   fitEnd[variable] == 0)
							fitEnd[variable] = DACcode[variable][int(input/10)];
						if(fitEnd[variable] == 0 && input >= 4000) fitEnd[variable] = DACcode[variable][int(input/10)];
					}
				}
			}
        });

        return results;
    }

    void draw(const ChipDataMap<ChipResults>& results) const {
        for (const auto& item : results) {
			DACScanHistograms* histos = new DACScanHistograms;
            histos->fillDAC(
                item.second.fitStart, 
                item.second.fitEnd, 
                item.second.VMUXvolt, 
                item.second.DACcode, 
                writeVar
            );
        }
    }

};

}

#endif


