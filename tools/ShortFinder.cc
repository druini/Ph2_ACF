#include "ShortFinder.h"
#include "ContainerFactory.h"
#include "Occupancy.h"
#include "CBCChannelGroupHandler.h"
#include "Visitor.h"
#include "CommonVisitors.h"
#include "DataContainer.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

//initialize the static member

ShortFinder::ShortFinder() :
  Tool()
{
}

ShortFinder::~ShortFinder()
{
}

void ShortFinder::Initialise ()
{
    fChannelGroupHandler = new CBCChannelGroupHandler();
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    
    // now read the settings from the map
  
    auto cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    cSetting = fSettingsMap.find ( "TestPulseAmplitude" );
    fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;

    if ( fTestPulseAmplitude == 0 ) fTestPulse = 0;
    else fTestPulse = 1;
    
}

void ShortFinder::FindShorts(uint16_t pThreshold, uint16_t pTPamplitude)
{
    // prepare container 
    ContainerFactory::copyAndInitChannel<int>(*fDetectorContainer, fShortsContainer);
    uint8_t cFirmwareTPdelay=100;
    uint8_t cFirmwareTriggerDelay=200;
    
    // set-up for TP
    fAllChan = true;
    fMaskChannelsFromOtherGroups = !this->fAllChan;
    this->enableTestPulse(true);
    this->SetTestPulse(true);
    setSameGlobalDac("TestPulsePotNodeSel",  0xFF -  pTPamplitude);
    LOG (INFO) << BOLDBLUE << "Starting short finding loop." << RESET;
    // configure test pulse trigger 
    static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTestPulseFSM(cFirmwareTPdelay,cFirmwareTriggerDelay,1000);

    // check that the hits are there... so find test pulse
    for (auto cBoard : *fDetectorContainer)
    {
        BeBoard* theBoard = static_cast<BeBoard*>(cBoard);
        //first, set VCth to the target value for each CBC
        this->setSameDacBeBoard(theBoard , "VCth", pThreshold);
        auto& cThisShortsContainer = fShortsContainer.at(cBoard->getIndex());
        uint16_t cDelay = fBeBoardInterface->ReadBoardReg( theBoard, "fc7_daq_cnfg.fast_command_block.test_pulse.delay_after_test_pulse") ;
        this->setSameDacBeBoard(theBoard, "TriggerLatency", cDelay-1);
        uint8_t cTestGroup=0;
        for(auto cGroup : *fChannelGroupHandler)
        {
            setSameGlobalDac("TestPulseGroup",  cTestGroup);
            // bitset for this group
            std::bitset<NCHANNELS> cBitset = std::bitset<NCHANNELS>( static_cast<const ChannelGroup<NCHANNELS>*>(cGroup)->getBitset() );
            LOG (INFO) << "Injecting charge into front-end object using test capacitor " << +cTestGroup << " : L1A latency set to " << +cDelay << RESET; 
            this->ReadNEvents ( theBoard , fEventsPerPoint );
            const std::vector<Event*>& cEvents = this->GetEvents ( theBoard );
            for(auto cOpticalGroup : *cBoard)
            {
                for (auto cFe : *cOpticalGroup)
                {
                    auto& cHybridShorts = cThisShortsContainer->at(cOpticalGroup->getIndex())->at(cFe->getIndex());
                    for (auto cChip : *cFe) 
                    {
                        auto& cReadoutChipShorts = cHybridShorts->at(cChip->getIndex());
                        int cNhits=0;
                        for( auto cEvent : cEvents ) 
                        {
                            // Debug information
                            auto cEventCount = cEvent->GetEventCount(); 
                            // Hits
                            auto cHits = cEvent->GetHits( cFe->getId(), cChip->getId() ) ;
                            LOG (INFO) << BOLDBLUE << "\t\tGroup " << +cTestGroup << " OG" << +cOpticalGroup->getId() <<  " FE" << +cFe->getId() << " .. CBC" << +cChip->getId() << ".. Event " << +cEventCount << " FE" << +cFe->getId() << " - " << +cHits.size() << " hits found/" << +cBitset.count() << " channels in test group" << RESET;
                            for ( auto cHit : cHits )
                            {
                                if (cBitset[cHit] == 0) 
                                {
                                    cReadoutChipShorts->getChannelContainer<int>()->at(cHit)+=1;
                                }
                            }
                            cNhits += cHits.size();
                        }
                        // get list of channels with hits; remember - I've only added a hit if the channel is not in this test group 
                        auto cShorts = cReadoutChipShorts->getChannelContainer<int>();
                        float cNshorts = cShorts->size() - std::count (cShorts->begin(), cShorts->end(), 0) ; //
                        LOG (INFO) << BOLDBLUE << "\t\t\t FE" << +cFe->getId() << " CBC" << +cChip->getId() << " : number of shorts is  " << cNshorts << RESET;
                
                    }
                }
            }
            cTestGroup++;
        }
    }

    // disable TP 
    this->enableTestPulse(false);
    this->SetTestPulse(false);
    setSameGlobalDac( "TestPulsePotNodeSel",  0x00 );
}
