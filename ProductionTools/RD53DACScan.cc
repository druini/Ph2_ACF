/*!
  \file                  RD53DACScan.cc
  \brief                 Implementaion of DACScan
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  18/03/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53DACScan.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_ITchipTesting;

void DACScan::run(std::string configFile)
{
    ITpowerSupplyChannelInterface dKeithley2410(fPowerSupplyClient, "TestKeithley", "Front");

    dKeithley2410.setupKeithley2410ChannelSense(VOLTAGESENSE, 2.0);

    // #########################
    // # Mark enabled channels #
    // #########################
    auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);
    for(const auto cBoard: *fDetectorContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    for(int input = 0; input < 4096; input += 10)
                    {
                        LOG(INFO) << BOLDBLUE << "i        = " << BOLDYELLOW << input << " " << RESET;
                        for(int variable = 0; variable < 1; variable++)
                        {
                            if(input > 4095) continue;
                            if(input == 0)
                            {
                                VMUXvolt[variable] = new double[5000];
                                DACcode[variable]  = new double[5000];
                            }
                            fReadoutChipInterface->WriteChipReg(cChip, writeVar[variable], input);
                            //fReadoutChipInterface->WriteChipReg(cChip, "VCAL_MED", input);
                            DACcode[variable][int(input/10)] = input;
                            // Change voltage being read
							fReadoutChipInterface->WriteChipReg(cChip, "MonitorConfig", 0b1000000000111); //Choose VCAL_HIGH MUX entry
							//fReadoutChipInterface->WriteChipReg(cChip, "MonitorConfig", 0b1000000001000); //Choose VCAL_MED MUX entry

                            VMUXvolt[variable][int(input/10)] = dKeithley2410.getVoltage();
                            LOG(INFO) << BOLDBLUE << "VMUXvolt[int(input/10)]        = " << BOLDYELLOW << VMUXvolt[variable][int(input/10)] << " " << RESET;

                            if(input > 1)
                            {
                                if(((DACcode[variable][int(input/10)] > 0 && DACcode[variable][int(input/10) - 1] == 0) || (DACcode[variable][int(input/10) - 1] > 0 && DACcode[variable][int(input/10)] == 0)) &&
                                   fitStart[variable] == 0)
                                {
                                    fitStart[variable] = VMUXvolt[variable][int(input/10) - 1];
                                }
                                if(((DACcode[variable][int(input/10)] == 4095 && DACcode[variable][int(input/10) - 1] < 4095) || (DACcode[variable][int(input/10)] < 4095 && DACcode[variable][int(input/10) - 1] == 4095)) &&
                                   fitEnd[variable] == 0)
                                    fitEnd[variable] = VMUXvolt[variable][int(input/10)];
                                if(fitEnd[variable] == 0 && input == 4094) fitEnd[variable] = VMUXvolt[variable][int(input/10)];
                            }
                        }
                    }
                }
    // #endif
}

void DACScan::draw(bool saveData)
{
#ifdef __USE_ROOT__
    histos->fillDAC(*fDetectorContainer, fitStart, fitEnd, VMUXvolt, DACcode, writeVar);
#endif
}
