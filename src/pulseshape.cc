#include <cstring>

#include "../Utils/Utilities.h"
#include "../tools/PulseShape.h"
#include <TApplication.h>
#include "../Utils/argvparser.h"
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

    // init
    ArgvParser cmd;

    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  PulseShape tool to perform the following procedures:\n-Print scan test pulse delay" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/PulseShape.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "step", "Scan the delay with treshold scan step size, default value: 1", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "step", "s" );



    cmd.defineOption ( "output", "Output Directory . Default value: Results/", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/PulseShape.xml";
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    cDirectory += "PulseShape";
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    uint8_t cScanStep = ( cmd.foundOption ( "step" ) ) ? convertAnyInt ( cmd.optionValue ( "step" ).c_str() ) : 1;

    uint32_t cSelectedTestGroup = ( cmd.foundOption ( "testGroup" ) ) ? convertAnyInt ( cmd.optionValue ( "testGroup" ).c_str() ) : 1;

    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    PulseShape cPulseShape;

    std::stringstream outp;
    cPulseShape.InitializeHw ( cHWFile, outp);
    cPulseShape.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");

    cPulseShape.Initialize();

    cPulseShape.CreateResultDirectory ( cDirectory );
    std::string cResultfile;
    cResultfile = "PulseShape";
    cPulseShape.InitResultFile ( cResultfile );
    cPulseShape.ConfigureHw ();

    cPulseShape.ScanTestPulseDelay ( cScanStep );
    cPulseShape.SaveResults();

    cPulseShape.Destroy();

    if ( !batchMode ) cApp.Run();

    return 0;

}
