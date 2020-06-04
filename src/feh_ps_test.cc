#include <cstring>

#include "Utils/Utilities.h"
#include "Utils/Timer.h"
#include "tools/SSASCurveAsync.h"
#include "tools/ShortFinder.h"
#include "tools/OpenFinder.h"
#include "tools/CicFEAlignment.h"
#include "tools/BackEndAlignment.h"
#include "tools/DataChecker.h"
#include "Utils/argvparser.h"
#include "ExtraChecks.h"

#ifdef __USE_ROOT__
    #include "TROOT.h"
    #include "TApplication.h"
#endif

#define __NAMEDPIPE__ 

#ifdef __NAMEDPIPE__
    #include "gui_logger.h"
#endif


#ifdef __TCUSB__
  #include "USB_a.h"
#endif


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP

#define CHIPSLAVE 4

int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  Commissioning tool to perform the following procedures:\n-Timing / Latency scan\n-Threshold Scan\n-Stub Latency Scan" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/Commission_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "tuneOffsets", "tune offsets on readout chips connected to CIC.");
    cmd.defineOptionAlternative ( "tuneOffsets", "t" );
    
    cmd.defineOption ( "measurePedeNoise", "measure pedestal and noise on readout chips connected to CIC.");
    cmd.defineOptionAlternative ( "measurePedeNoise", "m" );
   
    cmd.defineOption ( "findOpens", "perform latency scan with antenna on UIB",  ArgvParser::NoOptionAttribute );
    cmd.defineOption ( "findShorts", "look for shorts", ArgvParser::NoOptionAttribute );

    cmd.defineOption ( "threshold", "Threshold value to set on chips for open and short finding",  ArgvParser::OptionRequiresValue );
    cmd.defineOption ( "hybridId", "Serial Number of front-end hybrid. Default value: xxxx", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    
    cmd.defineOption("pattern", "Data Player Pattern", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequires*/);
    cmd.defineOptionAlternative("pattern", "p");

    // general 
    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/Commissioning.xml";
    bool cTune = ( cmd.foundOption ( "tuneOffsets" ) ) ;
    bool cMeasurePedeNoise = ( cmd.foundOption( "measurePedeNoise") ); 
    bool cFindOpens = (cmd.foundOption ("findOpens") )? true : false;
    bool cShortFinder = ( cmd.foundOption ( "findShorts" ) ) ? true : false;
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    uint32_t  cThreshold = ( cmd.foundOption ( "threshold" ) )   ?  convertAnyInt ( cmd.optionValue ( "threshold" ).c_str() ) :  560 ;
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    std::string cHybridId = ( cmd.foundOption ( "hybridId" ) ) ? cmd.optionValue ( "hybridId" ) : "xxxx";
    cDirectory += Form("FEH_2S_%s",cHybridId.c_str());
    
    TApplication cApp ( "Root Application", &argc, argv );
    
    if ( batchMode ) 
        gROOT->SetBatch ( true );
    else 
        TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    std::string cResultfile = "Hybrid";
    Timer t;

    #ifdef __TCUSB__
    #endif
    
    std::stringstream outp;
    Tool cTool;
    cTool.InitializeHw ( cHWFile, outp);
    cTool.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    cTool.CreateResultDirectory ( cDirectory );
    cTool.InitResultFile ( cResultfile );
    cTool.ConfigureHw ();

    // measure hybrid current and temperature 
    #ifdef __TCUSB__
    #endif

    // align back-end 
    /*BackEndAlignment cBackEndAligner;
    cBackEndAligner.Inherit (&cTool);
    cBackEndAligner.Start(0);
    //reset all chip and board registers 
    // to what they were before this tool was called 
    cBackEndAligner.Reset(); 
    */
    // if CIC is enabled then align CIC first 
    /*
    CicFEAlignment cCicAligner;
    cCicAligner.Inherit (&cTool);
    cCicAligner.Start(0);
    //reset all chip and board registers 
    // to what they were before this tool was called 
    cCicAligner.Reset(); 
    cCicAligner.dumpConfigFiles();
    */
    

    
    
    // equalize thresholds on readout chips
    if( cTune ) 
    { 
        t.start();
        SSASCurve cScurve;  
        cScurve.Inherit (&cTool);
        cScurve.Initialise();
        cScurve.run();
        cScurve.writeObjects();
        cScurve.dumpConfigFiles();
    }

    #ifdef __TCUSB__
    #endif


    // measure noise on FE chips 
    if (cMeasurePedeNoise)
    {
        t.start();
    }
    
    // For next step... set all thresholds on CBCs to 560 
    if( cmd.foundOption ( "threshold" )  )
    { 
        cTool.setSameDac("VCth", cThreshold);
        LOG (INFO) << BOLDBLUE << "Threshold for next steps is set to " << +cThreshold << " DAC units." << RESET;
    }
    // Inject charge with antenna circuit and look for opens 
    if ( cFindOpens )
    {
        #ifdef __ANTENNA__
            int  cAntennaDelay = ( cmd.foundOption ( "antennaDelay" ) )   ?  convertAnyInt ( cmd.optionValue ( "antennaDelay" ).c_str() ) : -1;
            int  cLatencyRange = ( cmd.foundOption ( "latencyRange" ) )   ?  convertAnyInt ( cmd.optionValue ( "latencyRange" ).c_str() ) :  -1;
    
            OpenFinder::Parameters cOfp;
            // hard coded for now TODO: make this configurable
            cOfp.potentiometer = 0x265;
            // antenna group 
            auto cSetting = cTool.fSettingsMap.find ( "AntennaGroup" );
            cOfp.antennaGroup = ( cSetting != std::end ( cTool.fSettingsMap ) ) ? cSetting->second : (0);
            
            // antenna delay 
            if( cAntennaDelay > 0 )
                cOfp.antennaDelay = cAntennaDelay;
            else
            {
                auto cSetting = cTool.fSettingsMap.find ( "AntennaDelay" );
                cOfp.antennaDelay = ( cSetting != std::end ( cTool.fSettingsMap ) ) ? cSetting->second : (200);
            }
            
            // scan range for latency  
            if( cLatencyRange > 0 )
                cOfp.latencyRange = cLatencyRange;
            else
            {
                auto cSetting = cTool.fSettingsMap.find ( "ScanRange" );
                cOfp.latencyRange = ( cSetting != std::end ( cTool.fSettingsMap ) ) ? cSetting->second : (10);
            }

            
            OpenFinder cOpenFinder;
            cOpenFinder.Inherit (&cTool);
            cOpenFinder.Initialise (cOfp);
            LOG (INFO) << BOLDBLUE << "Starting open finding measurement [antenna potentiometer set to 0x" << std::hex << cOfp.potentiometer << std::dec << " written to the potentiometer" <<  RESET;
            cOpenFinder.FindOpens();
        #endif
    }   
    //inject charge with TP and look for shorts 
    if ( cShortFinder )
    {
        ShortFinder cShortFinder;
        cShortFinder.Inherit (&cTool);
        cShortFinder.Initialise ();
        cShortFinder.Start();
        cShortFinder.Stop();
    }

    cTool.SaveResults();
    cTool.WriteRootFile();
    cTool.CloseResultFile();
    cTool.Destroy();

    if ( !batchMode ) cApp.Run();
    return 0;

}