/*!
  \file                  LatencyScanAsync.cc
  \brief                 Implementaion of SCurve scan
  \author                kevin Nash
  \version               1.0
  \date                  28/06/18
  Support:               email to knash@gmail.com
*/

#include "SSALatencyScan.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

SSALatencyScan::SSALatencyScan() :
    Tool()
{
}


SSALatencyScan::~SSALatencyScan()
{
}


void SSALatencyScan::Initialise(void)
{
}

void SSALatencyScan::run(void)
{
	DetectorDataContainer       theHitContainer;
	ContainerFactory::copyAndInitChannel<std::pair<std::array<uint32_t,2>,float>>(*fDetectorContainer, theHitContainer);

	for (auto cBoard : theHitContainer)
	{
	BeBoard* theBeBoard = static_cast<BeBoard*>( fDetectorContainer->at(cBoard->getIndex()) );
        theBeBoard->setEventType(EventType::SSA);


	this->enableTestPulse( true );
	setFWTestPulse();

	setSameDacBeBoard(theBeBoard, "Bias_CALDAC", 120);
	setSameDacBeBoard(theBeBoard, "Bias_THDAC", 60);

	for(auto cOpticalGroup : *cBoard)
	{
		for(auto cHybrid : *cOpticalGroup)
		{
			for(auto cChip : *cHybrid)
			{
			    ReadoutChip *theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(cBoard ->getIndex())->at(cOpticalGroup ->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex()));
			    //this->fReadoutChipInterface->WriteChipReg(theChip, "Bias_CALDAC", 120);
			    //this->fReadoutChipInterface->WriteChipReg(theChip, "ReadoutMode", 0x0);
			    //this->fReadoutChipInterface->WriteChipReg(theChip, "Bias_THDAC", 60);
			    //this->fReadoutChipInterface->WriteChipReg(theChip, "FE_Calibration", 1);
	    		    //for (int i = 1; i<=120;i++ )
			    //		{
				//	    this->fReadoutChipInterface->WriteChipReg(theChip, "THTRIMMING_S" + std::to_string(i), 0);
				//	    this->fReadoutChipInterface->WriteChipReg(theChip, "ENFLAGS_S" + std::to_string(i), 17); // 17 = 10001 (enable strobe)
			    	//	}
			    this->fReadoutChipInterface->WriteChipReg(theChip, "L1-Latency_MSB", 0x0);

			}
		}
	}


	uint32_t bestlat=0;
	uint32_t maxcount=0;


	for (uint32_t lat = 0; lat<=255; lat++)
	{
		for(auto cOpticalGroup : *cBoard)
		{
			for(auto cHybrid : *cOpticalGroup)
			{
				for(auto cChip : *cHybrid)
				{
					ReadoutChip *theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(cBoard ->getIndex())->at(cOpticalGroup ->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex()));
					this->fReadoutChipInterface->WriteChipReg(theChip, "L1-Latency_LSB", lat);
				}

			}
		}
		//static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTriggerFSM( 100, 1000, 6, 0, 0);

		this->ReadNEvents(theBeBoard, 200);
		const std::vector<Event*> &eventVector = this->GetEvents ( theBeBoard );
		unsigned int thiscount = 0;
		for ( auto &event : eventVector ) //for on events - begin
		{
			for(auto cOpticalGroup : *cBoard)
			{
				for(auto hybrid: *cOpticalGroup) // for on hybrid - begin
				{
					for(auto cSSA: *hybrid) // for on chip - begin
					{
						unsigned int channelNumber = 0;
						for (int i = 1; i<=120;i++ )
						{
							thiscount = thiscount + event->DataBit ( hybrid->getId(), cSSA->getId(), channelNumber);
			                		channelNumber++;
						}
					}
				}
			}
		}

		if (thiscount>maxcount)
		{
			maxcount=thiscount;
			bestlat=lat;

		}
		std::cout<<"lat "<<lat<<" cnt "<<thiscount<<std::endl;
	}
	LOG (INFO) << BOLDRED << "Best Latency Value " <<std::hex<<bestlat<<std::dec<< RESET;
	for(auto cOpticalGroup : *cBoard)
	{
		for(auto cHybrid : *cOpticalGroup)
		{
			for(auto cChip : *cHybrid)
			{
				ReadoutChip *theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(cBoard ->getIndex())->at(cOpticalGroup ->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex()));
				this->fReadoutChipInterface->WriteChipReg(theChip, "L1-Latency_LSB", bestlat);
			}

		}
	}
}
}