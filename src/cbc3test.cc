#include "SystemController.h"
#include "CommonVisitors.h"
#include "argvparser.h"
#include "Timer.h"
#include "UsbUtilities.h"

#include "BiasSweep.h"
#include "StubSweep.h"
#include "Calibration.h"
#include "PedeNoise.h"

#include "TROOT.h"
#include "TApplication.h"
#include <sys/wait.h>
#ifdef __USBINST__
#include <zmq.hpp>
#include "AppLock.cc"
#include "HMP4040Controller.h"
#include "HMP4040Client.h"
#include "Ke2110Controller.h"
using namespace Ph2_UsbInst;
#endif
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP
int main ( int argc, char** argv )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);
    ArgvParser cmd;
    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  system test application" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/HWDescription_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    //cmd.defineOption ( "sweep", "test the bias sweep tool", ArgvParser::NoOptionAttribute );
    //cmd.defineOptionAlternative ( "sweep", "s" );

    //cmd.defineOption ( "stubs", "test the stub sweep tool", ArgvParser::NoOptionAttribute );

    cmd.defineOption ( "events", "Number of Events . Default value: 10", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "events", "e" );

    cmd.defineOption ( "vcth", "Threshold in VCth units (hex (including 0x) or decimal) . Default values from HW description .XML file", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "vcth", "v" );

    cmd.defineOption ( "dqm", "Print every i-th event.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "dqm", "d" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/Cbc3HWDescription.xml";
    //bool cConfigure = ( cmd.foundOption ( "configure" ) ) ? true : false;
    uint32_t pEventsperVcth = ( cmd.foundOption ( "events" ) ) ? convertAnyInt ( cmd.optionValue ( "events" ).c_str() ) : 10000;
    bool cVcthset = cmd.foundOption ("vcth");
    uint16_t cVcth = ( cmd.foundOption ( "vcth" ) ) ? convertAnyInt ( cmd.optionValue ( "vcth" ).c_str() ) : 200;
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    //bool cSweep = ( cmd.foundOption ( "sweep" ) ) ? true : false;
    //bool cStubSweep = ( cmd.foundOption ( "stubs" ) ) ? true : false;

    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    std::stringstream outp;

    //if (cSweep || cStubSweep)
    //{
    Tool cTool;
    cTool.InitializeHw ( cHWFile, outp );
    cTool.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");
    //cTool.CreateResultDirectory ( cDirectory, false, false );
    //cTool.InitResultFile (cResultfile);
    //cTool.StartHttpServer (8084);
    cTool.ConfigureHw ();

    BeBoard* pBoard = cTool.fBoardVector.at ( 0 );
    uint32_t cN = 1;
    //uint32_t cNthAcq = 0;
    //uint32_t count = 0;


    if (cVcthset)
    {
        ThresholdVisitor cVisitor (cTool.fCbcInterface, 0);
        cVisitor.setThreshold (cVcth);
        pBoard->accept (cVisitor);
    }

    //std::this_thread::sleep_for (std::chrono::milliseconds (500) );

    cTool.ReadNEvents (pBoard, pEventsperVcth);

    const std::vector<Event*>& pEvents = cTool.GetEvents ( pBoard );

    for ( auto& ev : pEvents )
    {
        LOG (INFO) << ">>> Event #" << cN++ << " Threshold: " << cVcth;
        outp.str ("");
        outp << *ev;
        LOG (INFO) << outp.str();
    }

    //cNthAcq++;
    //////////////////////////////////////////////////////////////////////////
    //cTool.fBeBoardInterface->Start ( pBoard );

    //while ( cN <= pEventsperVcth )
    //{
    //uint32_t cPacketSize = cTool.ReadData ( pBoard );

    //if ( cN + cPacketSize >= pEventsperVcth )
    //cTool.fBeBoardInterface->Stop ( pBoard );

    //const std::vector<Event*>& events = cTool.GetEvents ( pBoard );

    //for ( auto& ev : events )
    //{
    //count++;
    //cN++;

    //if ( cmd.foundOption ( "dqm" ) )
    //{
    //if ( count % atoi ( cmd.optionValue ( "dqm" ).c_str() ) == 0 )
    //{
    //LOG (INFO) << ">>> Event #" << count ;
    //outp.str ("");
    //outp << *ev << std::endl;
    //LOG (INFO) << outp.str();
    //}
    //}

    //if ( count % 1000  == 0 )
    //LOG (INFO) << ">>> Recorded Event #" << count ;
    //}

    //cNthAcq++;
    //}

    cTool.Destroy();

    LOG (INFO) << "*** End of the System test ***" ;

    if ( !batchMode )
        cApp.Run();


    return 0;
}
