#include "DataChecker.h"
#ifdef __USE_ROOT__

#include "../Utils/CBCChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "BackEndAlignment.h"
#include "Occupancy.h"
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;



DataChecker::DataChecker() :
    Tool            ()
{
    fRegMapContainer.reset();
}

DataChecker::~DataChecker()
{

}

void DataChecker::Initialise ()
{
    // get threshold range  
    auto cSetting = fSettingsMap.find ( "PulseShapeInitialVcth" );
    uint16_t cInitialTh = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 400;
    cSetting = fSettingsMap.find ( "PulseShapeFinalVcth" );
    uint16_t cFinalTh = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 600;
    cSetting = fSettingsMap.find ( "PulseShapeVCthStep" );
    uint16_t cThStep = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 5;
    int cSteps = std::ceil((cFinalTh-cInitialTh)/(float)cThStep);
    LOG (INFO) << BOLDMAGENTA << "pulse shape will be scanned from " << +cInitialTh << " to " << +cFinalTh << " in " << +cThStep << " steps." << RESET;
    

    // this is needed if you're going to use groups anywhere
    fChannelGroupHandler = new CBCChannelGroupHandler();//This will be erased in tool.resetPointers()
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    #ifdef __USE_ROOT__
    //    fDQMHistogram.book(fResultFile,*fDetectorContainer);
    #endif
    
    // retreive original settings for all chips 
    ContainerFactory::copyAndInitChip<ChipRegMap>(*fDetectorContainer, fRegMapContainer);
    for(auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for(auto cHybrid : *cOpticalGroup)
            {
                for(auto cChip : *cHybrid)
                {
                    fRegMapContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<ChipRegMap>() = static_cast<ReadoutChip*>(cChip)->getRegMap();
                }
            }
        }
    }

    for(auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for(auto cHybrid : *cOpticalGroup)
            {
                for(auto cChip : *cHybrid)
                {
                    // matched hits 
                    TString cName = Form ( "h_Hits_Fe%dCbc%d", cHybrid->getId() , cChip->getId() );
                    TObject* cObj = gROOT->FindObject ( cName );
                    if ( cObj ) delete cObj;
                    TH2D* cHist2D = new TH2D ( cName, Form("Number of hits - CBC%d; Trigger Number; Pipeline Address",(int)cChip->getId()) , 40  , 0 -0.5 , 40 -0.5 , 520 , 0-0.5 , 520-0.5 );
                    bookHistogram ( cChip , "Hits_perFe", cHist2D );

                    cName = Form ( "h_MatchedHits_Fe%dCbc%d", cHybrid->getId() , cChip->getId() );
                    cObj = gROOT->FindObject ( cName );
                    if ( cObj ) delete cObj;
                    cHist2D = new TH2D ( cName, Form("Number of matched hits - CBC%d; Trigger Number; Pipeline Address",(int)cChip->getId()) ,40  , 0 -0.5 , 40 -0.5 , 520 , 0-0.5 , 520-0.5 );
                    bookHistogram ( cChip , "MatchedHits_perFe", cHist2D );

                    cName = Form ( "h_EyeL1_Fe%dCbc%d", cHybrid->getId() , cChip->getId() );
                    cObj = gROOT->FindObject ( cName );
                    if ( cObj ) delete cObj;
                    TProfile2D* cProfile2D = new TProfile2D ( cName, Form("Number of matched hits - CBC%d; Phase Tap; Trigger Number",(int)cChip->getId()) ,  20 , 0-0.5 , 20-0.5 , 40 , 0-0.5 , 40-0.5);
                    bookHistogram ( cChip , "MatchedHits_eye", cProfile2D );

                    cName = Form ( "h_TestPulse_Fe%dCbc%d", cHybrid->getId() , cChip->getId() );
                    cObj = gROOT->FindObject ( cName );
                    if ( cObj ) delete cObj;
                    cProfile2D = new TProfile2D ( cName, Form("Number of matched hits - CBC%d; Time [ns]; Test Pulse Amplitude [DAC units]",(int)cChip->getId()) ,  500 , -250 , 250 , cSteps , cInitialTh , cFinalTh);
                    bookHistogram ( cChip , "MatchedHits_TestPulse", cProfile2D );

                    cName = Form ( "h_StubLatency_Fe%dCbc%d", cHybrid->getId() , cChip->getId() );
                    cObj = gROOT->FindObject ( cName );
                    if ( cObj ) delete cObj;
                    cProfile2D = new TProfile2D ( cName, Form("Number of matched stubs - CBC%d; Latency [40 MHz clock cycles]; Test Pulse Amplitude [DAC units]",(int)cChip->getId()) ,  512, 0 , 512 , cSteps , cInitialTh , cFinalTh);
                    bookHistogram ( cChip , "StubLatency", cProfile2D );
                    

                    cName = Form ( "h_HitLatency_Fe%dCbc%d", cHybrid->getId() , cChip->getId() );
                    cObj = gROOT->FindObject ( cName );
                    if ( cObj ) delete cObj;
                    cProfile2D = new TProfile2D ( cName, Form("Number of matched hits - CBC%d; Latency [40 MHz clock cycles]; Test Pulse Amplitude [DAC units]",(int)cChip->getId()) ,  512 , 0 , 512 , cSteps , cInitialTh , cFinalTh);
                    bookHistogram ( cChip , "HitLatency", cProfile2D );
                    

                    cName = Form ( "h_NoiseHits_Fe%dCbc%d", cHybrid->getId() , cChip->getId() );
                    cObj = gROOT->FindObject ( cName );
                    if ( cObj ) delete cObj;
                    TProfile* cHist = new TProfile ( cName, Form("Number of noise hits - CBC%d; Channelr",(int)cChip->getId()) , NCHANNELS, 0-0.5 , NCHANNELS-0.5);
                    bookHistogram ( cChip , "NoiseHits", cHist );

                    cName = Form ( "h_MissedHits_Fe%dCbc%d", cHybrid->getId() , cChip->getId() );
                    cObj = gROOT->FindObject ( cName );
                    if ( cObj ) delete cObj;
                    TH1D* cHist1D = new TH1D ( cName, Form("Events between missed hits - CBC%d; Channelr",(int)cChip->getId()) , 1000, 0-0.5 , 1000-0.5);
                    bookHistogram ( cChip , "FlaggedEvents", cHist1D );

                    cName = Form ( "h_ptCut_Fe%dCbc%d", cHybrid->getId() , cChip->getId() );
                    cObj = gROOT->FindObject ( cName );
                    if ( cObj ) delete cObj;
                    cHist = new TProfile ( cName, Form("Fraction of stubs matched to hits - CBC%d; Window Offset [half-strips]",(int)cChip->getId()) , 14, -7-0.5 , 7-0.5);
                    bookHistogram ( cChip , "PtCut", cHist );

                    

                }

                // matched stubs 
                TString cName = Form ( "h_Stubs_Cic%d", cHybrid->getId() );
                TObject* cObj = gROOT->FindObject ( cName );
                if ( cObj ) delete cObj;
                TH2D* cHist2D = new TH2D ( cName, Form("Number of stubs - CIC%d; Trigger Number; Bunch Crossing Id",(int)cHybrid->getId()) , 40  , 0 -0.5 , 40 -0.5 , 4000 , 0-0.5 , 4000-0.5 );
                bookHistogram ( cHybrid , "Stubs", cHist2D );

                cName = Form ( "h_MatchedStubs_Cic%d", cHybrid->getId() );
                cObj = gROOT->FindObject ( cName );
                if ( cObj ) delete cObj;
                cHist2D = new TH2D ( cName, Form("Number of matched stubs - CIC%d; Trigger Number; Bunch Crossing Id",(int)cHybrid->getId()) , 40  , 0 -0.5 , 40 -0.5 , 4000 , 0-0.5 , 4000-0.5 );
                bookHistogram ( cHybrid , "MatchedStubs", cHist2D );

                cName = Form ( "h_MissedHits_Cic%d", cHybrid->getId() );
                cObj = gROOT->FindObject ( cName );
                if ( cObj ) delete cObj;
                cHist2D = new TH2D ( cName, Form("Number of missed hits - CIC%d; Iteration number; CBC Id",(int)cHybrid->getId()) , 100  , 0 -0.5 , 100 -0.5 , 8 , 0-0.5 , 8-0.5 );
                bookHistogram ( cHybrid , "MissedHits", cHist2D );

                // TProfile* cProfile = new TProfile ( cName, Form("Number of matched stubs - CIC%d; CBC; Fraction of matched stubs",(int)cHybrid->getId()) ,8  , 0 -0.5 , 8 -0.5 );
                // bookHistogram ( cHybrid , "MatchedStubs", cProfile );
                
                // matched hits 
                // cName = Form ( "h_MatchedHits");
                // cObj = gROOT->FindObject ( cName );
                // if ( cObj ) delete cObj;
                // cProfile = new TProfile ( cName, Form("Number of matched hits - CIC%d; CBC; Fraction of matched hits",(int)cHybrid->getId()) ,8  , 0 -0.5 , 8 -0.5 );
                // bookHistogram ( cHybrid , "MatchedHits", cProfile );

                // cName = Form ( "h_L1Status_Fe%d", cHybrid->getId() );
                // cObj = gROOT->FindObject ( cName );
                // if ( cObj ) delete cObj;
                // TH2D* cHist2D = new TH2D ( cName, Form("Error Flag CIC%d; Event Id; Chip Id; Error Bit",(int)cHybrid->getId()) , 1000, 0 , 1000 , 9, 0-0.5 , 9-0.5 );
                // bookHistogram ( cHybrid, "L1Status", cHist2D ); 
            }
        }
    }

    // read original thresholds from chips ... 
    fDetectorDataContainer = &fThresholds;
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, fThresholds);  
    // read original logic configuration from chips .. [Pipe&StubInpSel&Ptwidth , HIP&TestMode]
    fDetectorDataContainer = &fLogic;
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, fLogic);  
    fDetectorDataContainer = &fHIPs;
    ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, fHIPs);
    ContainerFactory::copyAndInitChip<int>(*fDetectorContainer, fHitCheckContainer);
    ContainerFactory::copyAndInitChip<int>(*fDetectorContainer, fStubCheckContainer);
    for(auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for(auto cHybrid : *cOpticalGroup)
            {
                for(auto cChip : *cHybrid)
                {
                    ReadoutChip* theChip = static_cast<ReadoutChip*>(cChip);
                    fThresholds        .at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(theChip, "VCth" );
                    fLogic             .at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(theChip, "Pipe&StubInpSel&Ptwidth" );
                    fHIPs              .at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(theChip, "HIP&TestMode" );
                }
            }
        }
    }

    zeroContainers();
}

