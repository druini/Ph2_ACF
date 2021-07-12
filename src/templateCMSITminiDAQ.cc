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

#include <chrono>
#include <thread>

#ifdef __USE_ROOT__
#include "TApplication.h"
#endif

#ifdef __EUDAQ__
#include "../tools/RD53eudaqProducer.h"
#endif

// ##################
// # Default values #
// ##################
#define ARBITRARYDELAY 2 // [seconds]

INITIALIZE_EASYLOGGINGPP

using namespace Ph2_System;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void makeNtuple(const std::string& fileName, const std::vector<RD53Event>& events)
{
    TFile theFile(fileName.c_str(), "RECREATE");
    TTree theTree("theTree", "Ntuple with event data");

    uint32_t event, FW_block_size, FW_tlu_trigger_id, FW_data_format_ver, FW_tdc, FW_l1a_counter, FW_bx_counter;
    theTree.Branch("event", &event, "event/I");
    theTree.Branch("FW_block_size", &FW_block_size, "FW_block_size/I");
    theTree.Branch("FW_tlu_trigger_id", &FW_tlu_trigger_id, "FW_tlu_trigger_id/I");
    theTree.Branch("FW_data_format_ver", &FW_data_format_ver, "FW_data_format_ver/I");
    theTree.Branch("FW_tdc", &FW_tdc, "FW_tdc/I");
    theTree.Branch("FW_l1a_counter", &FW_l1a_counter, "FW_l1a_counter/I");
    theTree.Branch("FW_bx_counter", &FW_bx_counter, "FW_bx_counter/I");

    std::vector<uint64_t> FW_frame_event_error_code;
    std::vector<uint32_t> FW_frame_event_hybrid_id;
    std::vector<uint32_t> FW_frame_event_chip_lane;
    std::vector<uint32_t> FW_frame_event_l1a_data_size;
    std::vector<uint32_t> FW_frame_event_chip_type;
    std::vector<uint32_t> FW_frame_event_frame_delay;

    std::vector<uint32_t> RD53_frame_event_trigger_id;
    std::vector<uint32_t> RD53_frame_event_trigger_tag;
    std::vector<uint32_t> RD53_frame_event_bc_id;
    std::vector<uint32_t> RD53_frame_event_nhits;

    theTree.Branch("FW_frame_event_error_code", &FW_frame_event_error_code);
    theTree.Branch("FW_frame_event_hybrid_id", &FW_frame_event_hybrid_id);
    theTree.Branch("FW_frame_event_chip_lane", &FW_frame_event_chip_lane);
    theTree.Branch("FW_frame_event_l1a_data_size", &FW_frame_event_l1a_data_size);
    theTree.Branch("FW_frame_event_chip_type", &FW_frame_event_chip_type);
    theTree.Branch("FW_frame_event_frame_delay", &FW_frame_event_frame_delay);

    theTree.Branch("RD53_frame_event_trigger_id", &RD53_frame_event_trigger_id);
    theTree.Branch("RD53_frame_event_trigger_tag", &RD53_frame_event_trigger_tag);
    theTree.Branch("RD53_frame_event_bc_id", &RD53_frame_event_bc_id);
    theTree.Branch("RD53_frame_event_nhits", &RD53_frame_event_nhits);

    std::vector<int> RD53_hit_col;
    std::vector<int> RD53_hit_row;
    std::vector<int> RD53_hit_tot;

    theTree.Branch("RD53_hit_col", &RD53_hit_col);
    theTree.Branch("RD53_hit_row", &RD53_hit_row);
    theTree.Branch("RD53_hit_tot", &RD53_hit_tot);

    for(auto i = 0u; i < events.size(); i++)
    {
        auto& evt = events[i];

        event              = i;
        FW_block_size      = evt.block_size;
        FW_tlu_trigger_id  = evt.tlu_trigger_id;
        FW_data_format_ver = evt.data_format_ver;
        FW_tdc             = evt.tdc;
        FW_l1a_counter     = evt.l1a_counter;
        FW_bx_counter      = evt.bx_counter;

        FW_frame_event_error_code.clear();
        FW_frame_event_hybrid_id.clear();
        FW_frame_event_chip_lane.clear();
        FW_frame_event_l1a_data_size.clear();
        FW_frame_event_chip_type.clear();
        FW_frame_event_frame_delay.clear();

        RD53_frame_event_trigger_id.clear();
        RD53_frame_event_trigger_tag.clear();
        RD53_frame_event_bc_id.clear();
        RD53_frame_event_nhits.clear();

        for(auto& frame_event: evt.chip_frames_events)
        {
            FW_frame_event_error_code.push_back(frame_event.first.error_code);
            FW_frame_event_hybrid_id.push_back(frame_event.first.hybrid_id);
            FW_frame_event_chip_lane.push_back(frame_event.first.chip_lane);
            FW_frame_event_l1a_data_size.push_back(frame_event.first.l1a_data_size);
            FW_frame_event_chip_type.push_back(frame_event.first.chip_type);
            FW_frame_event_frame_delay.push_back(frame_event.first.frame_delay);

            RD53_frame_event_trigger_id.push_back(frame_event.second.trigger_id);
            RD53_frame_event_trigger_tag.push_back(frame_event.second.trigger_tag);
            RD53_frame_event_bc_id.push_back(frame_event.second.bc_id);
            RD53_frame_event_nhits.push_back(frame_event.second.hit_data.size());

            RD53_hit_row.clear();
            RD53_hit_col.clear();
            RD53_hit_tot.clear();

            for(const auto& hit: frame_event.second.hit_data)
            {
                RD53_hit_row.push_back(hit.row);
                RD53_hit_col.push_back(hit.col);
                RD53_hit_tot.push_back(hit.tot);
            }
        }

        theTree.Fill();
    }

    theTree.Write();
    theFile.Close();
}

