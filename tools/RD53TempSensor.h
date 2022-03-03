#ifndef RD53TempSensor_H
#define RD53TempSensor_H

#include "RD53BTool.h"

#include "../ProductionTools/ITchipTestingInterface.h"

#include "../DQMUtils/RD53TempSensorHistograms.h"


namespace RD53BTools {

template <class>
struct RD53TempSensor; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53TempSensor<Flavor>> = make_named_tuple(
    std::make_pair("exampleParam"_s, 42)
);

template <class Flavor>
struct RD53TempSensor : public RD53BTool<RD53TempSensor, Flavor> {
    using Base = RD53BTool<RD53TempSensor, Flavor>;
    using Base::Base;
	
    struct ChipResults {
		double idealityFactor[4];
		double valueLow;
		double valueHigh;
		double calibDV[4][2];
		double calibNTCtemp[4][2];
		double calibSenstemp[4][2];
		double NTCvoltage;
		//double ADCslope;
		//double ADCintercept;
		//double currentFactor;
    };

	
	static constexpr double power[2] = {1.25, 1.92};
	static constexpr int sensor_VMUX[4] = {0b1000000000101, 0b1000000000110, 0b1000000001110, 0b1000000010000};
	

    auto run() {
		constexpr float T0C = 273.15;         // [Kelvin]
		constexpr float kb  = 1.38064852e-23; // [J/K]
		constexpr float e   = 1.6021766208e-19;
		constexpr float R   = 15; // By circuit design
		//double      power[2] = {0.24, 0.72}; // In direct powering for RD53A
		constexpr float T25C = 298.15; // [Kelvin]
		constexpr float R25C = 10;     // [kOhm]
		constexpr float Rdivider = 39.2; // [kOhm]
		constexpr float Vdivider = 2.5;  // [V]
		constexpr float beta = 1000;  // [?]
		
        ChipDataMap<ChipResults> results;
        auto& chipInterface = Base::chipInterface();

        Ph2_ITchipTesting::ITpowerSupplyChannelInterface dKeithley2410(Base::system().fPowerSupplyClient, "TestKeithley", "Front");

        dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);

        LOG(INFO) << "[RD53TempSensor] exampleParam = " << Base::param("exampleParam"_s) << RESET;
		
		
		uint16_t sensorConfigData;