void DataChecker::zeroContainers()
{
    for(auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for(auto cHybrid : *cOpticalGroup)
            {
                for(auto cChip : *cHybrid)
                {
                    fHitCheckContainer .at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = 0;
                    fStubCheckContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = 0;
                }
            }
        }
    }
}
void DataChecker::print(std::vector<uint8_t> pChipIds )
{
    for(auto cBoard : fHitCheckContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for(auto cHybrid : *cOpticalGroup)
            {
                for(auto cChip : *cHybrid)
                {
                    auto cChipId = cChip->getId();
                    if( std::find(pChipIds.begin(), pChipIds.end(), cChipId) == pChipIds.end() )
                    continue;
                    auto cHitCheck  = cChip->getSummary<uint16_t>();
                    auto cStubCheck = fStubCheckContainer.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>();
                    LOG (INFO) << BOLDBLUE << "\t\t...Found " << +cHitCheck << " matched hits and " << +cStubCheck << " matched stubs in readout chip" << +cChipId << RESET;
                }
            }
        }
    }
}
void DataChecker::matchEvents(BeBoard* pBoard, std::vector<uint8_t>pChipIds , std::pair<uint8_t,int> pExpectedStub) 
{
    // LOG (INFO) << BOLDMAGENTA << "Let's see what's on the stub lines" << RESET;
    // (static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface()))->StubDebug(true,5);
        
    // get trigger multiplicity from register 
    size_t cTriggerMult = fBeBoardInterface->ReadBoardReg (pBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
    
    //get number of events from xml
    auto cSetting = fSettingsMap.find ( "Nevents" );
    size_t cEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 100;
   
    uint8_t cSeed = pExpectedStub.first;
    int cBend = pExpectedStub.second; 

    auto& cThisHitCheckContainer = fHitCheckContainer.at(pBoard->getIndex());
    auto& cThisStubCheckContainer = fStubCheckContainer.at(pBoard->getIndex());
    
    const std::vector<Event*>& cEvents = this->GetEvents ( pBoard );
    LOG (DEBUG) << BOLDMAGENTA << "Read back " << +cEvents.size() << " events from board." << RESET;

    for(auto cOpticalGroup : *pBoard)
    {
        for (auto cHybrid : *cOpticalGroup)
        {
            auto& cHybridHitCheck = cThisHitCheckContainer->at(cHybrid->getIndex());
            auto& cHybridStubCheck = cThisStubCheckContainer->at(cHybrid->getIndex());  
        

            auto cHybridId = cHybrid->getId();
            TH2D* cMatchedStubs = static_cast<TH2D*> ( getHist ( cHybrid, "MatchedStubs" ) );
            TH2D* cAllStubs = static_cast<TH2D*> ( getHist ( cHybrid, "Stubs" ) );
            TH2D* cMissedHits = static_cast<TH2D*> ( getHist ( cHybrid, "MissedHits" ) );
            
            // matching 
            for (auto cChip : *cHybrid) 
            {
                ReadoutChip *theChip = static_cast<ReadoutChip*>(cChip);
                auto cChipId = cChip->getId();
                if( std::find(pChipIds.begin(), pChipIds.end(), cChipId) == pChipIds.end() )
                    continue;

                TH2D* cAllHits = static_cast<TH2D*> ( getHist ( cChip, "Hits_perFe" ) );
                TH2D* cMatchedHits = static_cast<TH2D*> ( getHist ( cChip, "MatchedHits_perFe" ) );
                TProfile2D* cMatchedHitsEye = static_cast<TProfile2D*> ( getHist ( cChip, "MatchedHits_eye" ) );
                
                TH1D* cFlaggedEvents = static_cast<TH1D*> ( getHist ( cChip, "FlaggedEvents" ) );
                
                // container for this chip 
                auto& cReadoutChipHitCheck = cHybridHitCheck->at(cChip->getIndex());
                auto& cReadoutChipStubCheck = cHybridStubCheck->at(cChip->getIndex());

                std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( theChip );
                // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
                uint8_t cBendCode = cBendLUT[ (cBend/2. - (-7.0))/0.5 ]; 
                std::vector<uint8_t> cExpectedHits = static_cast<CbcInterface*>(fReadoutChipInterface)->stubInjectionPattern( theChip, cSeed, cBend ); 
                LOG (INFO) << BOLDMAGENTA << "Injected a stub with seed " << +cSeed << " with bend " << +cBend << RESET;
                for(auto cHitExpected : cExpectedHits )
                    LOG (INFO) << BOLDMAGENTA << "\t.. expect a hit in channel " << +cHitExpected << RESET;
                auto cEventIterator = cEvents.begin();
                LOG (DEBUG) << BOLDMAGENTA << "CBC" << +cChip->getId() << RESET;
                for( size_t cEventIndex=0; cEventIndex < cEventsPerPoint ; cEventIndex++) // for each event 
                {
                    uint32_t cPipeline_first=0; 
                    uint32_t cBxId_first=0; 
                    bool cMissedEvent=false;
                    LOG (INFO) << BOLDMAGENTA << "Event " << +cEventIndex << RESET;
                    for(size_t cTriggerIndex=0; cTriggerIndex <= cTriggerMult; cTriggerIndex++) // cTriggerMult consecutive triggers were sent 
                    {
                        auto cEvent = *cEventIterator;
                        auto cBxId = cEvent->BxId(cHybrid->getId());
                        uint32_t cPipeline = cEvent->PipelineAddress( cHybridId, cChipId );
                        cBxId_first = (cTriggerIndex == 0 ) ? cBxId : cBxId_first;
                        cPipeline_first = (cTriggerIndex == 0 ) ? cPipeline : cPipeline_first;
                        LOG (DEBUG) << BOLDBLUE << "Chip" << +cChipId << " Trigger number " << +cTriggerIndex << " : Pipeline address " << +cPipeline  << " -- bx id is " << +cBxId_first << RESET;

                        //hits
                        auto cHits = cEvent->GetHits( cHybridId, cChipId ) ;
                        cAllHits->Fill(cTriggerIndex,cPipeline,cHits.size());
                        for( auto cHit : cHits )
                        {
                            LOG (INFO) << BOLDMAGENTA << "\t... hit found in channel " << +cHit << " of readout chip" << +cChipId << RESET; 
                        }
                        size_t cMatched=0;
                        for( auto cExpectedHit : cExpectedHits ) 
                        {
                            bool cMatchFound = std::find(  cHits.begin(), cHits.end(), cExpectedHit) != cHits.end();
                            cMatched += cMatchFound;
                            cMatchedHits->Fill(cTriggerIndex,cPipeline, static_cast<int>(cMatchFound) );
                            cMatchedHitsEye->Fill(fPhaseTap, cTriggerIndex, static_cast<int>(cMatchFound) );
                        }
                        cMissedHits->Fill( (int)(fAttempt) ,cChipId,cExpectedHits.size() - cMatched );
                        if( cMatched == cExpectedHits.size() )
                        {
                            auto& cOcc = cReadoutChipHitCheck->getSummary<int>();
                            cOcc += static_cast<int>(cMatched == cExpectedHits.size());
                        }
                        else
                        {
                            cMissedEvent = true;
                        }
                        
                        //stubs
                        auto cStubs = cEvent->StubVector( cHybridId, cChipId );
                        int cNmatchedStubs=0;
                        for( auto cStub : cStubs ) 
                        {
                            LOG (DEBUG) << BOLDMAGENTA << "\t... stub seed " << +cStub.getPosition() << " --- bend code of " << +cStub.getBend() << " expect seed " << +cSeed << " and bend code " << +cBendCode << RESET;
                            bool cMatchFound = (cStub.getPosition() == cSeed && cStub.getBend() == cBendCode); 
                            auto& cOcc = cReadoutChipStubCheck->getSummary<int>();
                            cOcc += static_cast<int>(cMatchFound);
                            cNmatchedStubs += static_cast<int>(cMatchFound);
                            cMatchedStubs->Fill( cTriggerIndex , cBxId ,static_cast<int>(cMatchFound) );
                        }
                        cAllStubs->Fill( cTriggerIndex , cBxId , cStubs.size() );
                        LOG (INFO) << BOLDMAGENTA "Chip" << +cChipId << "\t\t...Trigger number " << +cTriggerIndex  << " : Pipeline address " << +cPipeline  <<   " :" << +cMatched << " matched hits found [ " << +cHits.size() << " in total]. " <<  +cNmatchedStubs << " matched stubs." <<RESET; 
                        cEventIterator++;
                    }
                    if(cMissedEvent)
                    {
                        int cDistanceToLast = fEventCounter-fMissedEvent;
                        LOG (DEBUG) << BOLDMAGENTA << "Event with a missing hit " << fEventCounter << " -- distance to last missed event is " << cDistanceToLast << RESET;
                        cFlaggedEvents->Fill(cDistanceToLast);
                        fMissedEvent = fEventCounter;
                    }
                    fEventCounter++;
                }
            }
        }

        // noise hits
        for (auto cHybrid : *cOpticalGroup)
        {
            for (auto cChip : *cHybrid) 
            {
                ReadoutChip *theChip = static_cast<ReadoutChip*>(cChip);
                auto cChipId = cChip->getId();
                TProfile* cNoiseHits = static_cast<TProfile*> ( getHist ( cChip, "NoiseHits" ) );  
        
                std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( theChip );
                // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
                std::vector<uint8_t> cExpectedHits = static_cast<CbcInterface*>(fReadoutChipInterface)->stubInjectionPattern( theChip, cSeed, cBend ); 
                auto cHybridId = cHybrid->getId();
                
                auto cEventIterator = cEvents.begin();
                LOG (DEBUG) << BOLDMAGENTA << "CBC" << +cChip->getId() << RESET;
                for( size_t cEventIndex=0; cEventIndex < cEventsPerPoint ; cEventIndex++) // for each event 
                {
                    uint32_t cPipeline_first=0; 
                    uint32_t cBxId_first=0; 
                    LOG (DEBUG) << BOLDMAGENTA << "\t..Event" << +cEventIndex << RESET;
                
                    for(size_t cTriggerIndex=0; cTriggerIndex <= cTriggerMult; cTriggerIndex++) // cTriggerMult consecutive triggers were sent 
                    {
                        auto cEvent = *cEventIterator;
                        auto cBxId = cEvent->BxId(cHybrid->getId());
                        uint32_t cPipeline = cEvent->PipelineAddress( cHybridId, cChipId );
                        cBxId_first = (cTriggerIndex == 0 ) ? cBxId : cBxId_first;
                        cPipeline_first = (cTriggerIndex == 0 ) ? cPipeline : cPipeline_first;
                        
                        //hits
                        auto cHits = cEvent->GetHits( cHybridId, cChipId ) ;
                        for( int cChannel=0; cChannel < NCHANNELS; cChannel++)
                        {
                            bool cHitFound = std::find(  cHits.begin(), cHits.end(), cChannel) != cHits.end();
                            //if( cHitFound && std::find(pChipIds.begin(), pChipIds.end(), cChipId) == pChipIds.end() )
                            //    LOG (INFO) << BOLDMAGENTA << "\t... noise hit found in channel " << +cChannel << " of readout chip" << +cChipId << RESET; 
                            cNoiseHits->Fill(cChannel, cHitFound);
                        }
                        // for( auto cHit : cHits )
                        // {
                        //     bool cExpected = std::find(  cExpectedHits.begin(), cExpectedHits.end(), cHit) != cExpectedHits.end();
                        //     if( std::find(pChipIds.begin(), pChipIds.end(), cChipId) == pChipIds.end() )
                        //         cNoiseHits->Fill(cHit);
                        //     else
                        //     {
                        //         if(!cExpected)
                        //         {
                        //             // this is not going to work..I've masked out everyhing else!
                        //             cNoiseHits->Fill(cHit);
                        //         }
                        //     }
                        // }
                        cEventIterator++;
                    }
                }
            }
        }
   }
}
void DataChecker::ReadDataTest()
{
    std::stringstream outp;
    for(auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for (auto cHybrid : *cOpticalGroup)
            {
                // matching 
                uint16_t cTh1 = (cHybrid->getId()%2==0) ? 900 : 1; 
                uint16_t cTh2 = (cHybrid->getId()%2==0) ? 1 : 900; 
                for (auto cChip : *cHybrid) 
                {
                    if( cChip->getId()%2 == 0 )
                        fReadoutChipInterface->WriteChipReg( static_cast<ReadoutChip*>(cChip), "VCth" , cTh1);
                    else
                        fReadoutChipInterface->WriteChipReg( static_cast<ReadoutChip*>(cChip), "VCth" , cTh2);
                }
            }
        }
        
        LOG (INFO) << BOLDRED << "Opening shutter ... press any key to close .." << RESET;
        BeBoard *theBoard = static_cast<BeBoard*>(cBoard);
        fBeBoardInterface->Start(theBoard);
        do
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (10) );
        }while( std::cin.get()!='\n');
        //std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        fBeBoardInterface->Stop(theBoard);
        
        LOG (INFO) << BOLDBLUE << "Stopping triggers..." << RESET;
        this->ReadData( theBoard , true);
        const std::vector<Event*>& cEvents = this->GetEvents ( theBoard );
        LOG (INFO) << BOLDBLUE << +cEvents.size() << " events read back from FC7 with ReadData" << RESET;
        // LOG (INFO) << BOLDRED << "Press any key to to see the event printout .." << RESET;
        // do
        // {
        //     std::this_thread::sleep_for (std::chrono::milliseconds (10) );
        // }while( std::cin.get()!='\n');

        // uint32_t cN=0;
        // for ( auto& cEvent : cEvents )
        // {
        //     LOG (INFO) << ">>> Event #" << cN++ ;
        //     outp.str ("");
        //     outp << *cEvent;
        //     LOG (INFO) << outp.str();
        // }
    }
    LOG (INFO) << BOLDBLUE << "Done!" << RESET;

}
void DataChecker::ReadNeventsTest()
{
    auto cSetting = fSettingsMap.find ( "Nevents" );
    uint32_t cNevents = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 100;
    std::stringstream outp;
    for(auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for (auto cHybrid : *cOpticalGroup)
            {
                // matching 
                uint16_t cTh1 = (cHybrid->getId()%2==0) ? 900 : 1; 
                uint16_t cTh2 = (cHybrid->getId()%2==0) ? 1 : 900; 
                for (auto cChip : *cHybrid) 
                {
                    if( cChip->getId()%2 == 0 )
                        fReadoutChipInterface->WriteChipReg( static_cast<ReadoutChip*>(cChip), "VCth" , cTh1);
                    else
                        fReadoutChipInterface->WriteChipReg( static_cast<ReadoutChip*>(cChip), "VCth" , cTh2);
                }
            }
        }
        BeBoard *theBoard = static_cast<BeBoard*>(cBoard);
  
        this->ReadNEvents( theBoard , cNevents);
        const std::vector<Event*>& cEvents = this->GetEvents ( theBoard );
        LOG (INFO) << BOLDBLUE << +cEvents.size() << " events read back from FC7 with ReadData" << RESET;
        uint32_t cN=0;
        for ( auto& cEvent : cEvents )
        {
            LOG (INFO) << ">>> Event #" << cN++ ;
            outp.str ("");
            outp << *cEvent;
            if( cN%1000 == 0)
                LOG (INFO) << outp.str();
        }
    }
    LOG (INFO) << BOLDBLUE << "Done!" << RESET;

}
// void DataChecker::noiseCheck(BeBoard* pBoard, std::vector<uint8_t>pChipIds , std::pair<uint8_t,int> pExpectedStub) 
// {
//     // get number of events from xml
//     auto cSetting = fSettingsMap.find ( "Nevents" );
//     size_t cEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 100;
   
