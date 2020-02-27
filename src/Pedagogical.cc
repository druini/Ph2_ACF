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
#include "TH2.h"
#include "TLine.h"
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

	LOG (INFO) << BOLDRED << "=============" << RESET;
	el::Configurations conf ("settings/logger.conf");
	el::Loggers::reconfigureAllLoggers (conf);	
	std::string cHWFile = "settings/D19C_2xSSA_Calib.xml";
	std::stringstream outp;
	Tool cTool;
	cTool.InitializeHw ( cHWFile, outp);
	cTool.InitializeSettings ( cHWFile, outp );
	//D19cFWInterface* IB = dynamic_cast<D19cFWInterface*>(cTool.fBeBoardFWMap.find(0)->second); // There has to be a better way!
	//IB->PSInterfaceBoard_PowerOff_SSA();
	cTool.ConfigureHw();
	
	BeBoard* pBoard = cTool.fBoardVector.at(0);
	std::vector < ReadoutChip* > &ChipVec = pBoard->getModule(0)->fReadoutChipVector;
	TH2I *strip_v_thdac_31 = new TH2I("strip_v_thdac_31", "All TRIMDACs = 31;strip # ; THDAC (lsb)", 360, -60, 300, 25, 0, 25);
	strip_v_thdac_31->SetStats(0);
	for(auto cSSA: ChipVec)
	{
		cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ReadoutMode", 0x0); // sync mode = 0
		for (int i = 1; i<=120;i++ ) // loop over all strips
		{
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "THTRIMMING_S" + std::to_string(i), 31); // MAXIMIZE THE TRIM
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ENFLAGS_S" + std::to_string(i), 1); // ENABLE THE STRIP
		}
	}
	// for(auto cSSA: ChipVec){cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ENFLAGS_S100", 0x1);}
	// for(auto cSSA: ChipVec){cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ENFLAGS_S101", 0x1);}
	// for(auto cSSA: ChipVec){cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ENFLAGS_S102", 0x1);}
	// for(auto cSSA: ChipVec){cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ENFLAGS_S103", 0x1);}
	// for(auto cSSA: ChipVec){cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ENFLAGS_S104", 0x1);}
	// for(auto cSSA: ChipVec){cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ENFLAGS_S105", 0x1);}
	for (int thd = 10; thd<=20; thd++)
	{
		for(auto cSSA: ChipVec)
		{
			std::cout<<"Setting threshold to " << thd << std::endl;
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "Bias_THDAC", thd);
		}
	cTool.ReadNEvents(pBoard, 1);
	const std::vector<Event*> &eventVector = cTool.GetEvents(pBoard);
	for ( auto &event : eventVector ) //for on events - begin 
    	{
		for(auto module: *pBoard) // for on module - begin 
		{
		    for(auto chip: *module) // for on chip - begin 
		    {
		        unsigned int channelNumber = 0;
		        for (int i = 0; i<=120;i++ ) // loop over all strips
			{
				if (event->DataBit ( module->getId(), chip->getId(), channelNumber)) LOG (INFO) << RED << i << ", " << int(chip->getId()) <<  RESET;
				strip_v_thdac_31->Fill(channelNumber+(120*int(chip->getId())), thd, event->DataBit ( module->getId(), chip->getId(), channelNumber));
		        	channelNumber++;
		        } // for on channel - end 
		        for (auto S: event->GetHits(module->getId(), chip->getId()))
			{
				LOG(INFO) << BOLDRED << "stub: " << float(S)/2. << RESET;
			}
		    } // for on chip - end 
		} // for on module - end 
	    } // for on events - end
	}
	TLine * L1 = new TLine(120,0,120,25);
	TLine * L2 = new TLine(240,0,240,25);
	L1->SetLineColor(2);
	L2->SetLineColor(2);


	TCanvas * C_svd = new TCanvas("C_svd", "C_svd", 600, 600);
	C_svd->cd();
	strip_v_thdac_31->Draw("colz");
	L1->Draw();
	L2->Draw();
	C_svd->Print("STRIP_DAC_MAP.png");		

	//IB->PSInterfaceBoard_PowerOff_SSA();
}
