#include <cstring>
#include "../Utils/Utilities.h"
#include "pugixml/pugixml.hpp"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/Timer.h"
#include <fstream>
#include <inttypes.h>
#include <boost/filesystem.hpp>
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"
#include "TString.h"
#include <sys/stat.h>


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

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/HWDescription_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "events", "Number of Events . Default value: 10", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "events", "e" );

    cmd.defineOption ( "dqm", "Print every i-th event.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "dqm", "d" );

    cmd.defineOption ( "daq", "Save the data into a .daq file using the phase-2 Tracker data format.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "daq", "d" );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    //bool cSaveToFile = false;
    std::string cOutputFile;
    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/HWDescription_2CBC.xml";

    const char* cDirectory = "Data";
    mkdir ( cDirectory,  777 );
    int cRunNumber = 0;
    getRunNumber ("${BASE_DIR}", cRunNumber) ;
    cOutputFile = "Data/" + string_format ("run_%04d.raw", cRunNumber);
    pEventsperVcth = ( cmd.foundOption ( "events" ) ) ? convertAnyInt ( cmd.optionValue ( "events" ).c_str() ) : 10;

    cSystemController.addFileHandler ( cOutputFile, 'w' );

    std::string cDAQFileName;
    FileHandler* cDAQFileHandler = nullptr;
    bool cDAQFile = cmd.foundOption ("daq");

    if (cDAQFile)
    {
        cDAQFileName = cmd.optionValue ("daq");
        cDAQFileHandler = new FileHandler (cDAQFileName, 'w');
        LOG (INFO) << "Writing DAQ File to:   " << cDAQFileName << " - ConditionData, if present, parsed from " << cHWFile ;
    }

    std::stringstream outp;
    cSystemController.InitializeHw ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");
    cSystemController.ConfigureHw ();

    BeBoard* pBoard = cSystemController.fBoardVector.at ( 0 );

    //if ( cmd.foundOption ( "parallel" ) )
    //{
    //uint32_t nbPacket = pBoard->getReg ( "pc_commands.CBC_DATA_PACKET_NUMBER" ), nbAcq = pEventsperVcth / ( nbPacket + 1 ) + ( pEventsperVcth % ( nbPacket + 1 ) != 0 ? 1 : 0 );
    //std::cout << "Packet number=" << nbPacket << ", Nb events=" << pEventsperVcth << " -> Nb acquisition iterations=" << nbAcq << std::endl;

    //AcqVisitor visitor;
    //std::cout << "Press Enter to start the acquisition, press Enter again to stop i" << std::endl;
    //std::cin.ignore();
    //cSystemController.fBeBoardInterface->StartThread ( pBoard, nbAcq, &visitor );
    //std::cin.ignore();
    //cSystemController.fBeBoardInterface->StopThread ( pBoard );
    //}
    //else
    //{
    // make event counter start at 1 as does the L1A counter
    uint32_t cN = 1;
    uint32_t cNthAcq = 0;
    uint32_t count = 0;

    cSystemController.fBeBoardInterface->Start ( pBoard );

    while ( cN <= pEventsperVcth )
    {
        uint32_t cPacketSize = cSystemController.ReadData ( pBoard );

        if ( cN + cPacketSize >= pEventsperVcth )
            cSystemController.fBeBoardInterface->Stop ( pBoard );

        const std::vector<Event*>& events = cSystemController.GetEvents ( pBoard );

        for ( auto& ev : events )
        {
            count++;
            cN++;

            if (cDAQFile)
            {
                SLinkEvent cSLev = ev->GetSLinkEvent (pBoard);
                cDAQFileHandler->set (cSLev.getData<uint32_t>() );
            }

            if ( cmd.foundOption ( "dqm" ) )
            {
                if ( count % atoi ( cmd.optionValue ( "dqm" ).c_str() ) == 0 )
                {
                    LOG (INFO) << ">>> Event #" << count ;
                    outp.str ("");
                    outp << *ev << std::endl;
                    LOG (INFO) << outp.str();
                }
            }

            if ( count % 100  == 0 )
                LOG (INFO) << ">>> Recorded Event #" << count ;
        }

        cNthAcq++;
    }

    if (cDAQFile)
        delete cDAQFileHandler;

    //}
    cSystemController.Destroy();
}
