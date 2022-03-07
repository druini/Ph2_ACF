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
        std::vector<std::vector<double>> VMUXvolt;
        std::vector<std::vector<double>> DACcode;
    };

    auto run() const {
        ChipDataMap<ChipResults> results;
        auto& chipInterface = Base::chipInterface();

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(Base::system().fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);


        LOG(INFO) << "[RD53DACScan] exampleParam = " << Base::param("exampleParam"_s) << RESET;

        Base::for_each_chip([&] (Chip* chip) {
            auto& DACcode = results[chip].DACcode;
            auto& VMUXvolt = results[chip].VMUXvolt;
            auto& fitStart = results[chip].fitStart;
            auto& fitEnd = results[chip].fitEnd;
			int stepSize = 100;
			
			for(int input = 0; input < 4096; input+=stepSize)
			{
				LOG(INFO) << BOLDBLUE << "i        = " << BOLDYELLOW << input << " " << RESET;
				DACcode.push_back(std::vector<double>());
				VMUXvolt.push_back(std::vector<double>());
				for(int variable = 0; variable < 1; variable++)
				{
					if(input > 4096) continue;
					chipInterface.WriteReg(chip, writeVar[variable], input);
					chipInterface.WriteReg(chip, "MonitorEnable", 1); //Choose MUX entry
					chipInterface.WriteReg(chip, "VMonitor", 0b000111);
					
					DACcode[variable].push_back(input); //Read DAC code
					VMUXvolt[variable].push_back(dKeithley2410.getVoltage());

					if(input > 1)
					{
						if(((DACcode[variable].end()[-1] > 0 && DACcode[variable].end()[-2] == 0) || (DACcode[variable].end()[-2] > 0 && DACcode[variable].end()[-1] == 0)) && fitStart[variable] == 0)
							fitStart[variable] = DACcode[variable].end()[-1]; 
						if(((DACcode[variable].end()[-1] == 4095 && DACcode[variable].end()[-2] < 4095) || (DACcode[variable].end()[-1] < 4095 && DACcode[variable].end()[-2] == 4095)) && fitEnd[variable] == 0)
							fitEnd[variable] = DACcode[variable].end()[-1];
						if(fitEnd[variable] == 0 && input >= 4000) fitEnd[variable] = DACcode[variable].end()[-1];
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


