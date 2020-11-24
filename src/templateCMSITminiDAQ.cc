/*!
  \file                  templateCMSITminiDAQ.cc
  \brief                 Template file to be used as example for a Mini DAQ to test RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "../DQMUtils/DQMInterface.h"
#include "../System/SystemController.h"
#include "../Utils/MiddlewareInterface.h"
#include "../Utils/RD53Shared.h"
#include "../Utils/argvparser.h"

#include "../tools/RD53ClockDelay.h"
#include "../tools/RD53Gain.h"
#include "../tools/RD53GainOptimization.h"
#include "../tools/RD53InjectionDelay.h"
#include "../tools/RD53Latency.h"
#include "../tools/RD53Physics.h"
#include "../tools/RD53PixelAlive.h"
#include "../tools/RD53SCurve.h"
#include "../tools/RD53ThrAdjustment.h"
#include "../tools/RD53ThrEqualization.h"
#include "../tools/RD53ThrMinimization.h"

#ifdef __USE_ROOT__
#include "TApplication.h"
#endif

#ifdef __EUDAQ__
#include "../tools/RD53eudaqProducer.h"
#endif

INITIALIZE_EASYLOGGINGPP

using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void readBinaryData(const std::string& binaryFile, SystemController& mySysCntr, std::vector<RD53FWInterface::Event>& decodedEvents)
{
    unsigned int          errors = 0;
    std::vector<uint32_t> data;

    mySysCntr.addFileHandler(binaryFile, 'r');
    mySysCntr.readFile(data, 0);

    RD53FWInterface::DecodeEventsMultiThreads(data, decodedEvents);
    LOG(INFO) << GREEN << "Total number of events in binary file: " << BOLDYELLOW << decodedEvents.size() << RESET;

    for(auto i = 0u; i < decodedEvents.size(); i++)
        if(RD53FWInterface::EvtErrorHandler(decodedEvents[i].evtStatus) == false)
        {
            LOG(ERROR) << BOLDBLUE << "\t--> Corrupted event n. " << BOLDYELLOW << i << RESET;
            errors++;
        }

    LOG(INFO) << GREEN << "Percentage of corrupted events: " << BOLDYELLOW << std::setprecision(3) << BOLDYELLOW << 1. * errors / decodedEvents.size() * 100. << "%" << std::setprecision(-1) << RESET;
    mySysCntr.closeFileHandler();
}

void decodeEvents(SystemController& mySysCntr, const std::vector<RD53FWInterface::Event>& decodedEvents)
{
    const int nTRIGxEvent = mySysCntr.findValueInSettings("nTRIGxEvent");

    for(auto i = 0u; i < decodedEvents.size(); i++)
    {
        auto&    evt           = decodedEvents[i];
        uint64_t triggerNumber = evt.l1a_counter / nTRIGxEvent;

        LOG(INFO) << BOLDGREEN << "===========================" << RESET;
        LOG(INFO) << BOLDGREEN << "Trigger number  = " << triggerNumber << RESET;
        LOG(INFO) << BOLDGREEN << "EVENT           = " << i << RESET;
        LOG(INFO) << BOLDGREEN << "block_size      = " << evt.block_size << RESET;
        LOG(INFO) << BOLDGREEN << "tlu_trigger_id  = " << evt.tlu_trigger_id << RESET;
        LOG(INFO) << BOLDGREEN << "data_format_ver = " << evt.data_format_ver << RESET;
        LOG(INFO) << BOLDGREEN << "tdc             = " << evt.tdc << RESET;
        LOG(INFO) << BOLDGREEN << "l1a_counter     = " << evt.l1a_counter << RESET;
        LOG(INFO) << BOLDGREEN << "bx_counter      = " << evt.bx_counter << RESET;

        for(auto j = 0u; j < evt.chip_events.size(); j++)
        {
            LOG(INFO) << CYAN << "------- Chip Header -------" << RESET;
            LOG(INFO) << CYAN << "error_code      = " << evt.chip_frames[j].error_code << RESET;
            LOG(INFO) << CYAN << "hybrid_id       = " << evt.chip_frames[j].hybrid_id << RESET;
            LOG(INFO) << CYAN << "chip_lane       = " << evt.chip_frames[j].chip_lane << RESET;
            LOG(INFO) << CYAN << "l1a_data_size   = " << evt.chip_frames[j].l1a_data_size << RESET;
            LOG(INFO) << CYAN << "chip_type       = " << evt.chip_frames[j].chip_type << RESET;
            LOG(INFO) << CYAN << "frame_delay     = " << evt.chip_frames[j].frame_delay << RESET;

            LOG(INFO) << CYAN << "trigger_id      = " << evt.chip_events[j].trigger_id << RESET;
            LOG(INFO) << CYAN << "trigger_tag     = " << evt.chip_events[j].trigger_tag << RESET;
            LOG(INFO) << CYAN << "bc_id           = " << evt.chip_events[j].bc_id << RESET;

            LOG(INFO) << BOLDYELLOW << "--- Hit Data (" << evt.chip_events[j].hit_data.size() << " hits) ---" << RESET;

            for(const auto& hit: evt.chip_events[j].hit_data)
            {
                uint16_t row = hit.row;
                uint16_t col = hit.col;
                uint8_t  tot = hit.tot;

                LOG(INFO) << BOLDYELLOW << "Column: " << std::setw(3) << col << std::setw(-1) << ", Row: " << std::setw(3) << row << std::setw(-1) << ", ToT: " << std::setw(3) << +tot << std::setw(-1)
                          << RESET;
            }
        }
    }
}

int main(int argc, char** argv)
{
    const std::string configFile   = "CMSIT.xml";
    const std::string binaryFile   = "binaryFile.raw";
    const std::string fileName     = "OutputRootFileName";
    const int         runNumber    = 0;
    const bool        doReset      = false;
    const bool        doReadBinary = false;

    SystemController mySysCntr;
    std::stringstream outp;
    mySysCntr.InitializeHw(configFile, outp, true, false);
    mySysCntr.InitializeSettings(configFile, outp);

    // ##################
    // # Reset hardware #
    // ##################
    if(doReset == true)
    {
        static_cast<RD53FWInterface*>(mySysCntr.fBeBoardFWMap[mySysCntr.fDetectorContainer->at(0)->getId()])->ResetSequence();
        return EXIT_SUCCESS;
    }

    if(doReadBinary == true)
    {
        // ######################################
        // # Read binary file and decode events #
        // ######################################
        readBinaryData(binaryFile, mySysCntr, RD53FWInterface::decodedEvents);
        decodeEvents(mySysCntr, RD53FWInterface::decodedEvents);
    }
    else
    {
        // #######################
        // # Initialize Hardware #
        // #######################
        mySysCntr.Configure(configFile);
    }

    // ##################
    // # Run PixelAlive #
    // ##################
    // PixelAlive pa;
    // pa.Inherit(&mySysCntr);
    // pa.localConfigure(fileName, runNumber);
    // pa.run();
    // pa.analyze();
    // pa.draw();

    // ###############
    // # Run Physics #
    // ###############
    Physics ph;
    ph.Inherit(&mySysCntr);
    ph.localConfigure(fileName, -1);
    if(doReadBinary == false)
    {
        ph.Start(runNumber);
        usleep(2e6);
        ph.Stop();
    }
    else
    {
        ph.analyze(true);
        ph.draw();
    }

    return EXIT_SUCCESS;
}
