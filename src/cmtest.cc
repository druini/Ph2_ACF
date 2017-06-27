#include <cstring>
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../tools/CMTester.h"
#include <TApplication.h>
#include "../Utils/argvparser.h"
//#include "../Utils/easylogging++.h"
#include "TROOT.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP

int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  applicaton to take data and analyze it with respect to common mode noise" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/CMTest2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "output", "Output Directory . Default value: Results/", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "scan", "scan for noisy strips", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "scan", "s" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );

    cmd.defineOption ( "gui", "option only suitable when launching from gui", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "gui", "g" );


    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    bool isGui = ( cmd.foundOption ( "gui" ) ) ? true : false;

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/CMTest2CBC.xml";
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    cDirectory += "CMTest";
    bool cScan = ( cmd.foundOption ( "scan" ) ) ? true : false;
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;


    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    std::stringstream outp;
    CMTester cTester;
    cTester.InitializeHw ( cHWFile, outp );
    cTester.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");
    cTester.CreateResultDirectory ( cDirectory );
    cTester.InitResultFile ( "CMTest" );
    cTester.StartHttpServer ( 8082 );

    cTester.Initialize ();

    if ( !isGui ) cTester.ConfigureHw ();


    // Here comes our Part:
    if ( cScan ) cTester.ScanNoiseChannels();

    cTester.TakeData();
    cTester.FinishRun();
    cTester.SaveResults();
    cTester.CloseResultFile();
    cTester.Destroy();

    if ( !batchMode ) cApp.Run();

    return 0;

}