//     // get trigger multiplicity from register 
//     size_t cTriggerMult = fBeBoardInterface->ReadBoardReg (pBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
    
//     uint8_t cSeed = pExpectedStub.first;
//     int cBend = pExpectedStub.second; 

//     const std::vector<Event*>& cEvents = this->GetEvents ( pBoard );
//     LOG (DEBUG) << BOLDMAGENTA << "Read back " << +cEvents.size() << " events from board." << RESET;
//     for (auto& cHybrid : pBoard->fModuleVector)
//     {
//         auto& cCic = static_cast<OuterTrackerModule*>(cHybrid)->fCic;
//         auto cHybridId = cHybrid->getId();
            
//         for (auto& cChip : cHybrid->fReadoutChipVector) 
//         {
//             auto cChipId = cChip->getId();
//             TProfile* cNoiseHits = static_cast<TProfile*> ( getHist ( cChip, "NoiseHits" ) );  
    
//             std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cChip );
//             // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
//             uint8_t cBendCode = cBendLUT[ (cBend/2. - (-7.0))/0.5 ]; 
//             std::vector<uint8_t> cExpectedHits = static_cast<CbcInterface*>(fReadoutChipInterface)->stubInjectionPattern( cChip, cSeed, cBend ); 
            
//             auto cEventIterator = cEvents.begin();
//             size_t cEventCounter=0;
//             LOG (DEBUG) << BOLDMAGENTA << "CBC" << +cChip->getId() << RESET;
//             for( size_t cEventIndex=0; cEventIndex < cEventsPerPoint ; cEventIndex++) // for each event 
//             {
//                 uint32_t cPipeline_first=0; 
//                 uint32_t cBxId_first=0; 
//                 LOG (DEBUG) << BOLDMAGENTA << "\t..Event" << +cEventIndex << RESET;
            
//                 for(size_t cTriggerIndex=0; cTriggerIndex <= cTriggerMult; cTriggerIndex++) // cTriggerMult consecutive triggers were sent 
//                 {
//                     auto cEvent = *cEventIterator;
//                     auto cBxId = cEvent->BxId(cHybrid->getId());
//                     auto cErrorBit = cEvent->Error( cHybridId , cChipId );
//                     uint32_t cL1Id = cEvent->L1Id( cHybridId, cChipId );
//                     uint32_t cPipeline = cEvent->PipelineAddress( cHybridId, cChipId );
//                     cBxId_first = (cTriggerIndex == 0 ) ? cBxId : cBxId_first;
//                     cPipeline_first = (cTriggerIndex == 0 ) ? cPipeline : cPipeline_first;
                    
