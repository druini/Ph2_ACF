#include <fstream>
#include <ios>
#include <cstring>

#include "../Utils/Utilities.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/argvparser.h"
#include "../Utils/Timer.h"
#include "../tools/Tool.h"
#include "CtaFpgaConfig.h"
#include "TROOT.h"
#include "TApplication.h"

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
    cmd.defineOption ( "testpulse", "Check test pulse for different groups", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "testpulse", "t" );
    cmd.defineOption ( "rate", "Measure maximal readout rate", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "rate", "r" );
    cmd.defineOption ( "ipb_rate", "Measure maximal IPBus readout rate", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "ipb_rate", "i" );
    cmd.defineOption ( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );
    cmd.defineOption ( "configure", "Configure HW", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "configure", "c" );
    cmd.defineOption ( "save", "Save the data to a raw file.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "save", "s" );
    cmd.defineOption ( "events", "Number of Events . Default value: 10000", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "events", "e" );
    cmd.defineOption ( "pkgsize", "Avg package size (for IPBus readout speed test)", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "pkgsize", "p" );
    cmd.defineOption ( "evtsize", "Avg event size (for IPBus readout speed test)", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "evtsize", "w" );
    cmd.defineOption ( "dqm", "Print every i-th event.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "dqm", "d" );
    cmd.defineOption ( "hard_reset", "Hard Reset the board", ArgvParser::NoOptionAttribute );
    cmd.defineOption ( "ddr3test", "Test the on-board ddr3 chip", ArgvParser::NoOptionAttribute );


    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    bool cHardReset = ( cmd.foundOption ( "hard_reset" ) ) ? true : false;
    bool cDDR3SelfTest = ( cmd.foundOption ( "ddr3test" ) ) ? true : false;

    bool cSaveToFile = false;
    std::string cOutputFile;

    if ( cmd.foundOption ( "save" ) )
        cSaveToFile = true ;

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/D19CHWDescription.xml";

    std::stringstream outp;
    Tool cTool;
    cTool.InitializeHw ( cHWFile, outp);
    cTool.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    if ( cSaveToFile )
    {
        cOutputFile =  cmd.optionValue ( "save" );
        cTool.InitResultFile ( cOutputFile );
    }
    if (!cHardReset) cTool.ConfigureHw ();

    BeBoard* pBoard = cTool.fBoardVector.at(0);
    cTool.fBeBoardInterface->getBoardInfo(pBoard);

    bool cTestPulse = ( cmd.foundOption ( "testpulse" ) ) ? true : false;
    bool cRate = ( cmd.foundOption ( "rate" ) ) ? true : false;
    bool cIPB_Rate = ( cmd.foundOption ( "ipb_rate" ) ) ? true : false;

    if ( cHardReset ) {
        cTool.fBeBoardInterface->RebootBoard(pBoard);
    } else if ( cDDR3SelfTest ) {
        cTool.fBeBoardInterface->setBoard ( pBoard->getBeBoardIdentifier() );
        dynamic_cast<D19cFWInterface*>(cTool.fBeBoardInterface->getFirmwareInterface())->DDR3SelfTest();
        //(D19cFWInterface*)(cTool.fBeBoardInterface->fBoardFW)->DDR3SelfTest();
    } else {
        if ( cTestPulse )
        {
            auto cSetting = cTool.fSettingsMap.find ( "TestPulsePotentiometer" );
            uint8_t cTestPulseAmplitude = ( cSetting != std::end ( cTool.fSettingsMap ) ) ? cSetting->second : 0x7F;

            uint32_t cNGroups = 8;
            uint32_t cN = 0;
            cTool.setFWTestPulse();

            for (int i = 0; i < cNGroups; i++)
            {
                cTool.setSystemTestPulse(cTestPulseAmplitude,i,true,false);
                cTool.ReadNEvents( pBoard, 1 );

                const std::vector<Event*>& events = cTool.GetEvents ( pBoard );
                for ( auto& ev : events )
                {
                    LOG (INFO) << ">>> Event #" << cN++ ;
                    outp.str ("");
                    outp << *ev;
                    LOG (INFO) << outp.str();
                }
            }

        }

        if ( cRate )
        {
            uint32_t cN = 0;
            uint32_t count = 0;
            double cAvgOccupancy = 0;

            uint32_t cNEventsToCollect = ( cmd.foundOption ( "events" ) ) ? convertAnyInt ( cmd.optionValue ( "events" ).c_str() ) : 10000;

            // be careful works only for one hybrid
            std::vector < Cbc* > cCbcVector = pBoard->getModule(0)->fCbcVector;
            uint32_t cNCbc = cCbcVector.size();

            Timer t;
            t.start();

            cTool.Start ( pBoard );
            while ( cN < cNEventsToCollect )
            {
                cTool.ReadData ( pBoard );

                const std::vector<Event*>& events = cTool.GetEvents ( pBoard );

                for ( auto& ev : events )
                {
                    count++;
                    cN++;

                    double cAvgOccupancyHyb0 = 0;
                    for(auto cCbc: cCbcVector) cAvgOccupancyHyb0 += ev->GetNHits(0,cCbc->getCbcId());
                    cAvgOccupancy += (cAvgOccupancyHyb0/cNCbc);

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

                    if ( count % 10000  == 0 )
                        LOG (INFO) << ">>> Recorded Event #" << count ;
                }
            }
            cTool.Stop ( pBoard );

            t.stop();
            LOG (INFO) << "Average Occupancy for Hybrid#0: " << (double)cAvgOccupancy/cN << " hits/(event*CBC)";
            LOG (INFO) << "Measured maximal readout rate: " << (double)(cN/t.getElapsedTime())/1000 << "kHz (based on " << +cN << " events)";
        }

        if ( cIPB_Rate )
        {
            uint32_t cN = 0;

            uint32_t cNEventsToCollect = ( cmd.foundOption ( "events" ) ) ? convertAnyInt ( cmd.optionValue ( "events" ).c_str() ) : 10000;
            uint32_t cPackageSize = ( cmd.foundOption ( "pkgsize" ) ) ? convertAnyInt ( cmd.optionValue ( "pkgsize" ).c_str() ) : 100;
            uint32_t cEvtSize = ( cmd.foundOption ( "evtsize" ) ) ? convertAnyInt ( cmd.optionValue ( "evtsize" ).c_str() ) : 94;


            Timer t;
            t.start();

            while ( cN < cNEventsToCollect )
            {
                cTool.fBeBoardInterface->ReadBlockBoardReg(pBoard, "fc7_daq_ctrl.readout_block.readout_fifo", cPackageSize*cEvtSize);
                cN += cPackageSize;
            }
            cTool.Stop ( pBoard );

            t.stop();
            LOG (INFO) << "Measured maximal IPBus readout rate: " << (double)(cN/t.getElapsedTime())/1000 << "kHz (based on " << +cN << " events, avg package size: " << +cPackageSize << " events, avg event size: " << +cEvtSize << " words)";
        }
    }

    LOG (INFO) << "*** End of the DAQ test ***" ;
    cTool.SaveResults();
    cTool.CloseResultFile();
    cTool.Destroy();

    return 0;
}
