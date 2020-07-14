#include <cstring>

#include "Utils/Utilities.h"
#include "Utils/Timer.h"
#include "PSHybridTester.h"
#include "tools/SSASCurveAsync.h"
#include "tools/PedestalEqualization.h"
#include "tools/ShortFinder.h"
#include "tools/OpenFinder.h"
#include "tools/CicFEAlignment.h"
#include "tools/BackEndAlignment.h"
#include "tools/DataChecker.h"
#include "Utils/argvparser.h"

#ifdef __USE_ROOT__
    #include "TROOT.h"
    #include "TApplication.h"
#endif

#define __NAMEDPIPE__ 

#ifdef __NAMEDPIPE__
    #include "gui_logger.h"
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
   
    // cmd.defineOption ( "findOpens", "perform latency scan with antenna on UIB",  ArgvParser::NoOptionAttribute );
    // cmd.defineOption ( "findShorts", "look for shorts", ArgvParser::NoOptionAttribute );

    cmd.defineOption ( "threshold", "Threshold value to set on chips for open and short finding",  ArgvParser::OptionRequiresValue );
    cmd.defineOption ( "hybridId", "Serial Number of front-end hybrid. Default value: xxxx", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    
    cmd.defineOption("pattern", "Data Player Pattern", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequires*/);
    cmd.defineOptionAlternative("pattern", "p");

    cmd.defineOption ( "checkAsync", "Check async readout", ArgvParser::NoOptionAttribute );
    cmd.defineOption ( "checkAntenna",  "Check Antenna",  ArgvParser::NoOptionAttribute );
    
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
    //bool cTune = ( cmd.foundOption ( "tuneOffsets" ) ) ;
    //bool cMeasurePedeNoise = ( cmd.foundOption( "measurePedeNoise") ); 
    // bool cFindOpens = (cmd.foundOption ("findOpens") )? true : false;
    // bool cShortFinder = ( cmd.foundOption ( "findShorts" ) ) ? true : false;
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
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
    // Tool cTool;
    // cTool.InitializeHw ( cHWFile, outp);
    // cTool.InitializeSettings ( cHWFile, outp );
    // LOG (INFO) << outp.str();
    // cTool.CreateResultDirectory ( cDirectory );

    // hybrid testing tool 
    PSHybridTester cHybridTester;
    cHybridTester.InitializeHw ( cHWFile, outp);
    cHybridTester.InitializeSettings ( cHWFile, outp );
    cHybridTester.CreateResultDirectory ( cDirectory );
    cHybridTester.InitResultFile ( cResultfile );
    //set voltage  on PS FEH 
    cHybridTester.SetHybridVoltage();
    //check voltage on PS FEH 
    cHybridTester.CheckHybridVoltages();
    LOG (INFO) << outp.str();
    //select CIC readout 
    cHybridTester.SelectCIC(false);
    cHybridTester.ConfigureHw ();

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
    

    if( cmd.foundOption ( "checkAsync" ) )
    {
        DataChecker cDataChecker;
        cDataChecker.Inherit (&cHybridTester);
        cDataChecker.AsyncTest();
        //cDataChecker.resetPointers();
    }
    
    // // equalize thresholds on readout chips
    // if( cTune ) 
    // { 
        
    //     t.start();
    //     // now create a PedestalEqualization object
    //     PedestalEqualization cPedestalEqualization;
    //     cPedestalEqualization.Inherit (&cTool);
    //     // second parameter disables stub logic on CBC3
    //     cPedestalEqualization.Initialise ( true, true );
    //     cPedestalEqualization.FindVplus();
    //     cPedestalEqualization.FindOffsets();
    //     cPedestalEqualization.writeObjects();
    //     cPedestalEqualization.dumpConfigFiles();
    //     cPedestalEqualization.resetPointers();
    //     t.show ( "Time to tune the front-ends on the system: " );
    // }
    // #ifdef __TCUSB__
    // #endif


    // // measure noise on FE chips 
    // if (cMeasurePedeNoise)
    // {
    //     t.start();
    //     //if this is true, I need to create an object of type PedeNoise from the members of Calibration
    //     //tool provides an Inherit(Tool* pTool) for this purpose
    //     PedeNoise cPedeNoise;
    //     cPedeNoise.Inherit (&cTool);
    //     //second parameter disables stub logic on CBC3
    //     cPedeNoise.Initialise (true, true); // canvases etc. for fast calibration
    //     cPedeNoise.measureNoise();
    //     cPedeNoise.writeObjects();
    //     cPedeNoise.dumpConfigFiles();
    //     t.stop();
    //     t.show ( "Time to Scan Pedestals and Noise" );
    // }
    
    // if( cmd.foundOption ( "checkAntenna" ) )
    // {
    //     cTool.setSameDac("Threshold", 1);
    //     for(auto cBoard : *cTool.fDetectorContainer)
    //     {
    //         // read counters 
    //         BeBoard *cBeBoard = static_cast<BeBoard*>(cBoard);
    //         std::vector<uint32_t> cData(0);
    //         (static_cast<D19cFWInterface*>(cTool.fBeBoardInterface->getFirmwareInterface()))->ReadNEvents(cBeBoard, 100, cData);
    //         for( auto cDataWord : cData )
    //         {
    //             auto cCounter0 = (cDataWord & 0xFFFF);
    //             auto cCounter1 = (cDataWord & ((0xFFFF) << 16)) >> 16 ;
    //             LOG (INFO) << BOLDBLUE << std::bitset<32>(cDataWord) 
    //                 << "\t" << std::bitset<16>(cCounter0) << " [ " << +cCounter0  << " ]" 
    //                 << "\t" << std::bitset<16>(cCounter1) << " [ " << +cCounter1  << " ]"
    //                 << RESET;
    //         }
    //     }
    // }

    cHybridTester.SaveResults();
    cHybridTester.WriteRootFile();
    cHybridTester.CloseResultFile();
    cHybridTester.Destroy();

    if ( !batchMode ) cApp.Run();
    return 0;

}