		Base::for_each_board([&] (BeBoard* board) {
			auto& fwInterface = Base::getFWInterface(board);
			for_each_device<Chip>(board, [&] (Chip* chip) {
				
				auto& idealityFactor = results[chip].idealityFactor;
				auto& valueLow = results[chip].valueLow;
				auto& valueHigh = results[chip].valueHigh;
				auto& calibDV = results[chip].calibDV;
				auto& calibNTCtemp = results[chip].calibNTCtemp;
				auto& calibSenstemp = results[chip].calibSenstemp;
				auto& NTCvoltage = results[chip].NTCvoltage;
			
				for(int mode=0;mode<2;mode++){
					if(mode == 0){ //Low power mode
						chipInterface.WriteReg(chip, "DAC_KRUM_CURR_LIN", 0);
						chipInterface.WriteReg(chip, "DAC_LDAC_LIN", 0);
						chipInterface.WriteReg(chip, "DAC_COMP_LIN", 0);
						chipInterface.WriteReg(chip, "DAC_REF_KRUM_LIN", 0);
						chipInterface.WriteReg(chip, "EnCoreCol_0", 0);
						chipInterface.WriteReg(chip, "EnCoreCol_1", 0);
						chipInterface.WriteReg(chip, "EnCoreCol_2", 0);
						chipInterface.WriteReg(chip, "EnCoreCol_3", 0);
						//usleep(30000000);
					} else if(mode == 1){ //Standard power mode
						chipInterface.WriteReg(chip, "DAC_KRUM_CURR_LIN", 70);
						chipInterface.WriteReg(chip, "DAC_LDAC_LIN", 110);
						chipInterface.WriteReg(chip, "DAC_COMP_LIN", 110);
						chipInterface.WriteReg(chip, "DAC_REF_KRUM_LIN", 360);
						chipInterface.WriteReg(chip, "EnCoreCol_0", 0b0010010010010010);
						chipInterface.WriteReg(chip, "EnCoreCol_1", 0b1001001001001001);
						chipInterface.WriteReg(chip, "EnCoreCol_2", 0b0100100100100100);
						chipInterface.WriteReg(chip, "EnCoreCol_3", 0b010010);
						//usleep(30000000);
					}
					for(int sensor=0;sensor<4;sensor++){
						chipInterface.WriteReg(chip, "MonitorEnable", 1); //Choose NTC MUX entry
						chipInterface.WriteReg(chip, "VMonitor", 0b000010);
						//calibNTCtemp[sensor][mode] = chipInterface.ReadHybridTemperature(chip);
						fwInterface.ReadHybridTemperature(chip->getHybridId());
						NTCvoltage = dKeithley2410.getVoltage();
						float resistance  = NTCvoltage * Rdivider / (Vdivider - NTCvoltage);              // [kOhm]
						calibNTCtemp[sensor][mode] = 1. / (1. / T25C + log(resistance / R25C) / beta) - T0C; // [Celsius]
						LOG(INFO) << BOLDMAGENTA << "NTC VOLTAGE " << NTCvoltage << RESET;
						LOG(INFO) << BOLDMAGENTA << "NTC TEMPERATURE " << calibNTCtemp[sensor][mode] << RESET;
						chipInterface.WriteReg(chip, "MonitorConfig", sensor_VMUX[sensor]); //Choose MUX entry
						
						valueHigh = 0;
						// Get high bias voltage
						for(int sensorDEM=0;sensorDEM<16;sensorDEM+=7){ //Unsure if this actually works
							sensorConfigData = bits::pack<1, 4, 1, 1, 4, 1>(true, sensorDEM, 0, true, sensorDEM, 0);
							chipInterface.WriteReg(chip, "MON_SENS_SLDO", sensorConfigData);
							//chipInterface.WriteReg(chip, "SENSOR_CONFIG_1", sensorConfigData);
							valueHigh += dKeithley2410.getVoltage(); //dKeithley2410->getOutputVoltage();
							//LOG(INFO) << BOLDMAGENTA << dKeithley2410.getVoltage() << RESET;
						}
						valueHigh = valueHigh/3;
						
						valueLow = 0;
						// Get low bias voltage
						for(int sensorDEM=0;sensorDEM<16;sensorDEM+=7){
							sensorConfigData = bits::pack<1, 4, 1, 1, 4, 1>(true, sensorDEM, 1, true, sensorDEM, 1);
							chipInterface.WriteReg(chip, "MON_SENS_SLDO", sensorConfigData);
							//chipInterface.WriteReg(chip, "SENSOR_CONFIG_1", sensorConfigData);
							valueLow += dKeithley2410.getVoltage(); //dKeithley2410->getOutputVoltage();
								//LOG(INFO) << BOLDMAGENTA << dKeithley2410.getVoltage() << RESET;
						}
						valueLow = valueLow/3;
						
						calibDV[sensor][mode] = valueLow - valueHigh;
						LOG(INFO) << BOLDMAGENTA << calibDV[sensor][mode] << RESET; // use calcTemperature in /HWInterface/RD53FWInterface.h to go from volt to T (NTC is DV) 
						if (mode == 1){
							double NTCfactor = calibNTCtemp[sensor][0]-power[0]*(calibNTCtemp[sensor][1]-calibNTCtemp[sensor][0])/(power[1]-power[0]);
							idealityFactor[sensor] = e/(kb*log(R))*((calibDV[sensor][0]*power[1]-power[0]*calibDV[sensor][1])/((power[1]-power[0])*(NTCfactor+T0C)));
							calibSenstemp[sensor][0] = (e/(kb*log(R)))*(calibDV[sensor][0])/idealityFactor[sensor]-T0C;
							calibSenstemp[sensor][1] = (e/(kb*log(R)))*(calibDV[sensor][1])/idealityFactor[sensor]-T0C;
						}
					}
				}
			});
		});
        return results;
    }

    void draw(const ChipDataMap<ChipResults>& results) const {
        for (const auto& item : results) {
			TempSensorHistograms* histos = new TempSensorHistograms;
            histos->fillTC(
                item.second.idealityFactor, 
                item.second.calibNTCtemp, 
                item.second.calibSenstemp, 
                power
            );
        }
    }
};

template <class Flavor>
const double RD53TempSensor<Flavor>::power[];

template <class Flavor>
const int RD53TempSensor<Flavor>::sensor_VMUX[];
}

#endif


