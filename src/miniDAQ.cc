#include <cstring>
#include "TString.h"
#include <fstream>
#include <inttypes.h>
#include <sys/stat.h>

#include "pugixml.hpp"
#include <boost/filesystem.hpp>

#include "../HWDescription/Chip.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/ChipInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/Utilities.h"
#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"

#include "../System/SystemController.h"

#include "TROOT.h"
#include "../RootUtils/publisher.h"
#include "../DQMUtils/DQMEvent.h"
#include "../DQMUtils/SLinkDQMHistogrammer.h"


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

    uint32_t pEventsperVcth;

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

    cmd.defineOption ( "dqm", "Create DQM histograms");
    cmd.defineOptionAlternative ( "dqm", "q" );

    cmd.defineOption ( "postscale", "Print only every i-th event (only send every i-th event to DQM Histogramer)", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "postscale", "p" );

    cmd.defineOption ( "daq", "Save the data into a .daq file using the phase-2 Tracker data format.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "daq", "d" );

    cmd.defineOption ( "output", "Output Directory for DQM plots & page. Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

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

    bool cDQM = cmd.foundOption ("dqm");
    std::unique_ptr<SLinkDQMHistogrammer> dqmH = nullptr;

    if (cDQM)
        dqmH = std::unique_ptr<SLinkDQMHistogrammer> (new SLinkDQMHistogrammer (0) );

    bool cPostscale = cmd.foundOption ("postscale");
    int cScaleFactor = 1;

    if (cPostscale)
        cScaleFactor = atoi ( cmd.optionValue ( "postscale" ).c_str() );

    std::stringstream outp;

    cSystemController.InitializeHw ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");
    cSystemController.ConfigureHw ();

    BeBoard* pBoard = cSystemController.fBoardVector.at ( 0 );

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
        std::vector<DQMEvent*> cDQMEvents;

        for ( auto& ev : events )
        {

            //if we write a DAQ file or want to run the DQM, get the SLink format
            if (cDAQFile || cDQM)
            {
                SLinkEvent cSLev = ev->GetSLinkEvent (pBoard);

                if (cDAQFile)
                  {
                    auto data = cSLev.getData<uint32_t>();
                    cDAQFileHandler->set (data);
                  }

                //if DQM histos are enabled and we are treating the first event, book the histograms
                if (cDQM && cN == 1)
                {
                    DQMEvent* cDQMEv = new DQMEvent (&cSLev);
                    dqmH->bookHistograms (cDQMEv->trkPayload().feReadoutMapping() );
                }

                if (cDQM)
                {
                    if (count % cScaleFactor == 0)
                        cDQMEvents.emplace_back (new DQMEvent (&cSLev) );
                }
            }

            if (cPostscale)
            {
                if ( count % cScaleFactor == 0 )
                {
                    LOG (INFO) << ">>> Event #" << count ;
                    outp.str ("");
                    outp << *ev << std::endl;
                    LOG (INFO) << outp.str();
                }
            }

            if ( count % 100  == 0 )
                LOG (INFO) << ">>> Recorded Event #" << count ;

            //increment event counter
            count++;
            cN++;
        }

        //finished  processing the events from this acquisition
        //thus now fill the histograms for the DQM
        if (cDQM)
        {
            dqmH->fillHistograms (cDQMEvents);
            cDQMEvents.clear();
        }

        cNthAcq++;
    }

    //done with the acquistion, now clean up
    if (cDAQFile)
        //this closes the DAQ file
        delete cDAQFileHandler;

    if (cDQM)
    {
        // save and publish
        // Create the DQM plots and generate the root file
        // first of all, strip the folder name
        std::vector<std::string> tokens;

        tokenize ( cOutputFile, tokens, "/" );
        std::string fname = tokens.back();

        // now form the output Root filename
        tokens.clear();
        tokenize ( fname, tokens, "." );
        std::string runLabel = tokens[0];
        std::string dqmFilename =  runLabel + "_dqm.root";
        dqmH->saveHistograms (dqmFilename);

        // find the folder (i.e DQM page) where the histograms will be published
        std::string cDirBasePath;

        if ( cmd.foundOption ( "output" ) )
        {
            cDirBasePath = cmd.optionValue ( "output" );
            cDirBasePath += "/";

        }
        else cDirBasePath = "Results/";

        // now read back the Root file and publish the histograms on the DQM page
        RootWeb::makeDQMmonitor ( dqmFilename, cDirBasePath, runLabel );
        LOG (INFO) << "Saving root file to " << dqmFilename << " and webpage to " << cDirBasePath ;
    }

    return 0;
}
