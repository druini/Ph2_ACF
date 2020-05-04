

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
#include "../tools/SSASCurveAsync.h"
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
int main ( int argc, char* argv[] )
{
	el::Configurations conf ("settings/logger.conf");
	el::Loggers::reconfigureAllLoggers (conf);	
	std::string cHWFile = "settings/D19C_2xSSA_PreCalib.xml";
	std::stringstream outp;
	Tool cTool;
	cTool.InitializeHw ( cHWFile, outp);
	cTool.InitializeSettings ( cHWFile, outp );
	cTool.ConfigureHw();
    std::string cDirectory = "Results/";
    cDirectory += "SSASCurveAsync";
    cTool.CreateResultDirectory ( cDirectory );
    cTool.InitResultFile ( "SSASCurveAsync" );
    SSASCurve cScurve;	
    cScurve.Inherit (&cTool);
    cScurve.Initialise();
    cScurve.run();
    cScurve.writeObjects();
    cScurve.dumpConfigFiles();
    cTool.WriteRootFile();
    cTool.CloseResultFile();
    cTool.Destroy();
    return 0;
}