//                     //hits
//                     auto cHits = cEvent->GetHits( cHybridId, cChipId ) ;
//                     for( int cChannel=0; cChannel < NCHANNELS; cChannel++)
//                     {
//                         bool cHitFound = std::find(  cHits.begin(), cHits.end(), cChannel) != cHits.end();
//                         if( cHitFound )
//                             LOG (INFO) << BOLDMAGENTA << "\t... noise hit hit found in channel " << +cHit << " of readout chip" << +cChipId << RESET; 
//                         cNoiseHits->Fill(cHit);
//                     }
//                     cEventIterator++;
//                 }
//             }
//         }
//    }
// }
void DataChecker::TestPulse(std::vector<uint8_t> pChipIds)
{

    //Prepare container to hold  measured occupancy
    DetectorDataContainer     cMeasurement ;
    fDetectorDataContainer = &cMeasurement;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    
    
    //get number of events from xml
    auto cSetting = fSettingsMap.find ( "Nevents" );
    uint32_t cEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 100;
    
    // get trigger multiplicity from xml 
    cSetting = fSettingsMap.find ( "TriggerMultiplicity" );
    bool cConfigureTriggerMult = ( cSetting != std::end ( fSettingsMap ) );
    uint16_t cTriggerMult = cConfigureTriggerMult ? cSetting->second : 0;

    // get stub delay scan range from xml 
    cSetting = fSettingsMap.find ( "StubDelay" );
    bool cModifyStubScanRange = ( cSetting != std::end ( fSettingsMap ) );
    int cStubDelay = cModifyStubScanRange ? cSetting->second : 48 ;

    // get target threshold 
    cSetting = fSettingsMap.find ( "Threshold" );

    // get number of attempts 
    cSetting = fSettingsMap.find ( "Attempts" );
    // get mode
    cSetting = fSettingsMap.find ( "Mode" );
    uint8_t cMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0  ; 
    // get latency offset
    cSetting = fSettingsMap.find ( "LatencyOffset" );
    // resync between attempts 
    cSetting = fSettingsMap.find ( "ReSync" );
    
    // if TP is used - enable it 
    cSetting = fSettingsMap.find ( "PulseShapePulseAmplitude" );
    fTPconfig.tpAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 100; 

    // get TP amplitude range 
    // get threshold range  
    
    // get threshold range  
    cSetting = fSettingsMap.find ( "PulseShapeInitialVcth" );
    uint16_t cInitialTh = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 400;
    cSetting = fSettingsMap.find ( "PulseShapeFinalVcth" );
    uint16_t cFinalTh = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 600;
    cSetting = fSettingsMap.find ( "PulseShapeVCthStep" );
    uint16_t cThStep = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 5;
    


    // get TP delay range 
    cSetting = fSettingsMap.find ( "PulseShapeInitialDelay" );
    uint16_t cInitialTPdleay = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    cSetting = fSettingsMap.find ( "PulseShapeFinalDelay" );
    uint16_t cFinalTPdleay = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 25;
    cSetting = fSettingsMap.find ( "PulseShapeDelayStep" );
    uint16_t cTPdelayStep = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 3;
    

    // get injected stub from xmls 
    std::pair<uint8_t, int> cStub;
    cSetting = fSettingsMap.find ( "StubSeed" );
    cStub.first = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10; 
    cSetting = fSettingsMap.find ( "StubBend" );
    cStub.second = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0; 
    LOG (DEBUG) << BOLDBLUE << "Injecting a stub in position " << +cStub.first << " with bend " << cStub.second << " to test data integrity..." << RESET;

    // set-up for TP
    fAllChan = true;
    fMaskChannelsFromOtherGroups = !this->fAllChan;
    this->SetTestPulse(true);
    setSameGlobalDac("TestPulsePotNodeSel",  0xFF-fTPconfig.tpAmplitude );
   
    // configure FE chips so that stubs are detected [i.e. make sure HIP
    // suppression is off ] 

    for(auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for (auto cHybrid : *cOpticalGroup)
            {
                static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (static_cast<OuterTrackerModule*>(cHybrid)->getLinkId());
                //configure CBCs 
                for (auto cChip : *cHybrid)
                {
                    // switch off HitOr
                    static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "HitOr", 0);
                    //enable stub logic
                    if( cMode == 0 )
                        static_cast<CbcInterface*>(fReadoutChipInterface)->selectLogicMode( static_cast<ReadoutChip*>(cChip), "Sampled", true, true); 
                    else
                        static_cast<CbcInterface*>(fReadoutChipInterface)->selectLogicMode( static_cast<ReadoutChip*>(cChip), "Latched", true, true); 
                    static_cast<CbcInterface*>(fReadoutChipInterface)->enableHipSuppression( static_cast<ReadoutChip*>(cChip), false, false, 0);
                }
            }
        }
        fBeBoardInterface->ChipReSync ( static_cast<BeBoard*>(cBoard) );
    }

    // generate stubs in exactly chips with IDs that match pChipIds
    std::vector<int> cExpectedHits(0);
    for(auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for (auto cHybrid : *cOpticalGroup)
            {
                for (auto cChip : *cHybrid)
                {
                    ReadoutChip* theChip = static_cast<ReadoutChip*>(cChip);
                    if( std::find(pChipIds.begin(), pChipIds.end(), cChip->getId()) != pChipIds.end()  ) 
                    {
                        std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( theChip );
                        // both stub and bend are in units of half strips 
                        // if using TP then always inject a stub with bend 0 .. 
                        // later will use offset window to modify bend [ should probably put this in inject stub ]
                        uint8_t cBend_halfStrips = cStub.second ; 
                        static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( theChip , {cStub.first} , {cBend_halfStrips}, false );
                        // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
                        // set offsets
                        // needs to be fixed 
                        /*
                        uint8_t cOffsetCode = static_cast<uint8_t>(std::fabs(cBend_strips*2)) | (std::signbit(-1*cBend_strips) << 3);
                        //uint8_t cOffsetCode = static_cast<uint8_t>(std::fabs(cBend_strips*2)) | (std::signbit(-1*cBend_strips) << 3);
                        // set offsets
                        uint8_t cOffetReg = (cOffsetCode << 4) | (cOffsetCode << 0);
                        LOG (INFO) << BOLDBLUE << "\t..--- bend code is 0x" << std::hex << +cBendCode << std::dec << " ..setting offset window to  " << std::bitset<8>(cOffetReg) << RESET;
                        fReadoutChipInterface->WriteChipReg ( theChip, "CoincWind&Offset12", cOffetReg );
                        fReadoutChipInterface->WriteChipReg ( theChip, "CoincWind&Offset34", cOffetReg );
                        */
                        fReadoutChipInterface->enableInjection(theChip, true);
                        //static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theChip, "VCth" , cTargetThreshold);
                    }
                    else
                    static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( theChip, true);
                } 
            }
        }
    }

    
    
    // measure         
    for(auto cBoard : *fDetectorContainer)
    {
        BeBoard* theBoard = static_cast<BeBoard*>(cBoard);
        uint16_t cBoardTriggerMult = fBeBoardInterface->ReadBoardReg (theBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
        LOG (DEBUG) << BOLDBLUE << "Trigger multiplicity is set to " << +cBoardTriggerMult << " consecutive triggers per L1A." << RESET ; 
        if( cConfigureTriggerMult )
        {
            LOG (DEBUG) << BOLDBLUE << "Modifying trigger multiplicity to be " << +(1+cTriggerMult) << " consecutive triggers per L1A for DataTest" << RESET;
            fBeBoardInterface->WriteBoardReg (theBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", cTriggerMult);
        }
        
        // set TP amplitude 
        setSameGlobalDac("TestPulsePotNodeSel",  0xFF-fTPconfig.tpAmplitude );
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTestPulseFSM(fTPconfig.firmwareTPdelay ,fTPconfig.tpDelay ,fTPconfig.tpSequence, fTPconfig.tpFastReset );
        for(uint16_t cDelay = cInitialTPdleay; cDelay <= cFinalTPdleay; cDelay+=cTPdelayStep)
        {
            uint8_t  cDelayDAC   = 25 - cDelay%25;
            uint16_t cLatencyDAC = fTPconfig.tpDelay - cDelay/25;
            //configure TP delay on all chips
            setSameGlobalDac("TestPulseDelay", cDelayDAC);
            // configure hit latency on all chips  
            setSameDacBeBoard(theBoard, "TriggerLatency", cLatencyDAC);
            // set stub latency on back-end board 
            uint16_t cStubLatency = cLatencyDAC - 1*cStubDelay ;
            fBeBoardInterface->WriteBoardReg (theBoard, "fc7_daq_cnfg.readout_block.global.common_stubdata_delay", cStubLatency);
            fBeBoardInterface->ChipReSync ( theBoard ); // NEED THIS! ?? 
            // loop over threshold here 
            LOG (INFO) <<  BOLDMAGENTA << "Delay is " << -1*cDelay << " TP delay is " << +cDelayDAC << " latency DAC set to " << +cLatencyDAC << RESET;
            for( uint16_t cThreshold = cInitialTh ; cThreshold < cFinalTh ; cThreshold+= cThStep )
            {
                LOG (DEBUG) <<  BOLDMAGENTA << "\t\t...Threshold is " << +cThreshold << RESET;
                for(auto cOpticalGroup : *cBoard)
                {
                    for (auto cHybrid : *cOpticalGroup)
                    {
                        for (auto cChip : *cHybrid) 
                        {
                            if( std::find(pChipIds.begin(), pChipIds.end(), cChip->getId()) == pChipIds.end() )
                                continue;
                            fReadoutChipInterface->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "VCth", cThreshold );
                        }
                    }   
                }

                // start triggers 
                fBeBoardInterface->Start(theBoard);
                auto cNtriggers = fBeBoardInterface->ReadBoardReg (theBoard, "fc7_daq_stat.fast_command_block.trigger_in_counter");
                do
                {
                    std::this_thread::sleep_for (std::chrono::milliseconds (10) );
                    cNtriggers = fBeBoardInterface->ReadBoardReg (theBoard, "fc7_daq_stat.fast_command_block.trigger_in_counter");
                }while( cNtriggers < 100 );
                fBeBoardInterface->Stop(theBoard);
                //this->ReadNEvents ( theBoard , cEventsPerPoint);
                this->ReadData( theBoard , true);
                const std::vector<Event*>& cEvents = this->GetEvents ( theBoard );
                // matching 
                for(auto cOpticalGroup : *cBoard)
                {
                    for (auto cHybrid : *cOpticalGroup)
                    {
                        auto cHybridId = cHybrid->getId();
                        for (auto cChip : *cHybrid) 
                        {
                            ReadoutChip* theChip = static_cast<ReadoutChip*>(cChip);
                            auto cOffset = 0;
                            auto cThreshold = fReadoutChipInterface->ReadChipReg(theChip, "VCth");
                            auto cChipId = cChip->getId();
                            if( std::find(pChipIds.begin(), pChipIds.end(), cChipId) == pChipIds.end() )
                                continue;

                            std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( theChip );
                            // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
                            uint8_t cBendCode = cBendLUT[ ((cStub.second+cOffset)/2. - (-7.0))/0.5 ]; 
                            
                            std::vector<uint8_t> cExpectedHits = static_cast<CbcInterface*>(fReadoutChipInterface)->stubInjectionPattern( theChip, cStub.first, cStub.second  ); 
                            LOG (DEBUG) << BOLDMAGENTA << "Injected a stub with seed " << +cStub.first << " with bend " << +cStub.second << RESET;
                            for(auto cHitExpected : cExpectedHits )
                                LOG (DEBUG) << BOLDMAGENTA << "\t.. expect a hit in channel " << +cHitExpected << RESET;
                            
                            auto cEventIterator = cEvents.begin();
                            LOG (DEBUG) << BOLDMAGENTA << "CBC" << +cChip->getId() << RESET;
                            size_t cMatchedStubs=0;
                            // vector to keep track of number of matches
                            std::vector<uint32_t> cHitMatches(cExpectedHits.size(),0); 
                            for( size_t cEventIndex=0; cEventIndex < cEventsPerPoint ; cEventIndex++) // for each event 
                            {
                                uint32_t cPipeline_first=0; 
                                uint32_t cBxId_first=0; 
                                
                                if( cEventIndex == 0 )
                                    LOG (DEBUG) << BOLDMAGENTA << "'\tEvent " << +cEventIndex << RESET;
                                bool cIncorrectPipeline=false;
                                for(size_t cTriggerIndex=0; cTriggerIndex <= cTriggerMult; cTriggerIndex++) // cTriggerMult consecutive triggers were sent 
                                {
                                    auto cEvent = *cEventIterator;
                                    auto cBxId = cEvent->BxId(cHybrid->getId());
                                    uint32_t cPipeline = cEvent->PipelineAddress( cHybridId, cChipId );
                                    cBxId_first = (cTriggerIndex == 0 ) ? cBxId : cBxId_first;
                                    cPipeline_first = (cTriggerIndex == 0 ) ? cPipeline : cPipeline_first;
                                    bool cCountEvent = ( static_cast<size_t>(cPipeline - cPipeline_first) == cTriggerIndex );
                                    if(cCountEvent)
                                    {
                                        //hits
                                        auto cHits = cEvent->GetHits( cHybridId, cChipId ) ;
                                        size_t cMatched=0;
                                        int cLatency_eq = cLatencyDAC  - (cPipeline - cPipeline_first);
                                        double cTime_ns = -1*(cLatency_eq - fTPconfig.tpDelay)*25 + cDelayDAC ;
                                        auto cIterator =  cExpectedHits.begin(); 
                                        do
                                        {
                                            bool cMatchFound = std::find(  cHits.begin(), cHits.end(), *cIterator) != cHits.end();
                                            cHitMatches[ std::distance( cExpectedHits.begin() , cIterator )] += cMatchFound;
                                            cMatched += cMatchFound;
                                            cIterator++;
                                        }while( cIterator < cExpectedHits.end() );
                                        
                                        //stubs
                                        auto cStubs = cEvent->StubVector( cHybridId, cChipId );
                                        int cNmatchedStubs=0;
                                        for( auto cHybridStub : cStubs ) 
                                        {
                                            LOG (DEBUG) << BOLDMAGENTA << "\t.. expected seed is " << +cStub.first << " measured seed is " << +cHybridStub.getPosition() << RESET;
                                            LOG (DEBUG) << BOLDMAGENTA << "\t.. expected bend code is 0x" << std::hex << +cBendCode << std::dec << " measured bend code is 0x" << std::hex <<  +cHybridStub.getBend() << std::dec << RESET;
                                            bool cMatchFound = (cHybridStub.getPosition() == cStub.first && cHybridStub.getBend() == cBendCode); 
                                            cNmatchedStubs += static_cast<int>(cMatchFound);
                                        }
                                        if( cNmatchedStubs == 1 )
                                        {
                                            cMatchedStubs ++;
                                        }
                                        
                                        if( cEventIndex == 0 )
                                            LOG (DEBUG) << BOLDMAGENTA << "\t\t.. Threshold of " << +cThreshold << " [Trigger " << +cTriggerIndex << " Pipeline is " << +cPipeline << "] Delay of " << +cTime_ns << " ns [ " << +cDelay << " ] after the TP ... Found " << +cMatched << " matched hits and " <<  +cNmatchedStubs << " matched stubs." <<RESET; 
                                    }
                                    else
                                        cIncorrectPipeline = cIncorrectPipeline || true;
                                    cEventIterator++;
                                }
                                // if missed on pipeline .. count this event

                            }
                            for( auto cIterator = cHitMatches.begin(); cIterator < cHitMatches.end() ; cIterator++)
                            {
                                LOG (INFO) << BOLDMAGENTA << "\t.. " << +(*cIterator) << " events out of a possible " << +cEvents.size() << " with a hit in channel " << +cExpectedHits[ std::distance( cHitMatches.begin() , cIterator )] <<  RESET;
                            }
                        }
                    }
                }
            }
        }
        // change the chip latency 
        //int cStartLatency = (cLatencyOffsetStart > cLatencyOffsetStop) ? (fTPconfig.tpDelay-1*cLatencyOffsetStart) : (fTPconfig.tpDelay+cLatencyOffsetStart); 
        //int cStopLatency = (cLatencyOffsetStart > cLatencyOffsetStop) ? (fTPconfig.tpDelay+cLatencyOffsetStop) : (fTPconfig.tpDelay+cLatencyOffsetStop+1); 
        //LOG (INFO) << BOLDMAGENTA << "Scanning L1A latency between " << +cStartLatency << " and " << +cStopLatency << RESET;
        // for( uint16_t cLatency=cStartLatency; cLatency < cStopLatency; cLatency++)
        // {
        //     // configure TP trigger machine 
        //     double cDeltaTrigger = fTPconfig.tpDelay; // number of clock cycles after TP
        //     for(uint8_t cTPdelay=cInitialTPdleay; cTPdelay < cFinalTPdleay ; cTPdelay+=cTPdelayStep)
        //     {
        //         // configure TP 
        //         setSameGlobalDac("TestPulseDelay", cTPdelay);
        //         setSameGlobalDac("TestPulsePotNodeSel",  0xFF-fTPconfig.tpAmplitude );
        //         // configure hit latency on all chips  
        //         setSameDacBeBoard(cBoard, "TriggerLatency", cLatency);
        //         // set stub latency on back-end board 
        //         uint16_t cStubLatency = cLatency - 1*cStubDelay ;
        //         fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_cnfg.readout_block.global.common_stubdata_delay", cStubLatency);
        //         fBeBoardInterface->ChipReSync ( cBoard ); // NEED THIS!
        //         LOG (INFO) << BOLDMAGENTA << "TP amplitude set to " << +fTPconfig.tpAmplitude << " -- TP delay set to " << +cTPdelay << RESET;
        //         // set TP size  
        //         for( uint16_t cThreshold=585; cThreshold < 586; cThreshold++)
        //         {
        //             // set stub window offset
        //             for( int cOffset=cInitialWindowOffset; cOffset <= cFinalWindowOffset; cOffset++)
        //             {
        //                 LOG (INFO) << BOLDMAGENTA << "Correlation window offset set to " << +cOffset << RESET;
        //                 for (auto& cHybrid : cBoard->fModuleVector)
        //                 {
        //                     for (auto& cChip : cHybrid->fReadoutChipVector) 
        //                     {
        //                         if( std::find(pChipIds.begin(), pChipIds.end(), cChip->getId()) == pChipIds.end() )
        //                             continue;

        //                         if( cOffset < 0)
        //                         {
        //                             uint8_t cOffetReg = ((0xF-cOffset+1) << 4) | ((0xF-cOffset+1) << 0);
        //                             fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset12", cOffetReg );
        //                             fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset34", cOffetReg );
        //                         }
        //                         else
        //                         {
        //                             uint8_t cOffetReg = ((-1*cOffset) << 4) | ((-1*cOffset) << 0);
        //                             fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset12", cOffetReg );
        //                             fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset34", cOffetReg );
        //                         }
        //                         fReadoutChipInterface->WriteChipReg ( cChip, "VCth", cThreshold );
        //                     }
        //                 }   

        //                 // read N events and compare hits and stubs to injected stub 
        //                 for( size_t cAttempt=0; cAttempt < cAttempts ; cAttempt++)
        //                 {
        //                     this->zeroContainers();
                
        //                     fAttempt = cAttempt;
        //                     LOG (DEBUG) << BOLDBLUE << "Iteration# " << +fAttempt << RESET;
        //                     // send a resync
        //                     //if( cResync)
        //                     //    fBeBoardInterface->ChipReSync ( cBoard );
        //                     this->ReadNEvents ( cBoard , cEventsPerPoint);
        //                     const std::vector<Event*>& cEvents = this->GetEvents ( cBoard );
        //                     // matching 
        //                     for (auto& cHybrid : cBoard->fModuleVector)
        //                     {
        //                         auto cHybridId = cHybrid->getId();
        //                         for (auto& cChip : cHybrid->fReadoutChipVector) 
        //                         {
        //                             auto cChipId = cChip->getId();
        //                             if( std::find(pChipIds.begin(), pChipIds.end(), cChipId) == pChipIds.end() )
        //                                 continue;

        //                             TProfile2D* cMatchedHitsTP = static_cast<TProfile2D*> ( getHist ( cChip, "MatchedHits_TestPulse" ) );
        //                             TProfile2D* cMatchedHitsLatency = static_cast<TProfile2D*> ( getHist ( cChip, "HitLatency" ) );
        //                             TProfile2D* cMatchedStubsLatency = static_cast<TProfile2D*> ( getHist ( cChip, "StubLatency" ) );
        //                             TProfile* cMatchedStubsHist = static_cast<TProfile*>( getHist(cChip, "PtCut") );

        //                             std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cChip );
        //                             // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
        //                             uint8_t cBendCode = cBendLUT[ ((cStub.second+cOffset)/2. - (-7.0))/0.5 ]; 
                                    
        //                             std::vector<uint8_t> cExpectedHits = static_cast<CbcInterface*>(fReadoutChipInterface)->stubInjectionPattern( cChip, cStub.first, cStub.second  ); 
        //                             LOG (DEBUG) << BOLDMAGENTA << "Injected a stub with seed " << +cStub.first << " with bend " << +cStub.second << RESET;
        //                             for(auto cHitExpected : cExpectedHits )
        //                                 LOG (DEBUG) << BOLDMAGENTA << "\t.. expect a hit in channel " << +cHitExpected << RESET;
                                    
        //                             auto cEventIterator = cEvents.begin();
        //                             size_t cEventCounter=0;
        //                             LOG (INFO) << BOLDMAGENTA << "CBC" << +cChip->getId() << RESET;
        //                             size_t cMatchedHits=0;
        //                             size_t cMatchedStubs=0;
        //                             for( size_t cEventIndex=0; cEventIndex < cEventsPerPoint ; cEventIndex++) // for each event 
        //                             {
        //                                 uint32_t cPipeline_first=0; 
        //                                 uint32_t cBxId_first=0; 
        //                                 bool cMissedEvent=false;

        //                                 if( cEventIndex == 0 )
        //                                     LOG (INFO) << BOLDMAGENTA << "'\tEvent " << +cEventIndex << RESET;
        //                                 bool cIncorrectPipeline=false;
        //                                 for(size_t cTriggerIndex=0; cTriggerIndex <= cTriggerMult; cTriggerIndex++) // cTriggerMult consecutive triggers were sent 
        //                                 {
        //                                     auto cEvent = *cEventIterator;
        //                                     auto cBxId = cEvent->BxId(cHybrid->getId());
        //                                     auto cErrorBit = cEvent->Error( cHybridId , cChipId );
        //                                     uint32_t cL1Id = cEvent->L1Id( cHybridId, cChipId );
        //                                     uint32_t cPipeline = cEvent->PipelineAddress( cHybridId, cChipId );
        //                                     cBxId_first = (cTriggerIndex == 0 ) ? cBxId : cBxId_first;
        //                                     cPipeline_first = (cTriggerIndex == 0 ) ? cPipeline : cPipeline_first;
        //                                     bool cCountEvent = ( static_cast<size_t>(cPipeline - cPipeline_first) == cTriggerIndex );
        //                                     if(cCountEvent)
        //                                     {
        //                                         //hits
        //                                         auto cHits = cEvent->GetHits( cHybridId, cChipId ) ;
        //                                         size_t cMatched=0;
        //                                         int cLatency_eq = cLatency  - (cPipeline - cPipeline_first);
        //                                         double cTime_ns = 1*(static_cast<float>(fTPconfig.tpDelay - cLatency_eq)*25. + (25.0 - cTPdelay));
        //                                         for( auto cExpectedHit : cExpectedHits ) 
        //                                         {
        //                                             bool cMatchFound = std::find(  cHits.begin(), cHits.end(), cExpectedHit) != cHits.end();
        //                                             cMatched += cMatchFound;
        //                                             //cMatchedHits->Fill(cTriggerIndex,cPipeline, static_cast<int>(cMatchFound) );
        //                                             //cMatchedHitsEye->Fill(fPhaseTap, cTriggerIndex, static_cast<int>(cMatchFound) );
        //                                         }
        //                                         if( cMatched == cExpectedHits.size() )
        //                                         {
        //                                             cMatchedHits ++;
        //                                             //cMatchedHitsLatency->Fill( cLatency_eq , cTPamplitude , 1 );
        //                                             //cMatchedHitsTP->Fill( cTime_ns , cTPamplitude , 1);
        //                                         }
        //                                         //else
        //                                         //{
        //                                         //    cMatchedHitsLatency->Fill( cLatency_eq , cTPamplitude , 0 );
        //                                         //    cMatchedHitsTP->Fill( cTime_ns , cTPamplitude , 0);
        //                                         //}
        //                                         //stubs
        //                                         auto cStubs = cEvent->StubVector( cHybridId, cChipId );
        //                                         int cNmatchedStubs=0;
        //                                         for( auto cHybridStub : cStubs ) 
        //                                         {
        //                                             LOG (DEBUG) << BOLDMAGENTA << "\t.. expected seed is " << +cStub.first << " measured seed is " << +cHybridStub.getPosition() << RESET;
        //                                             LOG (DEBUG) << BOLDMAGENTA << "\t.. expected bend code is 0x" << std::hex << +cBendCode << std::dec << " measured bend code is 0x" << std::hex <<  +cHybridStub.getBend() << std::dec << RESET;
        //                                             bool cMatchFound = (cHybridStub.getPosition() == cStub.first && cHybridStub.getBend() == cBendCode); 
        //                                             cMatchedStubsHist->Fill( cOffset , cMatchFound);
        //                                             cNmatchedStubs += static_cast<int>(cMatchFound);
        //                                         }
        //                                         if( cNmatchedStubs == 1 )
        //                                         {
        //                                             cMatchedStubs ++;
        //                                             //cMatchedStubsLatency->Fill( cLatency_eq - cStubDelay  , cTPamplitude , 1 );
        //                                         }
        //                                         //else
        //                                         //    cMatchedStubsLatency->Fill( cLatency_eq - cStubDelay  , cTPamplitude , 0 );
                                                
        //                                         int cBin = cMatchedHitsLatency->GetXaxis()->FindBin( cLatency_eq + cTPdelay*(1.0/25) );
        //                                         if( cEventIndex == 0 )
        //                                             LOG (INFO) << BOLDMAGENTA << "\t\t.. Trigger " << +cTriggerIndex << " Pipeline is " << +cPipeline << " -Latency is  " << +cLatency << " TP fine delay is " << +cTPdelay << " -- threshold of " << +cThreshold << " Time in ns is " << +cTime_ns << " .Found " << +cMatched << " matched hits and " <<  +cNmatchedStubs << " matched stubs." <<RESET; 
        //                                             //LOG (INFO) << BOLDMAGENTA << "\t\t.. Trigger " << +cTriggerIndex << " Pipeline is " << +cPipeline << " -Latency is  " << +cLatency_eq << " , trigger sent " << fTPconfig.tpDelay+cLatencyOffset << " clocks after fast reset, TP delay is " << +cTPdelay << " L1A " << cLatency << " clocks. Time in ns is " << +cTime_ns << " .Found " << +cMatched << " matched hits and " <<  +cNmatchedStubs << " matched stubs." <<RESET; 
        //                                     }
        //                                     else
        //                                         cIncorrectPipeline = cIncorrectPipeline || true;
        //                                     cEventIterator++;
        //                                 }
        //                                 // if missed on pipeline .. count this event

        //                             }
        //                         }
        //                     }
        //                     //this->print(pChipIds);
        //                 }
        //             }
        //         }

        //         // for( uint16_t cTPamplitude=cInitialAmpl; cTPamplitude < cFinalAmpl; cTPamplitude+= cAmplStep)
        //         // {
        //         //     // set stub window offset
        //         //     for( int cOffset=cInitialWindowOffset; cOffset <= cFinalWindowOffset; cOffset++)
        //         //     {
        //         //         LOG (INFO) << BOLDMAGENTA << "Correlation window offset set to " << +cOffset << RESET;
        //         //         for (auto& cHybrid : cBoard->fModuleVector)
        //         //         {
        //         //             for (auto& cChip : cHybrid->fReadoutChipVector) 
        //         //             {
        //         //                 if( std::find(pChipIds.begin(), pChipIds.end(), cChip->getId()) == pChipIds.end() )
        //         //                     continue;

        //         //                 if( cOffset < 0)
        //         //                 {
        //         //                     uint8_t cOffetReg = ((0xF-cOffset+1) << 4) | ((0xF-cOffset+1) << 0);
        //         //                     fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset12", cOffetReg );
        //         //                     fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset34", cOffetReg );
        //         //                 }
        //         //                 else
        //         //                 {
        //         //                     uint8_t cOffetReg = ((-1*cOffset) << 4) | ((-1*cOffset) << 0);
        //         //                     fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset12", cOffetReg );
        //         //                     fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset34", cOffetReg );
        //         //                 }
        //         //             }
        //         //         }   

        //         //         // read N events and compare hits and stubs to injected stub 
        //         //         for( size_t cAttempt=0; cAttempt < cAttempts ; cAttempt++)
        //         //         {
        //         //             this->zeroContainers();
                
        //         //             fAttempt = cAttempt;
        //         //             LOG (DEBUG) << BOLDBLUE << "Iteration# " << +fAttempt << RESET;
        //         //             // send a resync
        //         //             if( cResync)
        //         //                 fBeBoardInterface->ChipReSync ( cBoard );
        //         //             this->ReadNEvents ( cBoard , cEventsPerPoint);
        //         //             const std::vector<Event*>& cEvents = this->GetEvents ( cBoard );
        //         //             // matching 
        //         //             for (auto& cHybrid : cBoard->fModuleVector)
        //         //             {
        //         //                 auto cHybridId = cHybrid->getId();
        //         //                 for (auto& cChip : cHybrid->fReadoutChipVector) 
        //         //                 {
        //         //                     auto cChipId = cChip->getId();
        //         //                     if( std::find(pChipIds.begin(), pChipIds.end(), cChipId) == pChipIds.end() )
        //         //                         continue;

        //         //                     TProfile2D* cMatchedHitsTP = static_cast<TProfile2D*> ( getHist ( cChip, "MatchedHits_TestPulse" ) );
        //         //                     TProfile2D* cMatchedHitsLatency = static_cast<TProfile2D*> ( getHist ( cChip, "HitLatency" ) );
        //         //                     TProfile2D* cMatchedStubsLatency = static_cast<TProfile2D*> ( getHist ( cChip, "StubLatency" ) );
        //         //                     TProfile* cMatchedStubsHist = static_cast<TProfile*>( getHist(cChip, "PtCut") );

        //         //                     std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cChip );
        //         //                     // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
        //         //                     uint8_t cBendCode = cBendLUT[ ((cStub.second+cOffset)/2. - (-7.0))/0.5 ]; 
                                    
        //         //                     std::vector<uint8_t> cExpectedHits = static_cast<CbcInterface*>(fReadoutChipInterface)->stubInjectionPattern( cChip, cStub.first, cStub.second  ); 
        //         //                     LOG (DEBUG) << BOLDMAGENTA << "Injected a stub with seed " << +cStub.first << " with bend " << +cStub.second << RESET;
        //         //                     for(auto cHitExpected : cExpectedHits )
        //         //                         LOG (DEBUG) << BOLDMAGENTA << "\t.. expect a hit in channel " << +cHitExpected << RESET;
                                    
        //         //                     auto cEventIterator = cEvents.begin();
        //         //                     size_t cEventCounter=0;
        //         //                     LOG (INFO) << BOLDMAGENTA << "CBC" << +cChip->getId() << RESET;
        //         //                     size_t cMatchedHits=0;
        //         //                     size_t cMatchedStubs=0;
        //         //                     for( size_t cEventIndex=0; cEventIndex < cEventsPerPoint ; cEventIndex++) // for each event 
        //         //                     {
        //         //                         uint32_t cPipeline_first=0; 
        //         //                         uint32_t cBxId_first=0; 
        //         //                         bool cMissedEvent=false;

        //         //                         if( cEventIndex == 0 )
        //         //                             LOG (INFO) << BOLDMAGENTA << "'\tEvent " << +cEventIndex << RESET;
        //         //                         bool cIncorrectPipeline=false;
        //         //                         for(size_t cTriggerIndex=0; cTriggerIndex <= cTriggerMult; cTriggerIndex++) // cTriggerMult consecutive triggers were sent 
        //         //                         {
        //         //                             auto cEvent = *cEventIterator;
        //         //                             auto cBxId = cEvent->BxId(cHybrid->getId());
        //         //                             auto cErrorBit = cEvent->Error( cHybridId , cChipId );
        //         //                             uint32_t cL1Id = cEvent->L1Id( cHybridId, cChipId );
        //         //                             uint32_t cPipeline = cEvent->PipelineAddress( cHybridId, cChipId );
        //         //                             cBxId_first = (cTriggerIndex == 0 ) ? cBxId : cBxId_first;
        //         //                             cPipeline_first = (cTriggerIndex == 0 ) ? cPipeline : cPipeline_first;
        //         //                             bool cCountEvent = ( static_cast<size_t>(cPipeline - cPipeline_first) == cTriggerIndex );
        //         //                             if(cCountEvent)
        //         //                             {
        //         //                                 //hits
        //         //                                 auto cHits = cEvent->GetHits( cHybridId, cChipId ) ;
        //         //                                 size_t cMatched=0;
        //         //                                 int cLatency_eq = cLatency  - (cPipeline - cPipeline_first);
        //         //                                 double cTime_ns = 1*(static_cast<float>(fTPconfig.tpDelay - cLatency_eq)*25. + (25.0 - cTPdelay));
        //         //                                 for( auto cExpectedHit : cExpectedHits ) 
        //         //                                 {
        //         //                                     bool cMatchFound = std::find(  cHits.begin(), cHits.end(), cExpectedHit) != cHits.end();
        //         //                                     cMatched += cMatchFound;
        //         //                                     //cMatchedHits->Fill(cTriggerIndex,cPipeline, static_cast<int>(cMatchFound) );
        //         //                                     //cMatchedHitsEye->Fill(fPhaseTap, cTriggerIndex, static_cast<int>(cMatchFound) );
        //         //                                 }
        //         //                                 if( cMatched == cExpectedHits.size() )
        //         //                                 {
        //         //                                     cMatchedHits ++;
        //         //                                     cMatchedHitsLatency->Fill( cLatency_eq , cTPamplitude , 1 );
        //         //                                     cMatchedHitsTP->Fill( cTime_ns , cTPamplitude , 1);
        //         //                                 }
        //         //                                 else
        //         //                                 {
        //         //                                     cMatchedHitsLatency->Fill( cLatency_eq , cTPamplitude , 0 );
        //         //                                     cMatchedHitsTP->Fill( cTime_ns , cTPamplitude , 0);
        //         //                                 }
        //         //                                 //stubs
        //         //                                 auto cStubs = cEvent->StubVector( cHybridId, cChipId );
        //         //                                 int cNmatchedStubs=0;
        //         //                                 for( auto cHybridStub : cStubs ) 
        //         //                                 {
        //         //                                     LOG (DEBUG) << BOLDMAGENTA << "\t.. expected seed is " << +cStub.first << " measured seed is " << +cHybridStub.getPosition() << RESET;
        //         //                                     LOG (DEBUG) << BOLDMAGENTA << "\t.. expected bend code is 0x" << std::hex << +cBendCode << std::dec << " measured bend code is 0x" << std::hex <<  +cHybridStub.getBend() << std::dec << RESET;
        //         //                                     bool cMatchFound = (cHybridStub.getPosition() == cStub.first && cHybridStub.getBend() == cBendCode); 
        //         //                                     cMatchedStubsHist->Fill( cOffset , cMatchFound);
        //         //                                     cNmatchedStubs += static_cast<int>(cMatchFound);
        //         //                                 }
        //         //                                 if( cNmatchedStubs == 1 )
        //         //                                 {
        //         //                                     cMatchedStubs ++;
        //         //                                     cMatchedStubsLatency->Fill( cLatency_eq - cStubDelay  , cTPamplitude , 1 );
        //         //                                 }
        //         //                                 else
        //         //                                     cMatchedStubsLatency->Fill( cLatency_eq - cStubDelay  , cTPamplitude , 0 );
                                                
        //         //                                 int cBin = cMatchedHitsLatency->GetXaxis()->FindBin( cLatency_eq + cTPdelay*(1.0/25) );
        //         //                                 if( cEventIndex == 0 )
        //         //                                     LOG (INFO) << BOLDMAGENTA << "\t\t.. Trigger " << +cTriggerIndex << " Pipeline is " << +cPipeline << " -Latency is  " << +cLatency_eq << " , trigger sent " << fTPconfig.tpDelay+cLatencyOffset << " clocks after fast reset, TP delay is " << +cTPdelay << " L1A " << cLatency << " clocks. Time in ns is " << +cTime_ns << " .Found " << +cMatched << " matched hits and " <<  +cNmatchedStubs << " matched stubs." <<RESET; 
        //         //                             }
        //         //                             else
        //         //                                 cIncorrectPipeline = cIncorrectPipeline || true;
        //         //                             cEventIterator++;
        //         //                         }
        //         //                         // if missed on pipeline .. count this event

        //         //                     }
        //         //                 }
        //         //             }
        //         //             //this->print(pChipIds);
        //         //         }
        //         //     }
        //         // }
        //     }
        // }
        if( cConfigureTriggerMult )
            fBeBoardInterface->WriteBoardReg (theBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", cBoardTriggerMult);
        
    }
        
    // if TP was used - disable it
    // disable TP 
    this->enableTestPulse(false);
    this->SetTestPulse(false);
    setSameGlobalDac("TestPulsePotNodeSel",  0x00 );
    

    //unmask all channels and reset offsets 
    // also re-configure thresholds + hit/stub detect logic to original values 
    // and re-load configuration of fast command block from register map loaded from xml file 
    for (auto cBoard : *fDetectorContainer)
    {   
        auto& cThresholdsThisBoard = fThresholds.at(cBoard->getIndex());
        auto& cLogicThisBoard = fLogic.at(cBoard->getIndex());
        auto& cHIPsThisBoard = fHIPs.at(cBoard->getIndex());
        for(auto cOpticalGroup : *cBoard)
        {
            auto& cThresholdsThisOpticalGroup = cThresholdsThisBoard->at(cOpticalGroup->getIndex());
            auto& cLogicThisOpticalGroup = cLogicThisBoard->at(cOpticalGroup->getIndex());
            auto& cHIPsThisOpticalGroup = cHIPsThisBoard->at(cOpticalGroup->getIndex());

            for (auto cHybrid : *cOpticalGroup)
            {
                auto& cThresholdsThisHybrid = cThresholdsThisOpticalGroup->at(cHybrid->getIndex());
                auto& cLogicThisHybrid = cLogicThisOpticalGroup->at(cHybrid->getIndex());
                auto& cHIPsThisHybrid = cHIPsThisOpticalGroup->at(cHybrid->getIndex());
                static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (static_cast<OuterTrackerModule*>(cHybrid)->getLinkId());
                for (auto cChip : *cHybrid)
                {
                    ReadoutChip* theChip = static_cast<ReadoutChip*>(cChip);
                    static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( theChip, false);
                    // set offsets back to default value 
                    fReadoutChipInterface->WriteChipReg ( theChip, "CoincWind&Offset12", (0 << 4) | (0 << 0) );
                    fReadoutChipInterface->WriteChipReg ( theChip, "CoincWind&Offset34", (0 << 4) | (0 << 0) );

                    LOG (DEBUG) << BOLDBLUE << "Setting threshold on CBC" << +cChip->getId() << " back to " << +cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() << RESET ;
                    static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theChip, "VCth" , cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                    static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theChip, "Pipe&StubInpSel&Ptwidth" , cLogicThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                    static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theChip, "HIP&TestMode" , cHIPsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                }
            }
        }
        //
        LOG (DEBUG) << BOLDBLUE << "Re-loading original coonfiguration of fast command block from hardware description file [.xml] " << RESET;
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureFastCommandBlock(static_cast<BeBoard*>(cBoard));
    }

}
// check hits and stubs using noise
void DataChecker::DataCheck(std::vector<uint8_t> pChipIds, uint8_t pSeed , int pBend)
{
    LOG (INFO) << BOLDBLUE << "Starting data checker.." << RESET;
    std::pair<uint8_t, int> cStub;
        
    //Prepare container to hold  measured occupancy
    DetectorDataContainer     cMeasurement ;
    fDetectorDataContainer = &cMeasurement;
    ContainerFactory::copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    
    //use xml to figure out whether to use noise or charge injection 
    bool pWithNoise=true; //default is to use noise 
    auto cSetting = fSettingsMap.find ( "UseNoise" );
    if ( cSetting != std::end ( fSettingsMap ) )
        pWithNoise = ( cSetting->second == 1 );

    //get number of events from xml
    cSetting = fSettingsMap.find ( "Nevents" );
    uint32_t cEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 100;
    
    // get trigger rate from xml
    cSetting = fSettingsMap.find ( "TriggerRate" );
    bool cConfigureTrigger = ( cSetting != std::end ( fSettingsMap ) );
    uint16_t cTriggerRate = cConfigureTrigger ? cSetting->second : 100;
    // get trigger multiplicity from xml 
    cSetting = fSettingsMap.find ( "TriggerMultiplicity" );
    bool cConfigureTriggerMult = ( cSetting != std::end ( fSettingsMap ) );
    uint16_t cTriggerMult = cConfigureTriggerMult ? cSetting->second : 0;

    // get stub delay scan range from xml 
    cSetting = fSettingsMap.find ( "StubDelay" );
    bool cModifyStubScanRange = ( cSetting != std::end ( fSettingsMap ) );
    int cStubDelay = cModifyStubScanRange ? cSetting->second : 48 ;

    // get injected stub from xmls 
    cSetting = fSettingsMap.find ( "StubSeed" );
    cStub.first = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10; 
    cSetting = fSettingsMap.find ( "StubBend" );
    cStub.second = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0; 
    LOG (DEBUG) << BOLDBLUE << "Injecting a stub in position " << +cStub.first << " with bend " << cStub.second << " to test data integrity..." << RESET;

    // get target threshold 
    cSetting = fSettingsMap.find ( "Threshold" );
    uint16_t cTargetThreshold = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 580  ; 

    // get number of attempts 
    cSetting = fSettingsMap.find ( "Attempts" );
    size_t cAttempts = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10  ; 
    
    // get injected stub from xmls 
    cSetting = fSettingsMap.find ( "ManualPhaseAlignment" );
    if( ( cSetting != std::end ( fSettingsMap ) ) )
    {
        // fPhaseTap = cSetting->second ; 
        for (auto cBoard : *fDetectorContainer)
        {
        //     for(auto cOpticalGroup : *cBoard)
        //     {
        //         for (auto cHybrid : *cOpticalGroup)
        //         {
        //             auto& cCic = static_cast<OuterTrackerModule*>(cHybrid)->fCic;
        //             if( cCic != NULL )
        //             {
        //                 for(auto cChipId : pChipIds )
        //                 {
        //                     // bool cConfigured = fCicInterface->SetStaticPhaseAlignment(  cCic , cChipId ,  0 , fPhaseTap);
        //                 }
        //             }
        //         }
        //     }
            fBeBoardInterface->ChipReSync (static_cast<BeBoard*>(cBoard));
        }
    }

    // get number of attempts 
    cSetting = fSettingsMap.find ( "Mode" );
    uint8_t cMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0  ; 

    // get latency offset
    cSetting = fSettingsMap.find ( "LatencyOffset" );
    int cLatencyOffset = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0  ; 

    // resync between attempts 
    cSetting = fSettingsMap.find ( "ReSync" );
    bool cResync = ( cSetting != std::end ( fSettingsMap ) ) ? (cSetting->second==1) : false  ; 
    

    uint16_t cTPdelay = 0;

    // if TP is used - enable it 
    if(!pWithNoise)
    {
        cSetting = fSettingsMap.find ( "PulseShapePulseAmplitude" );
        fTPconfig.tpAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 100; 

        // get TP delay 
        cSetting = fSettingsMap.find("PulseShapePulseDelay");
        cTPdelay = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0  ; 
    
        // set-up for TP
        fAllChan = true;
        fMaskChannelsFromOtherGroups = !this->fAllChan;
        this->SetTestPulse(true);
        setSameGlobalDac("TestPulsePotNodeSel",  0xFF-fTPconfig.tpAmplitude );
        setSameGlobalDac("TestPulseDelay", cTPdelay);
        cStub.second = 0; // for now - with TP can only test bend code of 0 
    }

    // configure FE chips so that stubs are detected [i.e. make sure HIP
    // suppression is off ] 
    for (auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for (auto cHybrid : *cOpticalGroup)
            {
                //configure CBCs 
                for (auto cChip : *cHybrid)
                {
                    // switch off HitOr
                    static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "HitOr", 0);
                    //enable stub logic
                    if( cMode == 0 )
                        static_cast<CbcInterface*>(fReadoutChipInterface)->selectLogicMode( static_cast<ReadoutChip*>(cChip), "Sampled", true, true); 
                    else
                        static_cast<CbcInterface*>(fReadoutChipInterface)->selectLogicMode( static_cast<ReadoutChip*>(cChip), "Latched", true, true); 
                    static_cast<CbcInterface*>(fReadoutChipInterface)->enableHipSuppression( static_cast<ReadoutChip*>(cChip), false, false, 0);
                }
            }
        }
        fBeBoardInterface->ChipReSync (static_cast<BeBoard*>(cBoard));
    }

    // generate stubs in exactly chips with IDs that match pChipIds
    std::vector<int> cExpectedHits(0);
    for (auto cBoard : *fDetectorContainer)
    {
        for(auto cOpticalGroup : *cBoard)
        {
            for (auto cHybrid : *cOpticalGroup)
            {
                for (auto cChip : *cHybrid)
                {
                    ReadoutChip* theChip = static_cast<ReadoutChip*>(cChip);
                    if( std::find(pChipIds.begin(), pChipIds.end(), cChip->getId()) != pChipIds.end()  ) 
                    {
                        std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( theChip );
                        // both stub and bend are in units of half strips 
                        // if using TP then always inject a stub with bend 0 .. 
                        // later will use offset window to modify bend [ should probably put this in inject stub ]
                        uint8_t cBend_halfStrips = (pWithNoise) ? cStub.second : 0 ; 
                        static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( theChip , {cStub.first} , {cBend_halfStrips}, pWithNoise );
                        // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
                        // set offsets
                        // needs to be fixed 
                        /*
                        uint8_t cOffsetCode = static_cast<uint8_t>(std::fabs(cBend_strips*2)) | (std::signbit(-1*cBend_strips) << 3);
                        //uint8_t cOffsetCode = static_cast<uint8_t>(std::fabs(cBend_strips*2)) | (std::signbit(-1*cBend_strips) << 3);
                        // set offsets
                        uint8_t cOffetReg = (cOffsetCode << 4) | (cOffsetCode << 0);
                        LOG (INFO) << BOLDBLUE << "\t..--- bend code is 0x" << std::hex << +cBendCode << std::dec << " ..setting offset window to  " << std::bitset<8>(cOffetReg) << RESET;
                        fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset12", cOffetReg );
                        fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset34", cOffetReg );
                        */
                        if(!pWithNoise)
                            fReadoutChipInterface->enableInjection(theChip, true);
                    }
                    else if(!pWithNoise)
                        static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theChip, "VCth" , cTargetThreshold);
                    else
                    static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( theChip, true);
                }
            } 
        }
    }

    // zero containers
    //for( uint8_t cPackageDelay = 0 ; cPackageDelay < 8; cPackageDelay++)
    //{
        this->zeroContainers();
        // measure     
        for (auto cBoard : *fDetectorContainer)
        {
            BeBoard *theBoard = static_cast<BeBoard*>(cBoard);
            //LOG (INFO) << BOLDMAGENTA << "Setting stub package delay to " << +cPackageDelay << RESET;
            //fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_cnfg.physical_interface_block.cic.stub_package_delay", cPackageDelay);
            //static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->Bx0Alignment();

            uint16_t cBoardTriggerMult = fBeBoardInterface->ReadBoardReg (theBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
            uint16_t cBoardTriggerRate = fBeBoardInterface->ReadBoardReg (theBoard, "fc7_daq_cnfg.fast_command_block.user_trigger_frequency");
            LOG (DEBUG) << BOLDBLUE << "Trigger rate is set to " << +cBoardTriggerRate << " kHz" << RESET ; 
            LOG (DEBUG) << BOLDBLUE << "Trigger multiplicity is set to " << +cBoardTriggerMult << " consecutive triggers per L1A." << RESET ; 
            if( cConfigureTrigger && pWithNoise)
            {
                LOG (DEBUG) << BOLDBLUE << "Modifying trigger rate to be " << +cTriggerRate << " kHz for DataTest" << RESET;
                fBeBoardInterface->WriteBoardReg (theBoard, "fc7_daq_cnfg.fast_command_block.user_trigger_frequency", cTriggerRate);
            }
            if( cConfigureTriggerMult )
            {
                LOG (DEBUG) << BOLDBLUE << "Modifying trigger multiplicity to be " << +(1+cTriggerMult) << " consecutive triggers per L1A for DataTest" << RESET;
                fBeBoardInterface->WriteBoardReg (theBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", cTriggerMult);
            }
            
            // using charge injection 
            if( !pWithNoise)
            {
                // configure test pulse trigger 
                static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTestPulseFSM(fTPconfig.firmwareTPdelay ,fTPconfig.tpDelay ,fTPconfig.tpSequence, fTPconfig.tpFastReset );
                // set trigger latency 
                uint16_t cLatency = fTPconfig.tpDelay+cLatencyOffset;//+1;
                this->setSameDacBeBoard(theBoard, "TriggerLatency", cLatency);
                // set stub latency 
                uint16_t cStubLatency = cLatency - 1*cStubDelay ;
                fBeBoardInterface->WriteBoardReg (theBoard, "fc7_daq_cnfg.readout_block.global.common_stubdata_delay", cStubLatency);
                LOG (DEBUG) << BOLDBLUE << "Latency set to " << +cLatency << "...\tStub latency set to " << +cStubLatency << RESET;
                fBeBoardInterface->ChipReSync ( theBoard );
            }
            // read N events and compare hits and stubs to injected stub 
            for( size_t cAttempt=0; cAttempt < cAttempts ; cAttempt++)
            {
                fAttempt = cAttempt;
                LOG (INFO) << BOLDBLUE << "Iteration# " << +fAttempt << RESET;
                // send a resync
                if( cResync)
                    fBeBoardInterface->ChipReSync ( theBoard );
                this->ReadNEvents ( theBoard , cEventsPerPoint);
                this->matchEvents( theBoard , pChipIds ,cStub);
                this->print(pChipIds);
            }
            

            // and set it back to what it was 
            if( cConfigureTrigger )
                fBeBoardInterface->WriteBoardReg (theBoard, "fc7_daq_cnfg.fast_command_block.user_trigger_frequency", cBoardTriggerRate);
            if( cConfigureTriggerMult )
                fBeBoardInterface->WriteBoardReg (theBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", cBoardTriggerMult);
        }
    //}

    // if TP was used - disable it
    if(!pWithNoise)
    {
        // disable TP 
        this->enableTestPulse(false);
        this->SetTestPulse(false);
        setSameGlobalDac("TestPulsePotNodeSel",  0x00 );
    }

    //unmask all channels and reset offsets 
    // also re-configure thresholds + hit/stub detect logic to original values 
    // and re-load configuration of fast command block from register map loaded from xml file 
    for (auto cBoard : *fDetectorContainer)
    {   
        auto& cThresholdsThisBoard = fThresholds.at(cBoard->getIndex());
        auto& cLogicThisBoard = fLogic.at(cBoard->getIndex());
        auto& cHIPsThisBoard = fHIPs.at(cBoard->getIndex());
        for(auto cOpticalGroup : *cBoard)
        {
            auto& cThresholdsThisOpticalGroup = cThresholdsThisBoard->at(cOpticalGroup->getIndex());
            auto& cLogicThisOpticalGroup = cLogicThisBoard->at(cOpticalGroup->getIndex());
            auto& cHIPsThisOpticalGroup = cHIPsThisBoard->at(cOpticalGroup->getIndex());
            for (auto cHybrid : *cOpticalGroup)
            {
                auto& cThresholdsThisHybrid = cThresholdsThisOpticalGroup->at(cOpticalGroup->getIndex());
                auto& cLogicThisHybrid = cLogicThisOpticalGroup->at(cOpticalGroup->getIndex());
                auto& cHIPsThisHybrid = cHIPsThisOpticalGroup->at(cOpticalGroup->getIndex());
                static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (static_cast<OuterTrackerModule*>(cHybrid)->getLinkId());
                for (auto cChip : *cHybrid)
                {
                    ReadoutChip* theChip = static_cast<ReadoutChip*>(cChip);
                    static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( theChip, false);
                    // set offsets back to default value 
                    fReadoutChipInterface->WriteChipReg ( theChip, "CoincWind&Offset12", (0 << 4) | (0 << 0) );
                    fReadoutChipInterface->WriteChipReg ( theChip, "CoincWind&Offset34", (0 << 4) | (0 << 0) );

                    LOG (DEBUG) << BOLDBLUE << "Setting threshold on CBC" << +cChip->getId() << " back to " << +cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() << RESET ;
                    static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theChip, "VCth" , cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                    static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theChip, "Pipe&StubInpSel&Ptwidth" , cLogicThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                    static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( theChip, "HIP&TestMode" , cHIPsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                }
            }
        }
        //
        LOG (DEBUG) << BOLDBLUE << "Re-loading original coonfiguration of fast command block from hardware description file [.xml] " << RESET;
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureFastCommandBlock(static_cast<BeBoard*>(cBoard));
        //
        //fBeBoardInterface->ChipReSync ( cBoard );
    }
}

// check L1 eye 
void DataChecker::L1Eye(std::vector<uint8_t> pChipIds )
{
    //uint8_t cChipId=0;
    //uint8_t cSeed=10;
    //uint8_t cBendCode=0;
    
    BackEndAlignment cBackEndAligner; 
    cBackEndAligner.Inherit (this);
    cBackEndAligner.Initialise();
    for( uint8_t cPhase=4; cPhase < 14; cPhase +=1 )
    {
        fPhaseTap = cPhase;
        // zero container 
        zeroContainers();
        LOG (INFO) << BOLDBLUE << "Setting optimal phase tap in CIC to " << +cPhase << RESET;
        for (auto cBoard : *fDetectorContainer)
        {
            // for (auto cOpticalGroup : *cBoard)
            // {
            //     for (auto& cHybrid : *cOpticalGroup)
            //     {
            //         auto& cCic = static_cast<OuterTrackerModule*>(cHybrid)->fCic;
            //         //fCicInterface->ResetPhaseAligner(cCic);
            //         for(auto cChipId : pChipIds )
            //         {
            //             // bool cConfigured = fCicInterface->SetStaticPhaseAlignment(  cCic , cChipId ,  0 , cPhase);
            //             // check if a resync is needed
            //             //fCicInterface->CheckReSync( static_cast<OuterTrackerModule*>(cHybrid)->fCic); 
            //         }
            //     }
            // }
            // send a resync
            fBeBoardInterface->ChipReSync ( static_cast<BeBoard*>(cBoard) );
            // re-do back-end alignment
            //cBackEndAligner.L1Alignment2S(cBoard);    
            std::this_thread::sleep_for (std::chrono::milliseconds (100) );
        }
        // run data check 
        this->DataCheck(pChipIds);
        // print results
        //this->print({cChipId});
    }

}


void DataChecker::writeObjects()
{
    this->SaveResults();
    fResultFile->Flush();

}
// State machine control functions
void DataChecker::Start(int currentRun)
{
    Initialise ();
}

void DataChecker::Stop()
{
    this->SaveResults();
    fResultFile->Flush();
    dumpConfigFiles();

    SaveResults();
    CloseResultFile();
    Destroy();
}

void DataChecker::Pause()
{
}

void DataChecker::Resume()
{
}

#endif
