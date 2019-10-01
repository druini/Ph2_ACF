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

bool UserWriteReg(D19cFWInterface* BOARD, ReadoutChip* CHIP, std::string REGISTER, uint32_t VALUE)
{
	std::vector<uint32_t> cVecReq;
	CHIP->setReg(REGISTER, VALUE);
	ChipRegItem cRegItem = CHIP->getRegItem (REGISTER);
    BOARD->EncodeReg (cRegItem, CHIP->getFeId(), CHIP->getChipId(), cVecReq, true, true);
    uint8_t cWriteAttempts = 0;
    BOARD->WriteChipBlockReg (cVecReq, cWriteAttempts, true);
    cVecReq.clear();
	BOARD->EncodeReg (cRegItem, CHIP->getFeId(), CHIP->getChipId(), cVecReq, true, false);
	BOARD->ReadChipBlockReg (cVecReq);
	uint32_t CVQ = cVecReq[0];
	if (std::bitset<8>(CVQ) == VALUE)
	{
		LOG (INFO) << BOLDBLUE << REGISTER << " ----> " << std::bitset<8>(VALUE) <<RESET;
		return true;
	}
	else
	{
		LOG (INFO) << BOLDRED << REGISTER << " --> !!ERROR!! " << std::bitset<8>(VALUE) << "(" << std::bitset<8>(CVQ) << ")" << RESET;
		return false;
	}
}

uint8_t UserReadReg(D19cFWInterface* BOARD, ReadoutChip* CHIP, std::string REGISTER)
{
	std::vector<uint32_t> cVecReq;
	ChipRegItem cRegItem = CHIP->getRegItem(REGISTER);
	BOARD->EncodeReg (cRegItem, CHIP->getFeId(), CHIP->getChipId(), cVecReq, true, false);
	BOARD->ReadChipBlockReg( cVecReq );
	return (uint8_t)(cVecReq[0]&0xFF);
}

int main( int argc, char* argv[] )
{

	TH1I *h1 = new TH1I("h1", "h1 title", 256, 0, 256);


    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF calibration example" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/Calibration8CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );


    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

	el::Configurations conf ("settings/logger.conf");
	el::Loggers::reconfigureAllLoggers (conf);	
	std::string cHWFile = "settings/D19C_2xSSA_onechip.xml";
	std::stringstream outp;
	Tool cTool;
	cTool.InitializeHw ( cHWFile, outp);
	cTool.InitializeSettings ( cHWFile, outp );
	D19cFWInterface* IB = dynamic_cast<D19cFWInterface*>(cTool.fBeBoardFWMap.find(0)->second); // There has to be a better way!
	//IB->PSInterfaceBoard_PowerOn_SSA(1.25, 1.25, 1.25, 0.3, 0.0, 0, 0);
	//IB->ReadPower_SSA(0, 0);
	//cTool.ConfigureHw();
	cTool.ConfigureHw(); // make sure phase tuning works on (sometimes fails on second try)

	// ## TEST (Async counts:  )
	BeBoard* pBoard = cTool.fBoardVector.at(0);
	std::vector < ReadoutChip* > &ChipVec = pBoard->getModule(0)->fReadoutChipVector;
	/*cTool.setFWTestPulse();
	for(auto cSSA: ChipVec)
	{
		LOG (INFO) << BLUE << " -  --   --- Setting up Asynch mode for : " << std::bitset<8>(cSSA->getChipId()) << RESET;
		UserWriteReg(IB, cSSA, "ReadoutMode", 0x1);
		for (int i = 1; i<=120;i++ ) // loop over all strips
		{
			UserWriteReg(IB, cSSA, "ENFLAGS_S" + std::to_string(i), 0x0);
		}
		uint32_t thdac = 0;
		UserWriteReg(IB, cSSA, "ENFLAGS_S18", 0x15);
		UserWriteReg(IB, cSSA, "Bias_CALDAC", 200);
		while (thdac < 256)
		{
			IB->PS_Clear_counters();
			UserWriteReg(IB, cSSA, "Bias_THDAC", thdac);
			cTool.Start(0);
			cTool.Start(0);
			std::this_thread::sleep_for (std::chrono::milliseconds(50));
			cTool.Stop();
			uint8_t cRP1 = UserReadReg(IB, cSSA, "ReadCounter_LSB_S18");
			uint8_t cRP2 = UserReadReg(IB, cSSA, "ReadCounter_MSB_S18");
			uint16_t cRP = (cRP2*256) + cRP1; 

			LOG (INFO) << BOLDRED << "THDAC = " << thdac << ", HITS = " << cRP << RESET;
			h1->Fill(thdac, cRP);

			thdac++;

		}
	}
	TCanvas * c1 = new TCanvas("c", "c", 600, 600);
	c1->cd();
	h1->Draw("hist");
	c1->Print("test.png");
	*/

	//cTool.fReadoutChipInterface->WriteChipRegister("ReadoutMode", 0x0);


	TApplication cApp ( "Root Application", &argc, argv );
	LOG (INFO) << BOLDYELLOW << " - trying the calibration example -" << RESET;
	TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    cDirectory += "CalibrationExample";

    cTool.CreateResultDirectory ( cDirectory );
    cTool.InitResultFile ( "CalibrationResults" );
    cTool.StartHttpServer();
    //cTool.setFWTestPulse();
	for(auto cSSA: ChipVec)
	{
		cTool.fReadoutChipInterface->WriteChipReg(cSSA, "ReadoutMode", 0x0);
		//UserWriteReg(IB, cSSA, "ReadoutMode", 0x0);
		//UserWriteReg(IB, cSSA, "Bias_THDAC", 0x16);
		cTool.fReadoutChipInterface->WriteChipReg(cSSA, "Bias_THDAC", 0x16);
		for (int i = 1; i<=120;i++ ) // loop over all strips
		{
			UserWriteReg(IB, cSSA, "ENFLAGS_S" + std::to_string(i), 0x1);
		}
		UserWriteReg(IB, cSSA, "L1-Latency_LSB", 0x44);
	}
    CalibrationExample myCalibrationExample;
    LOG (INFO) << BOLDYELLOW << " - 1 -" << RESET;
    myCalibrationExample.Inherit (&cTool);
    LOG (INFO) << BOLDYELLOW << " - 2 -" << RESET;
    myCalibrationExample.Initialise ();
    LOG (INFO) << BOLDYELLOW << " - 3 -" << RESET;
    myCalibrationExample.runCalibrationExample();
    LOG (INFO) << BOLDYELLOW << " - 4 -" << RESET;


	//IB->PSInterfaceBoard_PowerOff_SSA(0, 0);
	cTool.Destroy();
}
