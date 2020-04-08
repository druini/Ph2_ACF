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
void ShortFinder::Reset()
{
    // set everything back to original values .. like I wasn't here 
    for (auto cBoard : *fDetectorContainer)
    {
        BeBoard *theBoard = static_cast<BeBoard*>(cBoard);
        LOG (INFO) << BOLDBLUE << "Resetting all registers on back-end board " << +cBoard->getId() << RESET;
        auto& cBeRegMap = fBoardRegContainer.at(cBoard->getIndex())->getSummary<BeBoardRegMap>();
        std::vector< std::pair<std::string, uint32_t> > cVecBeBoardRegs; cVecBeBoardRegs.clear();
        for(auto cReg : cBeRegMap )
            cVecBeBoardRegs.push_back(make_pair(cReg.first, cReg.second));
        fBeBoardInterface->WriteBoardMultReg ( theBoard, cVecBeBoardRegs);

        auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());

        for(auto cOpticalGroup : *cBoard)
        {
            auto& cRegMapThisModule = cRegMapThisBoard->at(cOpticalGroup->getIndex());
            for (auto cHybrid : *cOpticalGroup)
            {
                auto& cRegMapThisHybrid = cRegMapThisModule->at(cHybrid->getIndex());
                LOG (INFO) << BOLDBLUE << "Resetting all registers on readout chips connected to FEhybrid#" << (cHybrid->getId() ) << " back to their original values..." << RESET;
                for (auto cChip : *cHybrid)
                {
                    auto& cRegMapThisChip = cRegMapThisHybrid->at(cChip->getIndex())->getSummary<ChipRegMap>(); 
                    std::vector< std::pair<std::string, uint16_t> > cVecRegisters; cVecRegisters.clear();
                    for(auto cReg : cRegMapThisChip )
                        cVecRegisters.push_back(make_pair(cReg.first, cReg.second.fValue));
                    fReadoutChipInterface->WriteChipMultReg ( static_cast<ReadoutChip*>(cChip) , cVecRegisters );
                }
            }
        }
    }
    resetPointers();
}
void ShortFinder::Print()
{
    for (auto cBoard : *fDetectorContainer)
    {
        auto& cShorts = fShorts.at(cBoard->getIndex());
        for(auto cModule : *cBoard)
        {
            auto& cShortsThisModule = cShorts->at(cModule->getIndex());
            for (auto cHybrid : *cModule)
            {
                auto& cShortsHybrid = cShortsThisModule->at(cHybrid->getIndex());
                for (auto cChip : *cHybrid)
                {
                    auto& cShortsReadoutChip = cShortsHybrid->at(cChip->getIndex())->getSummary<ChannelList>();
                    if( cShortsReadoutChip.size() == 0 )
                        LOG (INFO) << BOLDGREEN << "No shorts found in readout chip" 
                            << +cChip->getId() 
                            << " on FE hybrid " 
                            << +cHybrid->getId() << RESET;
                    else
                        LOG (INFO) << BOLDRED << "Found " << +cShortsReadoutChip.size() 
                            << " shorts in readout chip"  
                            << +cChip->getId() 
                            << " on FE hybrid " 
                            << +cHybrid->getId() << RESET;


                    for( auto cShort : cShortsReadoutChip )
                        LOG (DEBUG) << BOLDRED << "Possible short in channel " << +cShort 
                                   << " in readout chip"  
                                   << +cChip->getId() 
                                   << " on FE hybrid " 
                                   << +cHybrid->getId() << RESET;

                    
                }
            }
        }
    }
}
void ShortFinder::Initialise ()
{
    fChannelGroupHandler = new CBCChannelGroupHandler();
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    cSetting = fSettingsMap.find ( "ShortsPulseAmplitude" );
    fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;

    if ( fTestPulseAmplitude == 0 ) 
        fTestPulse = 0;
    else 
        fTestPulse = 1;
    
    // prepare container 
    ContainerFactory::copyAndInitChannel<uint16_t>(*fDetectorContainer, fShortsContainer);
    ContainerFactory::copyAndInitChannel<uint16_t>(*fDetectorContainer, fHitsContainer);
    ContainerFactory::copyAndInitStructure<ChannelList>(*fDetectorContainer, fShorts);
    ContainerFactory::copyAndInitStructure<ChannelList>(*fDetectorContainer, fInjections);


    // retreive original settings for all chips and all back-end boards 
    ContainerFactory::copyAndInitStructure<ChipRegMap>(*fDetectorContainer, fRegMapContainer);
    ContainerFactory::copyAndInitStructure<BeBoardRegMap>(*fDetectorContainer, fBoardRegContainer);
    for (auto cBoard : *fDetectorContainer)
    {
        fBoardRegContainer.at(cBoard->getIndex())->getSummary<BeBoardRegMap>() = static_cast<BeBoard*>(cBoard)->getBeBoardRegMap();
        auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());
        auto& cShorts = fShorts.at(cBoard->getIndex());
        auto& cInjections = fInjections.at(cBoard->getIndex());
        for(auto cModule : *cBoard)
        {
            auto& cShortsModule = cShorts->at(cModule->getIndex());
            auto& cInjectionsModule = cInjections->at(cModule->getIndex());
            auto& cRegMapThisModule = cRegMapThisBoard->at(cModule->getIndex());
                
            for (auto cHybrid : *cModule)
            {
                auto& cShortsHybrid = cShortsModule->at(cHybrid->getIndex());
                auto& cInjectionsHybrid = cInjectionsModule->at(cHybrid->getIndex());
                auto& cRegMapThisHybrid = cRegMapThisModule->at(cHybrid->getIndex());
                for (auto cChip : *cHybrid)
                {
                    cInjectionsHybrid->at(cChip->getIndex())->getSummary<ChannelList>().clear();
                    cShortsHybrid->at(cChip->getIndex())->getSummary<ChannelList>().clear();
                    cRegMapThisHybrid->at(cChip->getIndex())->getSummary<ChipRegMap>() = static_cast<ReadoutChip*>(cChip)->getRegMap();
                }
            }
        }
    }
}
void ShortFinder::Stop()
{
    this->Reset();
}
void ShortFinder::Count(BeBoard* pBoard, const ChannelGroup<NCHANNELS>* pGroup)
{
    auto cBitset = std::bitset<NCHANNELS>( pGroup->getBitset() );
    auto& cThisShortsContainer = fShortsContainer.at(pBoard->getIndex());
    auto& cThisHitsContainer = fHitsContainer.at(pBoard->getIndex());
    auto& cShorts = fShorts.at(pBoard->getIndex());
    auto& cInjections = fInjections.at(pBoard->getIndex());

    
    for(auto cModule : *pBoard)
    {
        auto& cModuleShorts = cThisShortsContainer->at(cModule->getIndex());
        auto& cModuleHits = cThisHitsContainer->at(cModule->getIndex());
        auto& cShortsModule = cShorts->at(cModule->getIndex());
        auto& cInjectionsModule = cInjections->at(cModule->getIndex());

        for (auto cHybrid : *cModule)
        {
            auto& cHybridShorts = cModuleShorts->at(cHybrid->getIndex());
            auto& cHybridHits = cModuleHits->at(cHybrid->getIndex());
            auto& cShortsHybrid = cShortsModule->at(cHybrid->getIndex());
            auto& cInjectionsHybrid = cInjectionsModule->at(cHybrid->getIndex());
            for (auto cChip : *cHybrid)
            {
                auto& cReadoutChipShorts = cHybridShorts->at(cChip->getIndex());
                auto& cReadoutChipHits = cHybridHits->at(cChip->getIndex());
                auto& cShortsReadoutChip = cShortsHybrid->at(cChip->getIndex())->getSummary<ChannelList>();
                auto& cInjectionsReadoutChip = cInjectionsHybrid->at(cChip->getIndex())->getSummary<ChannelList>();
                for( size_t cIndex=0; cIndex < cBitset.size(); cIndex++ )
                {
                    if (cBitset[cIndex] == 0 && cReadoutChipShorts->getChannelContainer<uint16_t>()->at(cIndex) > THRESHOLD_SHORT*fEventsPerPoint )
                    {
                        cShortsReadoutChip.push_back(cIndex);
                        LOG (DEBUG) << BOLDRED << "Possible short in channel " << +cIndex << RESET;
                    }
                    if( cBitset[cIndex] == 1 && cReadoutChipHits->getChannelContainer<uint16_t>()->at(cIndex) == fEventsPerPoint )
                    {
                        cInjectionsReadoutChip.push_back(cIndex);
                    }
                }

                if( cInjectionsReadoutChip.size() == 0 )
                {
                    LOG (INFO) << BOLDRED << "Problem injecting charge in readout chip" 
                        << +cChip->getId() 
                        << " on FE hybrid " 
                        << +cHybrid->getId() 
                        << " .. STOPPING PROCEDURE!"
                        << RESET;
                    exit(FAILED_INJECTION);
                }
            }
        }
    }
    
    
}
void ShortFinder::FindShorts2S(BeBoard* pBoard)
{
    // configure test pulse on chip 
    setSameDacBeBoard(pBoard, "TestPulsePotNodeSel",  0xFF -  fTestPulseAmplitude);
    setSameDacBeBoard(pBoard, "TestPulseDelay", 0);
    uint16_t cDelay = fBeBoardInterface->ReadBoardReg( pBoard, "fc7_daq_cnfg.fast_command_block.test_pulse.delay_after_test_pulse") - 1;
    setSameDacBeBoard(pBoard, "TriggerLatency", cDelay);
    fBeBoardInterface->ChipReSync ( pBoard ); // NEED THIS! ?? 
    LOG (INFO) << BOLDBLUE << "L1A latency set to " << +cDelay << RESET; 
    

    // for (auto cBoard : this->fBoardVector)
    uint8_t cTestGroup=0;
    LOG (INFO) << BOLDBLUE << "Starting short finding loop for 2S hybrid " << RESET;
    for(auto cGroup : *fChannelGroupHandler)
    {
        setSameGlobalDac("TestPulseGroup",  cTestGroup);
        // bitset for this group
        auto cBitset = std::bitset<NCHANNELS>( static_cast<const ChannelGroup<NCHANNELS>*>(cGroup)->getBitset() );
        LOG (INFO) << BOLDBLUE << "Injecting charge into CBCs using test capacitor " << +cTestGroup << RESET; 
        LOG (DEBUG) << BOLDBLUE << "Test pulse channel mask is " << cBitset << RESET;
        
        auto& cThisShortsContainer = fShortsContainer.at(pBoard->getIndex());
        auto& cThisHitsContainer = fHitsContainer.at(pBoard->getIndex());
    
        this->ReadNEvents ( pBoard , fEventsPerPoint );
        const std::vector<Event*>& cEvents = this->GetEvents ( pBoard );
        for( auto cEvent : cEvents ) 
        {
            auto cEventCount = cEvent->GetEventCount(); 
            for(auto cModule : *pBoard)
            {
                auto& cShortsContainer = cThisShortsContainer->at(cModule->getIndex());
                auto& cHitsContainer = cThisHitsContainer->at(cModule->getIndex());
    
                for (auto cHybrid : *cModule)
                {
                    auto& cHybridShorts = cShortsContainer->at(cHybrid->getIndex());
                    auto& cHybridHits = cHitsContainer->at(cHybrid->getIndex());
                    for (auto cChip : *cHybrid)
                    {
                        auto& cReadoutChipShorts = cHybridShorts->at(cChip->getIndex());
                        auto& cReadoutChipHits = cHybridHits->at(cChip->getIndex());
        
                        auto cHits = cEvent->GetHits( cHybrid->getId(), cChip->getId() ) ;
                        LOG (INFO) << BOLDBLUE << "\t\tGroup " 
                            << +cTestGroup << " FE" << +cHybrid->getId() 
                            << " .. CBC" << +cChip->getId() 
                            << ".. Event " << +cEventCount 
                            << " - " << +cHits.size() 
                            << " hits found/"
                            << +cBitset.count() << " channels in test group" << RESET;
                        for ( auto cHit : cHits )
                        {
                            if (cBitset[cHit] == 0) 
                                cReadoutChipShorts->getChannelContainer<uint16_t>()->at(cHit)+=1;
                            else
                                cReadoutChipHits->getChannelContainer<uint16_t>()->at(cHit)+=1;
                        }
                    }
                }
            }
        }
        this->Count(pBoard, static_cast<const ChannelGroup<NCHANNELS>*>(cGroup) );
        cTestGroup++;
    }
}
void ShortFinder::FindShorts()
{
    uint8_t cFirmwareTPdelay=100;
    uint8_t cFirmwareTriggerDelay=200;
    
    // set-up for TP
    fAllChan = true;
    fMaskChannelsFromOtherGroups = !this->fAllChan;
    SetTestAllChannels(fAllChan);
    // enable TP injection 
    enableTestPulse( true ); 
    // configure test pulse trigger 
    static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTestPulseFSM(cFirmwareTPdelay,cFirmwareTriggerDelay,1000);
    
    for (auto cBoard : *fDetectorContainer)
    {
        ReadoutChip* cFirstReadoutChip = static_cast<ReadoutChip*>(cBoard->at(0)->at(0)->at(0));
        bool cWithCBC = (cFirstReadoutChip->getFrontEndType() == FrontEndType::CBC3);
        if( cWithCBC )
            this->FindShorts2S( static_cast<BeBoard*>(cBoard) );
        else
            LOG (INFO) << BOLDRED << "Short finding for this hybrid type not yet implemented." << RESET;        
    }

}
void ShortFinder::Start(int currentRun)
{
    Initialise ();
    this->FindShorts();
    this->Print();
}