/*!
  \file                  RD53ADCScan.cc
  \brief                 Implementaion of ADCScan
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  08/01/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53ADCScan.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_ITchipTesting;

void ADCScan::run(std::string configFile)
{

    ITpowerSupplyChannelInterface dKeithley2410(fPowerSupplyClient, "TestKeithley", "Front");

    dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);
	
	
	auto run = [&] (auto* chipInterface) {	
		// #########################
		// # Mark enabled channels #
		// #########################
		for(const auto cBoard: *fDetectorContainer)
			for(const auto cOpticalGroup: *cBoard)
				for(const auto cHybrid: *cOpticalGroup)
					for(const auto cChip: *cHybrid)
					{
						for(int input = 0; input < 4096; input+=10)
						{
							LOG(INFO) << BOLDBLUE << "i        = " << BOLDYELLOW << input << " " << RESET;
							for(int variable = 0; variable < 1; variable++)
							{
								if(input > 4096) continue;
								if(input == 0)
								{
									VMUXvolt[variable] = new double[5000];
									ADCcode[variable]  = new double[5000];
								}
								fReadoutChipInterface->WriteChipReg(cChip, "MEAS_CAP", 1);
								fReadoutChipInterface->WriteChipReg(cChip, writeVar[variable], input);
								chipInterface->SendGlobalPulse(cChip, {"ResetADC"},1); //Reset ADC
								fReadoutChipInterface->WriteChipReg(cChip, "MonitorConfig", 0b1000000000111); //Choose MUX entry
								chipInterface->SendGlobalPulse(cChip, {"ADCStartOfConversion"}); //ADC start conversion
								
								ADCcode[variable][int(input/10)] = fReadoutChipInterface->ReadChipReg(cChip, "MonitoringDataADC"); //Read ADC code
								VMUXvolt[variable][int(input/10)] = dKeithley2410.getVoltage();

								if(input > 1)
								{
									if(((ADCcode[variable][int(input/10)] > 0 && ADCcode[variable][int(input/10) - 1] == 0) || (ADCcode[variable][int(input/10) - 1] > 0 && ADCcode[variable][int(input/10)] == 0)) &&
									   fitStart[variable] == 0)
									{ fitStart[variable] = ADCcode[variable][int(input/10)]; }
									if(((ADCcode[variable][int(input/10)] == 4095 && ADCcode[variable][int(input/10) - 1] < 4095) || (ADCcode[variable][int(input/10)] < 4095 && ADCcode[variable][int(input/10) - 1] == 4095)) &&
									   fitEnd[variable] == 0)
										fitEnd[variable] = ADCcode[variable][int(input/10)];
									if(fitEnd[variable] == 0 && input >= 4000) fitEnd[variable] = ADCcode[variable][int(input/10)];
								}
							}
						}
					}
		};
	
		if (fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
			run(static_cast<RD53BInterface<RD53BFlavor::ATLAS>*>(fReadoutChipInterface));
		else 
			run(static_cast<RD53BInterface<RD53BFlavor::CMS>*>(fReadoutChipInterface));
		
    // #endif
}

void ADCScan::draw(bool saveData)
{
#ifdef __USE_ROOT__
    histos->fillADC(*fDetectorContainer, fitStart, fitEnd, VMUXvolt, ADCcode, writeVar);
#endif
}
