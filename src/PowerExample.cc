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
	D19cFWInterface* IB = dynamic_cast<D19cFWInterface*>(cTool.fBeBoardFWMap.find(0)->second); // There has to be a better way!
	IB->PSInterfaceBoard_PowerOn_SSA(1.25, 1.25, 1.25, 0.3, 0.0, 145);
	IB->ReadPower_SSA();

}
