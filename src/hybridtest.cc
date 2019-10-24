#include <cstring>
#include "../HWDescription/Chip.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/ChipInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../tools/HybridTester.h"
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
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  Hybrid validation test performs the following actions:\n-Test individual CBC registers one by one by writing complimentary bit patterns 0x55 and 0xAA\n-scan the threshold range and measure the noise occupancy to determine the pedestal and identify a threshold with ~0 noise occupancy\n-measure the single-strip efficiency under the influence of an external signal source to identify bad connections" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/HybridTest8CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "registers", "test registers", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "registers", "r" );

    cmd.defineOption ( "shorts", "look for shorts", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "shorts", "t" );

    cmd.defineOption ( "scan", "scan noise occupancy, if not set, the threshold from the .XML will be used", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "scan", "s" );

    cmd.defineOption ( "output", "Output Directory . Default value: Results/", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );

    cmd.defineOption ( "antenna", "Run the antenna testing routine", ArgvParser::OptionRequiresValue);
    cmd.defineOptionAlternative ( "antenna", "a" );

    cmd.defineOption ( "id", "Hybrid's ID . Default value: -1", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "id", "i" );


    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/HybridTest8CBC.xml";
    std::string cHybridId = ( cmd.foundOption ( "id" ) ) ? cmd.optionValue ( "id" ) : "-1";
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;

    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    cDirectory += "HybridTest";

    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    #ifdef __USE_ROOT__
        bool cRegisters = ( cmd.foundOption ( "registers" ) ) ? true : false;
        bool cShorts = ( cmd.foundOption ( "shorts" ) ) ? true : false;
        bool cScan = ( cmd.foundOption ( "scan" ) ) ? true : false;
        bool cAntenna = ( cmd.foundOption ( "antenna" ) ) ? true : false;

        HybridTester cHybridTester;

        std::stringstream outp;
        cHybridTester.InitializeHw ( cHWFile, outp );
        cHybridTester.InitializeSettings ( cHWFile, outp );
        LOG (INFO) << outp.str();
        cHybridTester.Initialize ( cScan );
        outp.str ("");
        cHybridTester.CreateResultDirectory ( cDirectory );
        cHybridTester.InitResultFile ( "HybridTest" );
        cHybridTester.StartHttpServer();
        cHybridTester.ConfigureHw ();


    // Here comes our Part:

    if (cAntenna)
    {
        #ifdef __ANTENNA__
            //cHybridTester.Initialize( cScan );
            //cHybridTester.Initialize( cScan );
            cHybridTester.AntennaScan(cAntennaPotential);
            cHybridTester.SaveTestingResults ( cHybridId );
        #else
            LOG (INFO) << "This feature is only available if the CMSPh2_AntennaDriver package is installed. It requires a recent version of libusb -devel and can be downloaded from: 'https://github.com/gauzinge/CMSPh2_AntennaDriver.git'" ;
        #endif
    }

    if ( !cAntenna && !cRegisters && !cShorts)
    {
        //cHybridTester.Initialize( cScan );
        //cHybridTester.Initialize( cScan );
        cHybridTester.Measure();
    }

    //std::cout << "Test Registers " << cRegisters << " , scan threshold " << cScan << std::endl;
    if ( cRegisters ) cHybridTester.TestRegisters();

    if ( cShorts ) cHybridTester.FindShorts();

    if ( cScan )
    {
        //Scan threshold to find pedestal
        cHybridTester.ScanThreshold();

        // Wait for user to acknowledge and turn on external Source!
        std::cout << "Identified the threshold for 0 noise occupancy - Start external Signal source!" << std::endl;
        mypause();
        cHybridTester.Measure();
    }

    cHybridTester.writeObjects();
    cHybridTester.SaveResults();
    cHybridTester.CloseResultFile();
    cHybridTester.Destroy();
    #endif

    if ( !batchMode ) cApp.Run();

    return 0;
}
