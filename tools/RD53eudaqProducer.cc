/*!OA
  \file                  RD53eudaqProducer.h
  \brief                 Implementaion of EUDAQ producer
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53eudaqProducer.h"

RD53eudaqProducer::RD53eudaqProducer(Ph2_System::SystemController& RD53SysCntr, const std::string configFile, const std::string producerName, const std::string runControl)
    : eudaq::Producer(producerName, runControl), configFile(configFile)
{
    try
    {
        doExit = false;
        RD53sysCntrPhys.Inherit(&RD53SysCntr);
        RD53sysCntrPhys.setGenericEvtConverter(RD53eudaqProducer::RD53eudaqEvtConverter(this));

        this->SetConnectionState(eudaq::ConnectionState::STATE_UNINIT, "RD53eudaqProducer::Uninitialized");
    }
    catch(...)
    {
        this->SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "RD53eudaqProducer::Constructor Error");
    }
}

void RD53eudaqProducer::OnReset()
{
    try
    {
        RD53sysCntrPhys.Stop();

        this->SetConnectionState(eudaq::ConnectionState::STATE_UNINIT, "RD53eudaqProducer::Uninitialized");
    }
    catch(...)
    {
        this->SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "RD53eudaqProducer::Resetting Error");
    }
}

void RD53eudaqProducer::OnInitialise(const eudaq::Configuration& param)
{
    try
    {
        std::stringstream outp;
        RD53sysCntrPhys.InitializeHw(configFile, outp, true, false);
        RD53sysCntrPhys.InitializeSettings(configFile, outp);

        this->SetConnectionState(eudaq::ConnectionState::STATE_UNCONF, "RD53eudaqProducer::Unconfigured");
    }
    catch(...)
    {
        this->SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "RD53eudaqProducer::Initialisation Error");
    }
}

void RD53eudaqProducer::OnConfigure(const eudaq::Configuration& param)
{
    try
    {
        RD53sysCntrPhys.localConfigure();

        this->SetConnectionState(eudaq::ConnectionState::STATE_CONF, "RD53eudaqProducer::Configured");
    }
    catch(...)
    {
        this->SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "RD53eudaqProducer::Configuration Error");
    }
}

void RD53eudaqProducer::OnStartRun(unsigned runNumber)
{
    try
    {
        theRunNumber = runNumber;
        evCounter    = 0;

        // #####################
        // # Send a BORE event #
        // #####################
        eudaq::RawDataEvent evBORE(eudaq::RawDataEvent::BORE(EUDAQ::EVENT, theRunNumber));
        RD53eudaqProducer::MySendEvent(evBORE);

        // ###################################################
        // # Get configuration directly from EUDAQ framework #
        // ###################################################
        // auto eudaqConf = this->GetConfiguration();
        // std::string fileName(eudaqConf->Get("Results", "Run" + RD53Shared::fromInt2Str(runNumber) + "_Physics"));
        std::string fileName("Run" + RD53Shared::fromInt2Str(theRunNumber) + "_Physics");
        RD53sysCntrPhys.initializeFiles(fileName);
        RD53sysCntrPhys.Start(theRunNumber);

        this->SetConnectionState(eudaq::ConnectionState::STATE_RUNNING, "RD53eudaqProducer::Running");
    }
    catch(...)
    {
        this->SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "RD53eudaqProducer::Running Error");
    }
}

void RD53eudaqProducer::OnStopRun()
{
    try
    {
        RD53sysCntrPhys.Stop();
        RD53sysCntrPhys.draw();

        // #####################
        // # Send a EORE event #
        // #####################
        eudaq::RawDataEvent evEORE(eudaq::RawDataEvent::EORE(EUDAQ::EVENT, theRunNumber, evCounter));
        RD53eudaqProducer::MySendEvent(evEORE);

        // ###########################
        // # Copy configuration file #
        // ###########################
        const auto configFileBasename = configFile.substr(configFile.find_last_of("/\\") + 1);
        const auto outputConfigFile   = std::string(RD53Shared::RESULTDIR) + "/Run" + RD53Shared::fromInt2Str(theRunNumber) + "_" + configFileBasename;
        system(("cp " + configFile + " " + outputConfigFile).c_str());

        this->SetConnectionState(eudaq::ConnectionState::STATE_CONF, "RD53eudaqProducer::Configured");
    }
    catch(...)
    {
        this->SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "RD53eudaqProducer::Stopping Error");
    }
}

void RD53eudaqProducer::OnTerminate()
{
    std::unique_lock<std::mutex> theGuard(theMtx);
    doExit = true;
    theGuard.unlock();
    wakeUp.notify_one();
}

void RD53eudaqProducer::MainLoop()
{
    std::unique_lock<std::mutex> theGuard(theMtx);
    wakeUp.wait(theGuard, [this]() { return doExit; });
}

void RD53eudaqProducer::MySendEvent(eudaq::Event& theEvent)
{
    while(true)
    {
        try
        {
            this->SendEvent(theEvent);
            break;
        }
        catch(...)
        {
            std::cout << "Resource unavailable" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(EUDAQ::WAIT));
    }
}

void RD53eudaqProducer::RD53eudaqEvtConverter::operator()(const std::vector<Ph2_HwInterface::RD53Event>& RD53EvtList)
{
    if(RD53EvtList.size() != 0)
        for(const auto& evt: RD53EvtList)
        {
            eudaq::RawDataEvent eudaqEvent(EUDAQ::EVENT, eudaqProducer->theRunNumber, eudaqProducer->evCounter++);

            eudaqEvent.SetTag("TIMESTAMP", std::time(nullptr));
            eudaqEvent.SetTag("L1A_COUNTER", evt.l1a_counter);
            eudaqEvent.SetTag("TDC", evt.tdc);
            eudaqEvent.SetTag("BX_COUNTER", evt.bx_counter);
            eudaqEvent.SetTag("TLU_TRIGGER_ID", evt.tlu_trigger_id);

            for(auto i = 0u; i < evt.chip_frames_events.size(); i++)
            {
                std::vector<uint8_t> eudaq_hits;

                eudaq_hits.push_back((Ph2_HwDescription::RD53::nRows >> 0) & 0xFF);
                eudaq_hits.push_back((Ph2_HwDescription::RD53::nRows >> 8) & 0xFF);
                eudaq_hits.push_back((Ph2_HwDescription::RD53::nCols >> 0) & 0xFF);
                eudaq_hits.push_back((Ph2_HwDescription::RD53::nCols >> 8) & 0xFF);
                eudaq_hits.push_back((evt.chip_frames_events[i].second.hit_data.size() >> 0) & 0xFF);
                eudaq_hits.push_back((evt.chip_frames_events[i].second.hit_data.size() >> 8) & 0xFF);

                for(const auto& hit: evt.chip_frames_events[i].second.hit_data)
                {
                    // #######
                    // # ROW #
                    // #######
                    eudaq_hits.push_back((hit.row >> 0) & 0xFF);
                    eudaq_hits.push_back((hit.row >> 8) & 0xFF);
                    // #######
                    // # COL #
                    // #######
                    eudaq_hits.push_back((hit.col >> 0) & 0xFF);
                    eudaq_hits.push_back((hit.col >> 8) & 0xFF);
                    // #######
                    // # TOT #
                    // #######
                    eudaq_hits.push_back(hit.tot);
                    eudaq_hits.push_back(0);
                }
                // ###########
                // # Chip ID #
                // ###########
                eudaqEvent.AddBlock(evt.chip_frames_events[i].first.chip_id, eudaq_hits);
            }

            eudaqProducer->MySendEvent(eudaqEvent);
        }
}