void readBinaryData(const std::string& binaryFile, SystemController& mySysCntr, std::vector<RD53Event>& decodedEvents)
{
    const unsigned int    wordDataSize = 32; // @CONST@
    unsigned int          errors       = 0;
    std::vector<uint32_t> data;

    RD53Event::ForkDecodingThreads();

    LOG(INFO) << BOLDMAGENTA << "@@@ Decoding binary data file @@@" << RESET;
    mySysCntr.addFileHandler(binaryFile, 'r');
    LOG(INFO) << BOLDBLUE << "\t--> Data are being readout from binary file" << RESET;
    mySysCntr.readFile(data, 0);

    uint16_t status;
    RD53Event::DecodeEventsMultiThreads(data, decodedEvents, status);
    LOG(INFO) << GREEN << "Total number of events in binary file: " << BOLDYELLOW << decodedEvents.size() << RESET;

    for(auto i = 0u; i < decodedEvents.size(); i++)
        if(RD53Event::EvtErrorHandler(decodedEvents[i].eventStatus) == false)
        {
            LOG(ERROR) << BOLDBLUE << "\t--> Corrupted event n. " << BOLDYELLOW << i << RESET;
            errors++;
            RD53Event::PrintEvents({decodedEvents[i]});
        }

    if(decodedEvents.size() != 0)
    {
        LOG(INFO) << GREEN << "Corrupted events: " << BOLDYELLOW << std::setprecision(3) << errors << " (" << 1. * errors / decodedEvents.size() * 100. << "%)" << std::setprecision(-1) << RESET;
        int avgEventSize = data.size() / decodedEvents.size();
        LOG(INFO) << GREEN << "Average event size is " << BOLDYELLOW << avgEventSize * wordDataSize << RESET << GREEN << " bits over " << BOLDYELLOW << decodedEvents.size() << RESET << GREEN
                  << " events" << RESET;
    }

    std::string fileName(binaryFile);
    makeNtuple(fileName.replace(fileName.find(".raw"), 4, ".root"), decodedEvents);
    LOG(INFO) << GREEN << "Saving raw data into ROOT ntuple: " << BOLDYELLOW << fileName << RESET;

    mySysCntr.closeFileHandler();
}

int main(int argc, char** argv)
{
    const std::string configFile   = "CMSIT.xml";
    const std::string binaryFile   = "binaryFile.raw";
    const std::string fileName     = "OutputRootFileName";
    const int         runNumber    = 0;
    const bool        doReset      = false;
    const bool        doReadBinary = false;

    SystemController  mySysCntr;
    std::stringstream outp;
    mySysCntr.InitializeHw(configFile, outp, true, false);
    mySysCntr.InitializeSettings(configFile, outp);

    // ##################
    // # Reset hardware #
    // ##################
    if(doReset == true)
    {
        if(mySysCntr.fDetectorContainer->at(0)->at(0)->flpGBT == nullptr)
            static_cast<RD53FWInterface*>(mySysCntr.fBeBoardFWMap[mySysCntr.fDetectorContainer->at(0)->getId()])->ResetSequence("160");
        else
            static_cast<RD53FWInterface*>(mySysCntr.fBeBoardFWMap[mySysCntr.fDetectorContainer->at(0)->getId()])->ResetSequence("320");
        return EXIT_SUCCESS;
    }

    if(doReadBinary == true)
    {
        // ######################################
        // # Read binary file and decode events #
        // ######################################
        readBinaryData(binaryFile, mySysCntr, RD53Event::decodedEvents);
        // RD53Event::PrintEvents(RD53Event::decodedEvents);
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
    if(doReadBinary == false)
    {
        ph.localConfigure(fileName, -1);
        ph.Start(runNumber);
        std::this_thread::sleep_for(std::chrono::seconds(ARBITRARYDELAY));
        ph.Stop();
    }
    else
    {
        ph.localConfigure(fileName, runNumber);
        ph.analyze(true);
        ph.draw();
    }

    // #############################
    // # Destroy System Controller #
    // #############################
    mySysCntr.Destroy();

    return EXIT_SUCCESS;
}
