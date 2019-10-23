#include <cstring>
#include <stdint.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <inttypes.h>

#include "../Utils/SLinkEvent.h"
#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/easylogging++.h"

#include "TROOT.h"
#include "../RootUtils/publisher.h"
#include <boost/filesystem.hpp>

#include "../DQMUtils/DQMEvent.h"
#include "../DQMUtils/SLinkDQMHistogrammer.h"

using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP

void dumpEvents (const std::vector<DQMEvent*>& elist, size_t evt_limit, std::ostream& os)
{
    for ( size_t i = 0; i < std::min (elist.size(), evt_limit); ++i)
    {
        os << "Event index: " << i + 1 << std::endl;
        const DQMEvent* ev = elist.at (i);
        os << *ev << std::endl;
    }
}
void readSLinkFromFile (const std::string& filename, std::vector<DQMEvent*>& evList)
{
    // open the binary file
    std::ifstream fh (filename, std::fstream::in | std::fstream::binary);

    if (!fh)
    {
        std::cerr << "Error opening file " << filename << "!" << std::endl;
        return;
    }

    evList.clear();

    // read line-by-line
    while (1)
    {
        std::vector<uint64_t> cData;
        bool trailerFound = false;

        while (!fh.eof() )
        {
            char buffer[8];
            fh.read (buffer, sizeof (uint64_t) );

            if (!fh)
            {
                std::cerr << "Error reading data from file " << filename << "!" << std::endl;
                //continue;
            }

            uint64_t word;
            //fh.read ( (char*) &word, sizeof (uint64_t) );
            std::memcpy (&word, buffer, sizeof (uint64_t) );
            uint64_t correctedWord = ((word & 0xFFFFFFFF) << 32) | ((word >> 32) & 0xFFFFFFFF);
            cData.push_back (correctedWord);

            // Now find the last word of the event
            if ( (correctedWord & 0xFF00000000000000) >> 56 == 0xA0 &&
                    (correctedWord & 0x00000000000000F0) >> 4  == 0x7)    // SLink Trailer
            {
                trailerFound = true;
                break;
            }
        }

        // 3 is the minimum size for an empty SLinkEvent (2 words header and 1 word trailer)
        if (trailerFound && cData.size() > 3)
        {
            DQMEvent* ev = new DQMEvent (new SLinkEvent (cData) );
            evList.push_back (ev);
        }

        if (fh.eof() ) break;
    }

    // time to close the file
    if (fh.is_open() ) fh.close();
}
int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF SlinkEvent based miniDQM application" );

    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );

    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Binary .daq Data File", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "output", "Output Directory for DQM plots & page. Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "dqm", "Build DQM webpage. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "dqm", "d" );

    cmd.defineOption ( "tree", "Create a ROOT tree also. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );

    cmd.defineOption ( "nevt", "Specify number of events to be read from file at a time", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "nevt", "n" );

    cmd.defineOption ( "skipDebugHist", "Switch off debug histograms. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "skipDebugHist", "g" );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        std::cerr << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string daqFilename = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "";

    if ( daqFilename.empty() )
    {
        std::cerr << "Error, no binary SLink64 .daq format file provided. Quitting" ;
        exit ( 2 );
    }

    // Check if the file can be found
    if ( ! boost::filesystem::exists ( daqFilename ) )
    {
        std::cerr << "Error!! binary .daq file " << daqFilename << " not found, exiting!";
        exit ( 3 );
    }

    bool cDQMPage  = ( cmd.foundOption ( "dqm" ) ) ? true : false;
    int maxevt     = ( cmd.foundOption ( "nevt" ) ) ? stoi (cmd.optionValue ( "nevt" ) ) : 10;

    // read .daq file and build DQMEvents
    std::vector<DQMEvent*> evList;
    readSLinkFromFile (daqFilename, evList);

    // either prepare and public DQM histograms or simply dump (a few) events in raw and human readable formats
    if ( cDQMPage )
    {
        gROOT->SetBatch ( true );
        std::unique_ptr<SLinkDQMHistogrammer> dqmH = std::unique_ptr<SLinkDQMHistogrammer> (new SLinkDQMHistogrammer (0) );

        // book histograms
        dqmH->bookHistograms (evList[0]->trkPayload().feReadoutMapping() );

        // fill
        dqmH->fillHistograms (evList);

        // save and publish
        // Create the DQM plots and generate the root file
        // first of all, strip the folder name
        std::vector<std::string> tokens;
        tokenize ( daqFilename, tokens, "/" );
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
    else
    {
        std::stringstream outp;
        dumpEvents ( evList, maxevt, outp );
        LOG (INFO) << outp.str();
    }

    return 0;
}
