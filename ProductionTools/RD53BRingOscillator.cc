/*!
  \file                  RD53BRingOscillator.cc
  \brief                 Implementaion of RingOscillator
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  21/09/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53BRingOscillator.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_ITchipTesting;


void BRingOscillator::run()
{

	ITpowerSupplyChannelInterface dKeithley2410(fPowerSupplyClient, "TestKeithley", "Front");

	dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);
	
	auto run = [&] (auto* chipInterface) {	
    for(const auto cBoard: *fDetectorContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid){
					for(int gPulse = 0; gPulse < 10; gPulse++){ //VS PULSE DURATION
						gloPulse[gPulse] = gPulse*10;
						for(int ringOsc = 0; ringOsc < 8; ringOsc++){
							fReadoutChipInterface->WriteChipReg(cChip, "RingOscConfig", 0b000000111111111); //Reset Oscillator
							fReadoutChipInterface->WriteChipReg(cChip, "RingOscConfig", 0b000000011111111); //Enable Oscillator
							fReadoutChipInterface->WriteChipReg(cChip, "RingOscRoute", 64*(ringOsc));
							chipInterface->SendGlobalPulse(cChip, {"StartRingOscillatorsA"},gPulse*10); //Start Oscillators 
							oscCounts[ringOsc][gPulse] = fReadoutChipInterface->ReadChipReg(cChip, "RING_OSC_A_OUT") - 4096;
							LOG(INFO) << BOLDMAGENTA << "Counts: " << oscCounts[ringOsc][gPulse] << RESET;
							oscFrequency[ringOsc][gPulse] = oscCounts[ringOsc][gPulse]/((2*(gloPulse[gPulse]+1))/40);
						}
					}
					
					for(int vTrim = 0; vTrim < 16; vTrim++){ //VS VDDD
						//Trim voltages
						fReadoutChipInterface->WriteChipReg(cChip, "VOLTAGE_TRIM", vTrim);
						fReadoutChipInterface->WriteChipReg(cChip, "MonitorConfig", 4096+38); //Choose MUX entry	
						trimVoltage[vTrim] = dKeithley2410.getVoltage()*2;
						for(int ringOsc = 0; ringOsc < 34; ringOsc++){
							//Set up oscillators
							fReadoutChipInterface->WriteChipReg(cChip, "RingOscConfig", 0b111111000000000); //Reset Oscillator
							fReadoutChipInterface->WriteChipReg(cChip, "RingOscConfig", 0b011111000000000); //Enable Oscillator
							fReadoutChipInterface->WriteChipReg(cChip, "RingOscRoute", ringOsc);
							chipInterface->SendGlobalPulse(cChip, {"StartRingOscillatorsB"},50); //Start Oscillators 
							trimOscCounts[ringOsc][vTrim] = fReadoutChipInterface->ReadChipReg(cChip, "RING_OSC_B_OUT") - 4096;
							LOG(INFO) << BOLDMAGENTA << "Counts: " << trimOscCounts[ringOsc][vTrim] << RESET;
							trimOscFrequency[ringOsc][vTrim] = trimOscCounts[ringOsc][vTrim]/((2*51)/40);
						}
					}
				}
		};
	
		if (fDetectorContainer->at(0)->getFrontEndType() == FrontEndType::RD53B)
			run(static_cast<RD53BInterface<RD53BFlavor::ATLAS>*>(fReadoutChipInterface));
		else 
			run(static_cast<RD53BInterface<RD53BFlavor::CMS>*>(fReadoutChipInterface));
}

void BRingOscillator::draw()
{
	#ifdef __USE_ROOT__
		histos->fillRO(gloPulse, oscCounts, oscFrequency, trimOscCounts, trimOscFrequency, trimVoltage);
	#endif
}
