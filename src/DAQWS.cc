//
#include <cstring>
#include <iostream>
#include <fstream>
#include "../Utils/Utilities.h"
#include "../HWDescription/Chip.h"
#include "../HWDescription/ReadoutChip.h"
#include "../HWDescription/OuterTrackerModule.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/D19cFWInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/FrontEndDescription.h"
#include "../Utils/Timer.h"
#include <inttypes.h>
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../tools/Tool.h"
#include "../tools/CalibrationExample.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TApplication.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

int main( int argc, char* argv[] )
{
	el::Configurations conf ("settings/logger.conf");
	el::Loggers::reconfigureAllLoggers (conf);	
	std::string cHWFile = "settings/D19C_2xSSA_onechip.xml";
	std::stringstream outp;
	Tool cTool;
	cTool.InitializeHw ( cHWFile, outp);
	cTool.InitializeSettings ( cHWFile, outp );
	D19cFWInterface* IB = dynamic_cast<D19cFWInterface*>(cTool.fBeBoardFWMap.find(0)->second); // There has to be a better way!
	IB->PSInterfaceBoard_PowerOn_SSA(1.25, 1.25, 1.25, 0.3, 0.0, 145);
	IB->ReadPower_SSA();
	cTool.ConfigureHw();
	
	BeBoard* pBoard = cTool.fBoardVector.at(0);
	std::vector < ReadoutChip* > &ChipVec = pBoard->getModule(0)->fReadoutChipVector;
	cTool.setFWTestPulse();
	TH1I *h1 = new TH1I("h1", "S-CURVE", 256, 0, 256);
	for (int thd = 0; thd<=256; thd++)
	{
		for(auto cSSA: ChipVec)
		{
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "Bias_CALDAC", 30);
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ReadoutMode", 0x1); // sync mode = 0
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "Bias_THDAC", thd);
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "FE_Calibration", 1);
			for (int i = 1; i<=120;i++ ) // loop over all strips
			{
				//cTool.fReadoutChipInterface->WriteChipReg(cSSA, "THTRIMMING_S" + std::to_string(i), 31);
				cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ENFLAGS_S" + std::to_string(i), 5); // 17 = 10001 (enable strobe)
			}
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "L1-Latency_LSB", 0x44);
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "L1-Latency_MSB", 0x0);
		}
		
		cTool.Start(0);
		std::this_thread::sleep_for (std::chrono::milliseconds(50));
		cTool.Stop();
		for(auto cSSA: ChipVec)
		{
			uint8_t cRP1 = cTool.fReadoutChipInterface->ReadChipReg(cSSA, "ReadCounter_LSB_S18");
			uint8_t cRP2 = cTool.fReadoutChipInterface->ReadChipReg(cSSA, "ReadCounter_MSB_S18");
			uint16_t cRP = (cRP2*256) + cRP1; 

			LOG (INFO) << BOLDRED << "THDAC = " << thd << ", HITS = " << cRP << RESET;
			h1->Fill(thd, cRP);
		}
		/*
		cTool.ReadNEvents(pBoard, 200);
		const std::vector<Event*> &eventVector = cTool.GetEvents(pBoard);

		for ( auto &event : eventVector ) //for on events - begin 
	    {
	        for(auto module: *pBoard) // for on module - begin 
	        {
	            for(auto chip: *module) // for on chip - begin 
	            {
	                unsigned int channelNumber = 0;
	                for (int i = 1; i<=120;i++ ) // loop over all strips
					{
						h1->Fill(thd, event->DataBit ( module->getId(), chip->getId(), channelNumber));
						LOG (INFO) << BOLDBLUE << "hits on channel "<<channelNumber<< " = " << event->DataBit ( module->getId(), chip->getId(), channelNumber) <<RESET;
	                	channelNumber++;
	                } // for on channel - end 
	            } // for on chip - end 
	        } // for on module - end 
	    } // for on events - end*/
	}
	TCanvas * c1 = new TCanvas("c", "c", 600, 600);
	c1->cd();
	h1->Draw("hist");
	c1->Print("INJ.png");

	IB->PSInterfaceBoard_PowerOff_SSA();
}