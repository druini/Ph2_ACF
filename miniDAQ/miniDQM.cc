#include <cstring>
#include <stdint.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <inttypes.h>

#include "../Utils/Utilities.h"
#include "../Utils/Data.h"
#include "../Utils/Event.h"
#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"

#include "TROOT.h"
#include "publisher.h"
#include "DQMHistogrammer.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP

void tokenize ( const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters )
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of ( delimiters, 0 );

    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of ( delimiters, lastPos );

    while ( std::string::npos != pos || std::string::npos != lastPos )
    {
        // Found a token, add it to the vector.
        tokens.push_back ( str.substr ( lastPos, pos - lastPos ) );

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of ( delimiters, pos );

        // Find next "non-delimiter"
        pos = str.find_first_of ( delimiters, lastPos );
    }
}

void dumpEvents ( const std::vector<Event*>& elist, size_t evt_limit )
{
    for ( int i = 0; i < min (elist.size(), evt_limit); i++ )
    {
        //LOG (INFO) << "Event index: " << i + 1 << std::endl;
        std::stringstream outp;
        outp << *elist[i] << std::endl;
        //LOG (INFO) << outp.str();
    }
}

int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  miniDQM application" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Binary Data File", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "output", "Output Directory for DQM plots & page. Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "dqm", "Build DQM webpage. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "dqm", "d" );

    cmd.defineOption ( "reverse", "reverse bit order for CBC data in Data::set. Default = false (needs to be used for Imperial FW);", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "reverse", "r" );

    cmd.defineOption ( "swap", "Swap endianness in Data::set. Default = true (Ph2_ACF); should be true only for legacy GlibStreamer Data", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "swap", "s" );

    cmd.defineOption ( "tree", "Create a ROOT tree also. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );

    cmd.defineOption ( "ncolumn", "Specify no. of columns.", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "ncolumn", "u" );

    cmd.defineOption ( "filter", "Select Event Filtering. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "filter", "l" );

    cmd.defineOption ( "cbcType", "Specify the CBC type(2,4,8 or 16).", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "cbcType", "c" );

    cmd.defineOption ( "nevt", "Specify number of events to be read from file at a time", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "nevt", "n" );

    cmd.defineOption ( "skipDebugHist", "Switch off debug histograms. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "skipDebugHist", "g" );

    std::map<std::string, pair<int, std::string>>  cbcTypeEvtSizeMap;
    cbcTypeEvtSizeMap["2"] = { 2, XML_DESCRIPTION_FILE_2CBC };
    cbcTypeEvtSizeMap["4"] = { 4, XML_DESCRIPTION_FILE_4CBC };
    cbcTypeEvtSizeMap["8"] = { 8, XML_DESCRIPTION_FILE_8CBC };
    cbcTypeEvtSizeMap["16"] = { 16, XML_DESCRIPTION_FILE_16CBC };
    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string rawFilename = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "";

    if ( rawFilename.empty() )
    {
        LOG (ERROR) << "Error, no binary file provided. Quitting" ;
        exit ( 2 );
    }

    // Check if the file can be found
    if ( ! boost::filesystem::exists ( rawFilename ) )
    {
        LOG (ERROR) << "Error!! binary file " << rawFilename << " not found, exiting!";
        exit ( 3 );
    }

    std::string cbcType = ( cmd.foundOption ( "cbcType" ) ) ? cmd.optionValue ( "cbcType" ) : "";

    if ( cbcTypeEvtSizeMap.find ( cbcType ) == cbcTypeEvtSizeMap.end()  )
    {
        LOG (ERROR) << "Wrong CBC type specified!!!!";
        exit ( 4 );
    }

    bool cReverse  = ( cmd.foundOption ( "reverse" ) ) ? true : false;
    bool cSwap     = ( cmd.foundOption ( "swap" ) ) ? true : false;
    bool cDQMPage  = ( cmd.foundOption ( "dqm" ) ) ? true : false;
    bool addTree   = ( cmd.foundOption ( "tree" ) ) ? true : false;
    int ncol       = ( cmd.foundOption ( "ncolumn" ) ) ? stoi (cmd.optionValue ( "ncolumn" ) ) : 2;
    bool evtFilter = ( cmd.foundOption ( "filter" ) ) ? true : false;
    int maxevt     = ( cmd.foundOption ( "nevt" ) ) ? stoi (cmd.optionValue ( "nevt" ) ) : 100000;
    bool skipHist  = ( cmd.foundOption ( "skipDebugHist" ) ) ? true : false;

    // Create the Histogrammer object
    DQMHistogrammer* dqmh = new DQMHistogrammer (addTree, ncol, evtFilter, skipHist);
    // Add File handler
    dqmh->addFileHandler ( rawFilename, 'r' );
    // Build the hardware setup
    std::string cHWFile = getenv ( "BASE_DIR" );
    cHWFile += "/";
    cHWFile += cbcTypeEvtSizeMap[cbcType].second;

    LOG (INFO) << "HWfile=" << cHWFile;
    //dqmh->parseHWxml ( cHWFile );
    std::ofstream outp;
    dqmh->InitializeHw ( cHWFile, outp );
    LOG (INFO) << outp;
    //dqmh->fParser.parseHW (cHWFile, fBeBoardFWMap, fBoardVector, os);
    const BeBoard* pBoard = dqmh->getBoard ( 0 );

    // Read the first event from the raw data file, needed to retrieve the event map
    std::vector<uint32_t> dataVec;
    int eventSize = EVENT_HEADER_TDC_SIZE_32 + CBC_EVENT_SIZE_32 * cbcTypeEvtSizeMap[cbcType].first;
    dqmh->readFile (dataVec, (cDQMPage) ? eventSize : 0);

    // Now split the data buffer in events
    int nEvents = dataVec.size() / eventSize;

    Data d;
    //call the Data::set() method - here is where i have to know the swap opitions
    d.Set ( pBoard, dataVec, nEvents, cReverse, cSwap );
    const std::vector<Event*>& elist = d.GetEvents ( pBoard );

    if ( cDQMPage )
    {
        gROOT->SetBatch ( true );
        dqmh->bookHistos (elist.at (0)->GetEventDataMap() );

        // now read the whole file in chunks of maxevt
        dqmh->getFileHandler()->rewind();
        long ntotevt = 0;

        while ( 1 )
        {
            dataVec.clear();
            dqmh->readFile (dataVec, maxevt * eventSize);
            nEvents = dataVec.size() / eventSize;

            if (!nEvents) break;

            d.Set ( pBoard, dataVec, nEvents, cReverse, cSwap );
            const std::vector<Event*>& evlist = d.GetEvents ( pBoard );
            dqmh->fillHistos (evlist, ntotevt, eventSize);
            ntotevt += nEvents;
            LOG (INFO) << "eventSize = "  << eventSize
                       << ", eventsRead = " << nEvents
                       << ", totalEventsRead = " << ntotevt;

            if ( !dqmh->getFileHandler()->file_open() ) break;
        }

        // Create the DQM plots and generate the root file
        // first of all, strip the folder name
        std::vector<std::string> tokens;
        tokenize ( rawFilename, tokens, "/" );
        std::string fname = tokens.back();

        // now form the output Root filename
        tokens.clear();
        tokenize ( fname, tokens, "." );
        std::string runLabel = tokens[0];
        std::string dqmFilename =  runLabel + "_dqm.root";

        dqmh->saveHistos (dqmFilename); // save histograms to file

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

    else dumpEvents ( elist, maxevt );

    delete dqmh;
    return 0;
}
