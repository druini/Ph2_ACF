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
	std::string cHWFile = "settings/D19C_2xSSA_onechip.xml";
	std::stringstream outp;
	Tool cTool;
	cTool.InitializeHw ( cHWFile, outp);
	cTool.InitializeSettings ( cHWFile, outp );
	D19cFWInterface* IB = dynamic_cast<D19cFWInterface*>(cTool.fBeBoardFWMap.find(0)->second); // There has to be a better way!
	cTool.ConfigureHw();
	
	BeBoard* pBoard = cTool.fBoardVector.at(0);
	std::vector < ReadoutChip* > &ChipVec = pBoard->getModule(0)->fReadoutChipVector;
	TH2I *hist1 = new TH2I("h1", "All Chip #1;number_in ; number_out", 120, 0, 120, 120, 0, 120);
	TH2I *hist2 = new TH2I("h2", "All Chip #2;number_in ; number_out", 120, 0, 120, 120, 0, 120);
	hist1->SetStats(0);
	hist2->SetStats(0);
	for(auto cSSA: ChipVec)
	{
		cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ReadoutMode", 0x0); // sync mode = 0
		for (int i = 1; i<=120;i++ ) // loop over all strips
		{
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "THTRIMMING_S" + std::to_string(i), 31); // MAXIMIZE THE TRIM
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ENFLAGS_S" + std::to_string(i), 0x0); // ENABLE THE STRIP
		}
	}
	for (int input = 1; input<=120;input++ ) // loop over all strips
	{
	    cTool.fReadoutChipInterface->WriteChipReg(ChipVec[0], "ENFLAGS_S" + std::to_string(input), 0x1); // ENABLE THE STRIP
	    cTool.fReadoutChipInterface->WriteChipReg(ChipVec[1], "ENFLAGS_S" + std::to_string(input), 0x1); // ENABLE THE STRIP
	    std::vector<unsigned int> thehits;
	    for (int thd = 0; thd<=25; thd++)
	    {
		for(auto cSSA: ChipVec)
		{
			cTool.fReadoutChipInterface->WriteChipReg(cSSA, "Bias_THDAC", thd);
		}
		std::chrono::milliseconds cWait( 100 );
		std::this_thread::sleep_for( cWait );
		cTool.ReadNEvents(pBoard, 10);
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
					if (event->DataBit ( module->getId(), chip->getId(), channelNumber))
					{
						if (chip->getId() == 0) hist1->Fill(input, channelNumber);
						if (chip->getId() == 1) hist2->Fill(input, channelNumber);
					}
					channelNumber++;
				} // for on channel - end 
			    } // for on chip - end 
			} // for on module - end 
		} // for on events - end
	    }
	    cTool.fReadoutChipInterface->WriteChipReg(ChipVec[0], "ENFLAGS_S" + std::to_string(input), 0x0); // ENABLE THE STRIP
	    cTool.fReadoutChipInterface->WriteChipReg(ChipVec[1], "ENFLAGS_S" + std::to_string(input), 0x0); // ENABLE THE STRIP
	}
	TCanvas * C_svd = new TCanvas("C_svd", "C_svd", 800, 350);
	C_svd->Divide(2,1);
	C_svd->cd(1);
	hist1->Draw("col");
	C_svd->cd(2);
	hist2->Draw("col");
	C_svd->Print("WTF.png");	
	C_svd->Print("WTF.root");		
}
