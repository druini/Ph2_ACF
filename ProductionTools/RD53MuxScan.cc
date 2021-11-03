/*!
  \file                  RD53MuxScan.cc
  \brief                 Implementaion of MuxScan
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  05/08/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53MuxScan.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_ITchipTesting;

void MuxScan::run()
{
	ITpowerSupplyChannelInterface dKeithley2410(fPowerSupplyClient, "TestKeithley", "Front");

	dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);
	
    // #########################
    // # Mark enabled channels #
    // #########################
    for(const auto cBoard: *fDetectorContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid){
					for(int VMUXcode=0;VMUXcode<40;VMUXcode++){
						//Set Mux output
						if (VMUXcode == 1){
							for(int IMUXcode=0;IMUXcode<32;IMUXcode++){
								fReadoutChipInterface->WriteChipReg(cChip, "MonitorConfig", 4096+IMUXcode*64+1); //Choose MUX entry	
								IMUXvolt[IMUXcode] = dKeithley2410.getVoltage();
								LOG(INFO) << BOLDBLUE << "IMUX: " << BOLDYELLOW <<  IMUXcode << " " << RESET;
							}
						}
						fReadoutChipInterface->WriteChipReg(cChip, "MonitorConfig", 4096+VMUXcode); //Choose MUX entry
						VMUXvolt[VMUXcode] = dKeithley2410.getVoltage();
						LOG(INFO) << BOLDBLUE << "VMUX: " << BOLDYELLOW <<  VMUXcode << " " << RESET;
					}
				}
}

void MuxScan::draw( int run_counter )
{
	#ifdef __USE_ROOT__
		histos->fillMUX( VMUXvolt, IMUXvolt, run_counter );
	#endif
}
