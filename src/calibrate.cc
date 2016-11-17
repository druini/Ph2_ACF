#include <cstring>
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../tools/Calibration.h"
#include "../tools/PedeNoise.h"
#include "../Utils/argvparser.h"
#include "TROOT.h"
#include "TApplication.h"
#include "../Utils/Timer.h"


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
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  calibration routine using K. Uchida's algorithm or a fast algorithm" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/Calibration2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "skip", "skip scaning VCth vs Vplus", ArgvParser::NoOptionAttribute );
	 cmd.defineOptionAlternative ( "skip", "s" );
    
	 //cmd.defineOption( "old", "Use old calibration algorithm", ArgvParser::NoOptionAttribute );
    //cms.defineOptionAlternative ("old", "v" );

    cmd.defineOption ( "noise", "Perform noise scan after Offset tuning", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "noise", "n" );

    //cmd.defineOption ( "allChan", "Do calibration using all channels? Default: false", ArgvParser::NoOptionAttribute );
    //cmd.defineOptionAlternative ( "allChan", "a" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );


    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/Calibration2CBC.xml";
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    cDirectory += "Calibration";
    bool cVplus = ( cmd.foundOption ( "skip" ) ) ? false : true;
    //bool cOld = ( cmd.foundOption( "old" ) ) ? true : false;

    //bool cCalibrateTGrp = ( cmd.foundOption ( "allChan" ) ) ? true : false;
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    bool cNoiseScan = ( cmd.foundOption ("noise") ) ? true : false;

    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    Timer t;

    //create a genereic Tool Object, I can then construct all other tools from that using the Inherit() method
    //this tool stays on the stack and lives until main finishes - all other tools will update the HWStructure from cTool
    Tool cTool;
    std::stringstream outp;
    cTool.InitializeHw ( cHWFile, outp );
    cTool.InitializeSettings ( cHWFile, outp );
    cTool.CreateResultDirectory ( cDirectory );
    cTool.InitResultFile ( "CalibrationResults" );
    cTool.StartHttpServer();
    cTool.ConfigureHw (outp);
    LOG (INFO) << outp.str();
    outp.str ("");
    //if ( !cOld )
    //{
    t.start();

    // now create a calibration object 
    Calibration cCalibration;
    cCalibration.Inherit (&cTool);
    //cCalibration->ConfigureHw();
    cCalibration.Initialise ( false );

    if ( cVplus ) cCalibration.FindVplus();

    cCalibration.FindOffsets();
    cCalibration.SaveResults();
    cCalibration.dumpConfigFiles();
    t.stop();
    t.show ( "Time to Calibrate the system: " );

    if (cNoiseScan)
    {
        t.start();
        //if this is true, I need to create an object of type PedeNoise from the members of Calibration
        //tool provides an Inherit(Tool* pTool) for this purpose
        PedeNoise cPedeNoise;
        cPedeNoise.Inherit (&cTool);
        cPedeNoise.ConfigureHw (outp);
        LOG (INFO) << outp.str();
        outp.str ("");
        cPedeNoise.Initialise(); // canvases etc. for fast calibration
        cPedeNoise.measureNoise();
        cPedeNoise.Validate();
        cPedeNoise.SaveResults( );
        cPedeNoise.dumpConfigFiles();
        t.stop();
        t.show ( "Time to Scan Pedestals and Noise" );
    }
    cTool.SaveResults();
    cTool.CloseResultFile();
    cTool.Destroy();

    if ( !batchMode ) cApp.Run();

    return 0;
}
