#include <iostream>
#include "Utilities.h"
#include "argvparser.h"
#include "ArdNanoController.h"
#include "StubSweep.h"

using namespace Ph2_UsbInst;
using namespace CommandLineProcessing;

INITIALIZE_EASYLOGGINGPP

std::atomic<bool> fQuit;

int main (int argc, char** argv)
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    //CMD line parsing goes here!
    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_Arduino Driver and Monitoring library") ;
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "send", "Send single character to serial port", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "send", "s" );

    cmd.defineOption ( "blink", "Test connection to ARDUINO board by making on-board LED blink on/off N times.", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "blink", "b" );

    cmd.defineOption ( "relay", "Control relay connected to Arduino board [on/off]", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "relay", "r" );


    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        std::cout << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    bool cBlink = ( cmd.foundOption ( "blink" ) ) ? true : false;
    bool cRelay = ( cmd.foundOption ( "relay" ) ) ? true : false;
    std::string cSerialCommand = ( cmd.foundOption ( "send" ) ) ? cmd.optionValue ( "send" ) : "";

    bool cAsync = false;
    bool cMultex = false;
    ArdNanoController* cController = new ArdNanoController ();
    bool cState = cController->CheckArduinoState();


    if ( cState)
    {
        if ( !cSerialCommand.empty() )
            cController->Write (cSerialCommand.c_str() );

        if ( cBlink )
        {
            //mypause(); // here so I can start sniffer...
            for ( unsigned int i = 0 ; i < atoi ( cmd.optionValue ("blink").c_str() )  ; i++ )
            {
                cController->ControlLED (1); //cController->Write("");
                std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
                cController->ControlLED (0); //cController->Write("");
                std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
            }
        }

        if ( cRelay )
        {
            uint8_t cRelayState = ( cmd.optionValue ( "relay" ) == "on" || cmd.optionValue ( "relay" ) == "ON" || cmd.optionValue ( "relay" ) == "On") ? 1 : 0;
            LOG (INFO) << "Controling relay connected to Arduino nano .... switching it : " << BOLDGREEN << cmd.optionValue ( "relay" ) <<  RESET << " !!";
            cController->ControlRelay (cRelayState);

            std::string cReply = ( cController->GetRelayState() == 1 ) ? "On" : "Off" ;
            LOG (INFO) << "Relay now.... " << cReply;

        }

    }

    if (cController) delete cController;
}
