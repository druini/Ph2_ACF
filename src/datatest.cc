#include <cstring>
#include <fstream>
#include "../Utils/Utilities.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/Timer.h"
#include <inttypes.h>
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/SLinkEvent.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);


    int pEventsperVcth;
    int cVcth;

    SystemController cSystemController;
    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  Data acquisition test and Data dump" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "ignoreI2c", "Ignore I2C configuration of CBCs. Allows to run acquisition on a bare board without CBC." );
    cmd.defineOptionAlternative ( "ignoreI2c", "i" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/HWDescription_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "vcth", "Threshold in VCth units (hex (including 0x) or decimal) . Default values from HW description .XML file", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "vcth", "v" );

    cmd.defineOption ( "events", "Number of Events . Default value: 10", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "events", "e" );

    cmd.defineOption ( "save", "Save the data to a raw file.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "save", "s" );

    cmd.defineOption ( "daq", "Save the data into a .daq file using the phase-2 Tracker data format.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "daq", "d" );

    cmd.defineOption ( "read", "Read the data from a raw file instead of the board.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "read", "r" );

    // cmd.defineOption( "option", "Define file access mode: w : write , a : append, w+ : write/update", ArgvParser::OptionRequiresValue );
    // cmd.defineOptionAlternative( "option", "o" );


    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    bool cSaveToFile = false;
    std::string cOutputFile;
    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/HWDescription_2CBC.xml";

    if ( cmd.foundOption ( "save" ) )
        cSaveToFile = true ;

    if ( cSaveToFile )
    {
        cOutputFile =  cmd.optionValue ( "save" );
        cSystemController.addFileHandler ( cOutputFile, 'w' );
        LOG (INFO) << "Writing Binary Rawdata to:   " << cOutputFile ;
    }

    std::string cDAQFileName;
    FileHandler* cDAQFileHandler = nullptr;
    bool cDAQFile = cmd.foundOption ("daq");

    if (cDAQFile)
    {
        cDAQFileName = cmd.optionValue ("daq");
        cDAQFileHandler = new FileHandler (cDAQFileName, 'w');
        LOG (INFO) << "Writing DAQ File to:   " << cDAQFileName << " - ConditionData, if present, parsed from " << cHWFile ;
    }

    cVcth = ( cmd.foundOption ( "vcth" ) ) ? convertAnyInt ( cmd.optionValue ( "vcth" ).c_str() ) : 0;
    pEventsperVcth = ( cmd.foundOption ( "events" ) ) ? convertAnyInt ( cmd.optionValue ( "events" ).c_str() ) : 10;

    Timer t;
    t.start();

    std::stringstream outp;
    cSystemController.InitializeHw ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");

    if (!cmd.foundOption ("read") )
        cSystemController.ConfigureHw ( cmd.foundOption ( "ignoreI2c" ) );

    t.stop();
    t.show ( "Time to Initialize/configure the system: " );

    if ( cVcth != 0 )
    {
        t.start();
        ThresholdVisitor cVisitor (cSystemController.fCbcInterface, 0);
        cVisitor.setThreshold (cVcth);
        cSystemController.accept (cVisitor);

        t.stop();
        t.show ( "Time for changing VCth on all CBCs:" );
    }

    BeBoard* pBoard = cSystemController.fBoardVector.at ( 0 );
    t.start();
    // make event counter start at 1 as does the L1A counter
    uint32_t cN = 1;
    uint32_t cNthAcq = 0;

    Counter cCbcCounter;
    pBoard->accept ( cCbcCounter );
    Data data;

    if (!cmd.foundOption ( "read") )
        cSystemController.fBeBoardInterface->Start ( pBoard );

    const std::vector<Event*>* pEvents ;

    while ( cN <= pEventsperVcth )
    {
        if (cmd.foundOption ( "read") )
        {
            //FileHandler fFile (cmd.optionValue ("read"), 'r');
            ////TODO
            ////uint32_t cEventSize = ;
            ////std::vector<uint32_t> cReadVec = fFile.readFileChunks (cEventSize * pEventsperVcth);
            //std::vector<uint32_t> cReadVec = fFile.readFileChunks (940000);
            //data.Set ( pBoard, cReadVec, 10000, cSystemController.fBeBoardInterface->getBoardType (pBoard) );
            //pEvents = &data.GetEvents ( pBoard);
            LOG (ERROR) << "Read option currently not supported!";
        }
        else
        {
            uint32_t cPacketSize = cSystemController.ReadData ( pBoard );


            if ( cN + cPacketSize > pEventsperVcth )
                cSystemController.fBeBoardInterface->Stop ( pBoard );

            pEvents = &cSystemController.GetEvents ( pBoard );
        }

        for ( auto& ev : *pEvents )
        {
            LOG (INFO) << ">>> Event #" << cN++ ;
            LOG (INFO) << *ev;

            if (cDAQFile)
            {
                SLinkEvent cSLev = ev->GetSLinkEvent (pBoard);
                cDAQFileHandler->set (cSLev.getData<uint32_t>() );
            }
        }

        cNthAcq++;
    }

    t.stop();
    t.show ( "Time to take data:" );


    if (cDAQFile)
        delete cDAQFileHandler;

    cSystemController.Destroy();

}
