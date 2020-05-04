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
	std::string cHWFile = "settings/D19C_2xSSA_PreCalibSYNC.xml";
	std::stringstream outp;
	Tool cTool;
	cTool.InitializeHw ( cHWFile, outp);
	cTool.InitializeSettings ( cHWFile, outp );
	D19cFWInterface* IB = dynamic_cast<D19cFWInterface*>(cTool.fBeBoardFWMap.find(0)->second); // There has to be a better way!
	IB->PSInterfaceBoard_PowerOn_SSA(1.25, 1.25, 1.25, 0.3, 0.0, 1);
	IB->ReadPower_SSA();
	cTool.ConfigureHw();

	BeBoard* pBoard = static_cast<BeBoard*>(cTool.fDetectorContainer->at(0));
	ModuleContainer* ChipVec = pBoard->at(0)->at(0);
	cTool.setFWTestPulse();
	cTool.setSystemTestPulse (200, 0, true, false);
	TH1I *h1 = new TH1I("h1", ";Latency (lsb)", 256, 0, 256);

	for(auto cSSA: *ChipVec)
	{
		ReadoutChip* theSSA = static_cast<ReadoutChip*>(cSSA);
		cTool.fReadoutChipInterface->WriteChipReg(theSSA, "Bias_CALDAC", 100);
		cTool.fReadoutChipInterface->WriteChipReg(theSSA, "ReadoutMode", 0x0); // sync mode = 0
		cTool.fReadoutChipInterface->WriteChipReg(theSSA, "Bias_THDAC", 50);
		cTool.fReadoutChipInterface->WriteChipReg(theSSA, "FE_Calibration", 1);
		for (int i = 1; i<=120;i++ ) // loop over all strips
		{
			cTool.fReadoutChipInterface->WriteChipReg(theSSA, "THTRIMMING_S" + std::to_string(i), 0);
			cTool.fReadoutChipInterface->WriteChipReg(theSSA, "ENFLAGS_S" + std::to_string(i), 17); // 17 = 10001 (enable strobe)
		}
		cTool.fReadoutChipInterface->WriteChipReg(theSSA, "L1-Latency_MSB", 0x0);
	}
	int count_max = 0;
	int BestLat = 0;
	for (int lat = 0; lat<=255; lat++)
	{
		for(auto cSSA: *ChipVec)
		{
		ReadoutChip* theSSA = static_cast<ReadoutChip*>(cSSA);
			cTool.fReadoutChipInterface->WriteChipReg(theSSA, "L1-Latency_LSB", lat);
		}
		cTool.ReadNEvents(pBoard, 50);
		int thiscount = 0;
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
						thiscount = thiscount + event->DataBit ( module->getId(), chip->getId(), channelNumber);
						h1->Fill(lat, event->DataBit ( module->getId(), chip->getId(), channelNumber));
	                	channelNumber++;
	                } // for on channel - end 
	            } // for on chip - end 
	        } // for on module - end 
	    } // for on events - end*/
	    if (thiscount > count_max) BestLat = lat;
	}
	TCanvas * c1 = new TCanvas("c", "c", 600, 600);
	c1->cd();
	h1->Draw("hist");
	c1->Print("LATENCY_SCAN.png");
	LOG (INFO) << BOLDRED << "THE LATENCY IS: " << BestLat << RESET;
	for(auto cSSA: *ChipVec)
	{
		ReadoutChip* theSSA = static_cast<ReadoutChip*>(cSSA);
		cTool.fReadoutChipInterface->WriteChipReg(theSSA, "L1-Latency_LSB", BestLat);
		cTool.fReadoutChipInterface->WriteChipReg(theSSA, "Bias_CALDAC", 60);
	}
	TH1I *h2 = new TH1I("h2", "S-CURVE STRIP #7;THDAC (lsb)", 256, 0, 256);
	for (int thd = 0; thd<=255; thd++)
	{
		for(auto cSSA: *ChipVec)
		{
		ReadoutChip* theSSA = static_cast<ReadoutChip*>(cSSA);
			cTool.fReadoutChipInterface->WriteChipReg(theSSA, "Bias_THDAC", thd);
			for (int i = 1; i<=120;i++ ) // loop over all strips
			{
				if (i != 7)	cTool.fReadoutChipInterface->WriteChipReg(theSSA, "ENFLAGS_S" + std::to_string(i), 0);
			}
		}
		cTool.ReadNEvents(pBoard, 100);
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
						h2->Fill(thd, event->DataBit ( module->getId(), chip->getId(), channelNumber));
	                	channelNumber++;
	                } // for on channel - end 
	            } // for on chip - end 
	        } // for on module - end 
	    } // for on events - end*/
	}
	TCanvas * c2 = new TCanvas("c2", "c2", 600, 600);
	c2->cd();
	h2->Draw("hist");
	c2->Print("SCURVE_7.png");

	IB->PSInterfaceBoard_PowerOff_SSA();
}
