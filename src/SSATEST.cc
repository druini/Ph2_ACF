#include <cstring>
#include <iostream>
#include <fstream>
#include "../Utils/Utilities.h"
#include "../HWDescription/SSA.h"
#include "../HWDescription/OuterTrackerModule.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/MPAInterface.h"
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
#include "TH1.h"
#include "TCanvas.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

int main( int argc, char* argv[] )
// Tests functionality of SSA implementation in Ph2_ACF
{

	el::Configurations conf ("settings/logger.conf");
	el::Loggers::reconfigureAllLoggers (conf);	
	LOG (INFO) << BOLDBLUE << "========================================" << RESET;
	std::string cHWFile = "settings/D19CDescriptionSSA.xml";std::stringstream outp; // Next 4 lines, configure Ph2_ACF to look at this hardware
	Tool cTool;
	cTool.InitializeHw ( cHWFile, outp); 
	cTool.InitializeSettings ( cHWFile, outp ); 
	BeBoard* pBoard = cTool.fBoardVector.at(0); 
	pBoard->setFrontEndType(FrontEndType::SSA); // FIXME Make sure the board knows that this is an SSA. Unclear why this isn't done automatically.
	SSAInterface* fSSAInterface = cTool.fSSAInterface;

	fSSAInterface->PowerDiagnostic(); // Prints current power consumption.
	fSSAInterface->MainPowerOn(); // flip main power on!
	std::this_thread::sleep_for (std::chrono::milliseconds (1000) ); // Wait a bit, probably 5s is longer than needed, but better safe than sorry.
	fSSAInterface->PowerOn();
	fSSAInterface->PowerDiagnostic();	
	std::this_thread::sleep_for (std::chrono::milliseconds (1000) ); // Again, may be longer than needed, but we know from the python power-on code that this isn't instantaneous.
	LOG (INFO) << BOLDRED << "SSA POWERED ON" << RESET;

	cTool.ConfigureHw();
	cTool.fBeBoardInterface->getBoardInfo(pBoard);
	std::vector < SSA* > cSSAVector = static_cast<OuterTrackerModule*>(pBoard->getModule(0))->fSSAVector;


}
