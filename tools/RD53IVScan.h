#ifndef RD53IVScan_H
#define RD53IVScan_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

//#include "../DQMUtils/RD53MuxScanHistograms.h"

#include <iostream>

namespace RD53BTools
{
template <class>
struct RD53IVScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53IVScan<Flavor>> = make_named_tuple(
    std::make_pair("configFile"_s                    ,std::string("config/iv_it_sldo.xml")),
    std::make_pair("type"_s                          ,std::string("complete")),
    std::make_pair("powerSupplyName"_s               ,std::string("powerSupply")),
    std::make_pair("multimeterName"_s                ,std::string("multimeter")),
    std::make_pair("powerSupplyVoltageProtection"_s  ,3.0)
    );

template <class Flavor>
struct RD53IVScan : public RD53BTool<RD53IVScan, Flavor>
{
    using Base = RD53BTool<RD53IVScan, Flavor>;
    using Base::Base;
    // TODO The following struct has to be eliminated somehow - Antonio Nov 24 2021 18:59
    struct ChipResults
    {
        double VMUXvolt[41];
        double IMUXvolt[33];
    };

    auto run(Ph2_System::SystemController& system) const
    {
        // TODO The following line should be eliminated somehow - Antonio Nov 24 2021 18:41
        ChipDataMap<ChipResults> results;
	std::vector<Ph2_ITchipTesting::ITpowerSupplyChannelInterface> channelsPS;
	std::map<int, std::string> channelMap;

        LOG(INFO) << "PowerSupply client" << system.fPowerSupplyClient << RESET;

        Ph2_ITchipTesting::ITinstrumentsInterface scannerCardKeithley(system.fPowerSupplyClient, Base::param("configFile"_s), Base::param("multimeterName"_s));

        LOG(INFO) << "[RD53IVScan] configFile = " << Base::param("configFile"_s) << RESET;

        if     (Base::param("type"_s) == "complete") scannerCardKeithley.runScan();
        else if(Base::param("type"_s) == "steps")
	{
	  Ph2_ITchipTesting::ITpowerSupplyChannelInterface digitalChannel(system.fPowerSupplyClient, Base::param("powerSupplyName"_s), "first");
	  Ph2_ITchipTesting::ITpowerSupplyChannelInterface analogChannel(system.fPowerSupplyClient, Base::param("powerSupplyName"_s), "second");
	  channelsPS.push_back(digitalChannel);
	  channelsPS.push_back(analogChannel);
	  //std::cout << "Reading analog channel voltage: "  << std::endl << analogChannel .getVoltage()  << std::endl;
	  //std::cout << "Reading digital channel voltage: " << std::endl << digitalChannel.getVoltage() << std::endl;
	  //std::cout << "Reading analog channel current: "  << std::endl << analogChannel .getCurrent()  << std::endl;
	  //std::cout << "Reading digital channel current: " << std::endl << digitalChannel.getCurrent() << std::endl;
	  scannerCardKeithley.prepareMultimeter();
	  float voltageProtection = Base::param("powerSupplyVoltageProtection"_s);
	  for (unsigned int v = 0; v < channelsPS.size(); ++v)
	    channelsPS[v].setVoltageProtection(voltageProtection);
	  for (unsigned int v = 0; v < channelsPS.size(); ++v)
	    channelsPS[v].turnOn();
	  scannerCardKeithley.createScannerCardMap();
	}
	else LOG(ERROR) << BOLDRED << "Bad ""type"" value in toml configuration file for IVScan, please choose among [complete steps]!" << RESET;

        return results;
    }
};

} // namespace RD53BTools

#endif
