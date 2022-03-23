#ifndef RD53ShortTempSensor_H
#define RD53ShortTempSensor_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include "../DQMUtils/RD53ShortTempSensorHistograms.h"


namespace RD53BTools {

template <class>
struct RD53ShortTempSensor; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53ShortTempSensor<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);

template <class Flavor>
struct RD53ShortTempSensor : public RD53BTool<RD53ShortTempSensor, Flavor> {
    using Base = RD53BTool<RD53ShortTempSensor, Flavor>;
    using Base::Base;
	
    struct ChipResults {
		double valueLow;
		double valueHigh;
		double temperature[5];
    };

	static constexpr int sensor_VMUX[4] = {0b000101, 0b000110, 0b001110, 0b010000};
	static constexpr double idealityFactor[4] = {1.4, 1.4, 1.4, 1.4};
	

    auto run() {
		constexpr float T0C = 273.15;         // [Kelvin]
		constexpr float kb  = 1.38064852e-23; // [J/K]
		constexpr float e   = 1.6021766208e-19;
		constexpr float R   = 15; // By circuit design
		
        ChipDataMap<ChipResults> results;
        auto& chipInterface = Base::chipInterface();

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(Base::system().fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);

        LOG(INFO) << "[RD53ShortTempSensor] exampleParam = " << Base::param("exampleParam"_s) << RESET;
		
		
		uint16_t sensorConfigData;

        Base::for_each_chip([&] (Chip* chip) {
			auto& valueLow = results[chip].valueLow;
			auto& valueHigh = results[chip].valueHigh;
			auto& temperature = results[chip].temperature;
		
			for(int sensor=0;sensor<4;sensor++){				
				chipInterface.WriteReg(chip, "MonitorEnable", 1); //Choose VMUX entry
				chipInterface.WriteReg(chip, "VMonitor", sensor_VMUX[sensor]);
				
				valueHigh = 0;
				// Get high bias voltage
				for(int sensorDEM=0;sensorDEM<16;sensorDEM+=7){ //Unsure if this actually works
					sensorConfigData = bits::pack<1, 4, 1, 1, 4, 1>(true, sensorDEM, 0, true, sensorDEM, 0);
					chipInterface.WriteReg(chip, "MON_SENS_SLDO", sensorConfigData); //Apply high bias
					valueHigh += dKeithley2410.getVoltage();
				}
				valueHigh = valueHigh/3;
				
				valueLow = 0;
				// Get low bias voltage
				for(int sensorDEM=0;sensorDEM<16;sensorDEM+=7){
					sensorConfigData = bits::pack<1, 4, 1, 1, 4, 1>(true, sensorDEM, 1, true, sensorDEM, 1);
					chipInterface.WriteReg(chip, "MON_SENS_SLDO", sensorConfigData); //Apply low bias
					valueLow += dKeithley2410.getVoltage();
				}
				valueLow = valueLow/3;
				
				temperature[sensor] = (e/(kb*log(R)))*(valueLow-valueHigh)/idealityFactor[sensor]-T0C;
			}
			//Get NTC temperature
			//temperature[4]=chipInterface.ReadHybridTemperature(chip); //Does not currently work, to be fixed
			chipInterface.WriteReg(chip, "VMonitor", 2);
			temperature[4]=dKeithley2410.getVoltage(); 
        });
        return results;
    }

    void draw(const ChipDataMap<ChipResults>& results) const {
        for (const auto& item : results) {
			ShortTempSensorHistograms* histos = new ShortTempSensorHistograms;
            histos->fillSTS(
                item.second.temperature
            );
        }
    }
};

template <class Flavor>
const double RD53ShortTempSensor<Flavor>::idealityFactor[];

template <class Flavor>
const int RD53ShortTempSensor<Flavor>::sensor_VMUX[];
}

#endif


