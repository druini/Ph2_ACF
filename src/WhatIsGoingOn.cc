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
	el::Configurations conf ("settings/logger.conf");
	el::Loggers::reconfigureAllLoggers (conf);	
	std::string cHWFile = "settings/D19C_2xSSA_Calib.xml";
	std::stringstream outp;
	LOG (INFO) << BOLDRED << "== 1 =============" << RESET;
	Tool cTool;
	LOG (INFO) << BOLDRED << "== 2 =============" << RESET;
	cTool.InitializeHw ( cHWFile, outp);
	LOG (INFO) << BOLDRED << "== 3 =============" << RESET;
	cTool.InitializeSettings ( cHWFile, outp );
	LOG (INFO) << BOLDRED << "== 4 =============" << RESET;
	D19cFWInterface* IB = dynamic_cast<D19cFWInterface*>(cTool.fBeBoardFWMap.find(0)->second); // There has to be a better way!
	LOG (INFO) << BOLDRED << "== 5 =============" << RESET;
	cTool.ConfigureHw();	
	LOG (INFO) << BOLDRED << "== 6 =============" << RESET;
}
