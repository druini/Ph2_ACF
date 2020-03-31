#include <cstring>
#include "MultiplexingSetup.h"
#include "CicFEAlignment.h"
#include "BackEndAlignment.h"
#include "Utils/argvparser.h"

#ifdef __USE_ROOT__
    #include "TROOT.h"
    #include "TApplication.h"
#endif

#include "../Utils/Timer.h"

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
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF d19c Testboard Firmware Test Application" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );
    cmd.defineOption ( "file", "Hw Description File . Default value: settings/D19CHWDescription.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );
    cmd.defineOption ( "mux_disconnect", "Disable setup with multiplexing backplane", ArgvParser::NoOptionAttribute );
    cmd.defineOption ( "mux_scan", "Scan setup with multiplexing backplane", ArgvParser::NoOptionAttribute );
    cmd.defineOption ( "mux_configure", "Configure setup with multiplexing backplane", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOption ( "mux_auto_configure", "Configure all available cards one by one", ArgvParser::NoOptionAttribute );
    cmd.defineOption ( "enable_fmc_power", "Initialization of FMC power", ArgvParser::NoOptionAttribute );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    bool cMuxDisconnect = ( cmd.foundOption ( "mux_disconnect" ) ) ? true : false;
    bool cMuxScan = ( cmd.foundOption ( "mux_scan" ) ) ? true : false;
    bool cMuxConfigure = ( cmd.foundOption ( "mux_configure" ) ) ? true : false;
    bool cMuxAutoConfigure = ( cmd.foundOption ( "mux_auto_configure" ) ) ? true : false;
    bool cPowerEnable = ( cmd.foundOption ( "enable_fmc_power" ) ) ? true : false;

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/D19CHWDescription.xml";

    
    std::stringstream outp;
    Tool cTool;
    cTool.InitializeHw ( cHWFile, outp);
    cTool.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    #ifdef __MULTIPLEXING__
        MultiplexingSetup cMuxControl;
        cMuxControl.Inherit (&cTool);
        cMuxControl.Initialise ();
        if ( cMuxDisconnect ) 
        {
            cMuxControl.Disconnect();
        } 
        else if ( cMuxScan ) 
        {
            cMuxControl.Scan();
        } 
        else if ( cMuxConfigure ) 
        {
            std::string sBPNumCardNum = cmd.optionValue ( "mux_configure" );
            std::vector<int> vBPNumCardNum;
            std::stringstream ssBPNumCardNum( sBPNumCardNum );
            int i;
            while ( ssBPNumCardNum >> i )
            {
                vBPNumCardNum.push_back( i );
                if ( ssBPNumCardNum.peek() == ',' ) ssBPNumCardNum.ignore();
            };
            uint8_t cBackplaneNum = vBPNumCardNum.at(0);
            uint8_t cCardNum = vBPNumCardNum.at(1);
            cMuxControl.ConfigureSingleCard(cBackplaneNum, cCardNum);
        } 
        else if ( cMuxAutoConfigure ) 
        {
            cMuxControl.ConfigureAll();
        }    
    #endif //__MULTIPLEXING__
    
    // S.S. haven't tested this yet 
    // if ( cPowerEnable ) 
    // {
    //     cTool.fBeBoardInterface->setBoard ( pBoard->getBeBoardId() );
    //     dynamic_cast<D19cFWInterface*>(cTool.fBeBoardInterface->getFirmwareInterface())->InitFMCPower();
    // }

    // if you've tried to configure the mux .. then try and configure hardware that is on it 
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    // if( cMuxConfigure || cMuxAutoConfigure )
    // {
    //     cTool.ConfigureHw ();
    //     cTool.CreateResultDirectory ( cDirectory );
    //     cTool.InitResultFile ( "CicResults" );
    //     //t.start();

    //     //CIC FE alignment tool 
    //     CicFEAlignment cCicAligner;
    //     cCicAligner.Inherit (&cTool);
    //     cCicAligner.Initialise ();
    //     bool cPhaseAligned = cCicAligner.PhaseAlignment(50);
    //     if( !cPhaseAligned ) 
    //     {
    //         LOG (INFO) << BOLDRED << "FAILED " << BOLDBLUE << " phase alignment step on CIC input .. " << RESET; 
    //         exit(0);
    //     }
    //     LOG (INFO) << BOLDGREEN << "SUCCESSFUL " << BOLDBLUE << " phase alignment on CIC inputs... " << RESET; 
    //     bool cWordAligned = cCicAligner.WordAlignment(false);
    //     if( !cWordAligned ) 
    //     {
    //         LOG (INFO) << BOLDRED << "FAILED " << BOLDBLUE << "word alignment step on CIC input .. " << RESET; 
    //         exit(0);
    //     }
    //     LOG (INFO) << BOLDGREEN << "SUCCESSFUL " << BOLDBLUE << " word alignment on CIC inputs... " << RESET; 
        
    //     // align back-end .. if this moves to firmware then we can get rid of this step 
    //     BackEndAlignment cBackEndAligner;
    //     cBackEndAligner.Inherit (&cTool);
    //     cBackEndAligner.Initialise();
    //     bool cAligned = cBackEndAligner.Align();
    //     cBackEndAligner.resetPointers();
    //     if(!cAligned )
    //     {
    //         LOG (ERROR) << BOLDRED << "FAILED " << BOLDBLUE << " to align back-end " << RESET; 
    //         exit(0);
    //     }

    // }
    LOG (INFO) << "*** End of the operation ***" ;
    cTool.Destroy();

    return 0;
}
