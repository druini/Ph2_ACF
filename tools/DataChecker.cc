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
    // this is needed if you're going to use groups anywhere
    fChannelGroupHandler = new CBCChannelGroupHandler();//This will be erased in tool.resetPointers()
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    #ifdef __USE_ROOT__
    //    fDQMHistogram.book(fResultFile,*fDetectorContainer);
    #endif
    
    // retreive original settings for all chips 
    ContainerFactory::copyAndInitStructure<ChipRegMap>(*fDetectorContainer, fRegMapContainer);
    for (auto cBoard : this->fBoardVector)
    {
        auto& cRegMapThisBoard = fRegMapContainer.at(cBoard->getIndex());
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cRegMapThisHybrid = cRegMapThisBoard->at(cFe->getIndex());
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                cRegMapThisHybrid->at(cChip->getIndex())->getSummary<ChipRegMap>() = cChip->getRegMap();
            }
        }
    }

    for (auto cBoard : this->fBoardVector)
    {
        for (auto& cFe : cBoard->fModuleVector)
        {
            
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                // matched hits 
                TString cName = Form ( "h_Hits_Fe%dCbc%d", cFe->getFeId() , cChip->getChipId() );
                TObject* cObj = gROOT->FindObject ( cName );
                if ( cObj ) delete cObj;
                TH2D* cHist2D = new TH2D ( cName, Form("Number of hits - CBC%d; Trigger Number; Pipeline Address",(int)cChip->getChipId()) , 40  , 0 -0.5 , 40 -0.5 , 520 , 0-0.5 , 520-0.5 );
                bookHistogram ( cChip , "Hits_perFe", cHist2D );

                cName = Form ( "h_MatchedHits_Fe%dCbc%d", cFe->getFeId() , cChip->getChipId() );
                cObj = gROOT->FindObject ( cName );
                if ( cObj ) delete cObj;
                cHist2D = new TH2D ( cName, Form("Number of matched hits - CBC%d; Trigger Number; Pipeline Address",(int)cChip->getChipId()) ,40  , 0 -0.5 , 40 -0.5 , 520 , 0-0.5 , 520-0.5 );
                bookHistogram ( cChip , "MatchedHits_perFe", cHist2D );

                cName = Form ( "h_EyeL1_Fe%dCbc%d", cFe->getFeId() , cChip->getChipId() );
                cObj = gROOT->FindObject ( cName );
                if ( cObj ) delete cObj;
                TProfile2D* cProfile2D = new TProfile2D ( cName, Form("Number of matched hits - CBC%d; Phase Tap; Trigger Number",(int)cChip->getChipId()) ,  20 , 0-0.5 , 20-0.5 , 40 , 0-0.5 , 40-0.5);
                bookHistogram ( cChip , "MatchedHits_eye", cProfile2D );
            }

            // matched stubs 
            TString cName = Form ( "h_Stubs_Cic%d", cFe->getFeId() );
            TObject* cObj = gROOT->FindObject ( cName );
            if ( cObj ) delete cObj;
            TH2D* cHist2D = new TH2D ( cName, Form("Number of stubs - CIC%d; Trigger Number; Bunch Crossing Id",(int)cFe->getFeId()) , 40  , 0 -0.5 , 40 -0.5 , 4000 , 0-0.5 , 4000-0.5 );
            bookHistogram ( cFe , "Stubs", cHist2D );

            cName = Form ( "h_MatchedStubs_Cic%d", cFe->getFeId() );
            cObj = gROOT->FindObject ( cName );
            if ( cObj ) delete cObj;
            cHist2D = new TH2D ( cName, Form("Number of matched stubs - CIC%d; Trigger Number; Bunch Crossing Id",(int)cFe->getFeId()) , 40  , 0 -0.5 , 40 -0.5 , 4000 , 0-0.5 , 4000-0.5 );
            bookHistogram ( cFe , "MatchedStubs", cHist2D );

            // TProfile* cProfile = new TProfile ( cName, Form("Number of matched stubs - CIC%d; CBC; Fraction of matched stubs",(int)cFe->getFeId()) ,8  , 0 -0.5 , 8 -0.5 );
            // bookHistogram ( cFe , "MatchedStubs", cProfile );
            
            // matched hits 
            // cName = Form ( "h_MatchedHits");
            // cObj = gROOT->FindObject ( cName );
            // if ( cObj ) delete cObj;
            // cProfile = new TProfile ( cName, Form("Number of matched hits - CIC%d; CBC; Fraction of matched hits",(int)cFe->getFeId()) ,8  , 0 -0.5 , 8 -0.5 );
            // bookHistogram ( cFe , "MatchedHits", cProfile );

            // cName = Form ( "h_L1Status_Fe%d", cFe->getFeId() );
            // cObj = gROOT->FindObject ( cName );
            // if ( cObj ) delete cObj;
            // TH2D* cHist2D = new TH2D ( cName, Form("Error Flag CIC%d; Event Id; Chip Id; Error Bit",(int)cFe->getFeId()) , 1000, 0 , 1000 , 9, 0-0.5 , 9-0.5 );
            // bookHistogram ( cFe, "L1Status", cHist2D ); 
        }
    }

    // read original thresholds from chips ... 
    fDetectorDataContainer = &fThresholds;
    ContainerFactory::copyAndInitStructure<uint16_t>(*fDetectorContainer, fThresholds);  
    // read original logic configuration from chips .. [Pipe&StubInpSel&Ptwidth , HIP&TestMode]
    fDetectorDataContainer = &fLogic;
    ContainerFactory::copyAndInitStructure<uint16_t>(*fDetectorContainer, fLogic);  
    fDetectorDataContainer = &fHIPs;
    ContainerFactory::copyAndInitStructure<uint16_t>(*fDetectorContainer, fHIPs);  
    for (auto cBoard : this->fBoardVector)
    {
        auto& cThresholdsThisBoard = fThresholds.at(cBoard->getIndex());
        auto& cLogicThisBoard = fLogic.at(cBoard->getIndex());
        auto& cHIPsThisBoard = fHIPs.at(cBoard->getIndex());
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cThresholdsThisHybrid = cThresholdsThisBoard->at(cFe->getIndex());
            auto& cLogicThisHybrid = cLogicThisBoard->at(cFe->getIndex());
            auto& cHIPsThisHybrid = cHIPsThisBoard->at(cFe->getIndex());
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
            //configure CBCs 
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cChip, "VCth" );
                cLogicThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cChip, "Pipe&StubInpSel&Ptwidth" );
                cHIPsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cChip, "HIP&TestMode" );
            }
        }
    }

    // for stub + hit data check 
    ContainerFactory::copyAndInitStructure<int>(*fDetectorContainer, fHitCheckContainer);
    ContainerFactory::copyAndInitStructure<int>(*fDetectorContainer, fStubCheckContainer);
    for (auto cBoard : this->fBoardVector)
    {
        auto& cHitCheck = fHitCheckContainer.at(cBoard->getIndex());
        auto& cStubCheck = fStubCheckContainer.at(cBoard->getIndex());
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cHitCheckThisHybrid = cHitCheck->at(cFe->getIndex());
            auto& cStubCheckThisHybrid = cStubCheck->at(cFe->getIndex());
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                cHitCheckThisHybrid->at(cChip->getIndex())->getSummary<int>() = 0;
                cStubCheckThisHybrid->at(cChip->getIndex())->getSummary<int>() = 0;
            }
        }
    }

}

void DataChecker::zeroContainers()
{
    for (auto cBoard : this->fBoardVector)
    {
        auto& cThisHitCheckContainer = fHitCheckContainer.at(cBoard->getIndex());
        auto& cThisStubCheckContainer = fStubCheckContainer.at(cBoard->getIndex());
        // zero container 
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cHybridHitCheck = cThisHitCheckContainer->at(cFe->getIndex());
            auto& cHybridStubCheck = cThisStubCheckContainer->at(cFe->getIndex());  
            
            for (auto& cChip : cFe->fReadoutChipVector) 
            {
                auto& cReadoutChipHitCheck = cHybridHitCheck->at(cChip->getIndex());
                auto& cReadoutChipStubCheck = cHybridStubCheck->at(cChip->getIndex());

                cReadoutChipHitCheck->getSummary<int>() = 0;
                cReadoutChipStubCheck->getSummary<int>() = 0 ;
            }
        }
    }
}
void DataChecker::print(std::vector<uint8_t> pChipIds )
{
    for (auto cBoard : this->fBoardVector)
    {
        LOG (DEBUG) << BOLDBLUE << "\t..For BeBoard" << +cBoard->getIndex() << RESET;
        auto& cThisHitCheckContainer = fHitCheckContainer.at(cBoard->getIndex());
        auto& cThisStubCheckContainer = fStubCheckContainer.at(cBoard->getIndex());

        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cHybridHitCheck = cThisHitCheckContainer->at(cFe->getIndex());
            auto& cHybridStubCheck = cThisStubCheckContainer->at(cFe->getIndex());  
            
            for (auto& cChip : cFe->fReadoutChipVector) 
            {
                auto& cReadoutChipHitCheck = cHybridHitCheck->at(cChip->getIndex());
                auto& cReadoutChipStubCheck = cHybridStubCheck->at(cChip->getIndex());

                auto cChipId = cChip->getChipId();
                if( std::find(pChipIds.begin(), pChipIds.end(), cChipId) == pChipIds.end() )
                    continue;

                auto& cHitCheck = cReadoutChipHitCheck->getSummary<int>();
                auto& cStubCheck = cReadoutChipStubCheck->getSummary<int>();
                LOG (INFO) << BOLDBLUE << "\t\t...Found " << +cHitCheck << " matched hits and " << +cStubCheck << " matched stubs in readout chip" << +cChipId << RESET;
            }
        }
    }
}
void DataChecker::matchEvents(BeBoard* pBoard, std::vector<uint8_t>pChipIds , std::pair<uint8_t,int> pExpectedStub) 
{
    // get trigger multiplicity from xml 
    auto cSetting = fSettingsMap.find ( "TriggerMultiplicity" );
    bool cConfigureTriggerMult = ( cSetting != std::end ( fSettingsMap ) );
    size_t cTriggerMult = cConfigureTriggerMult ? cSetting->second : 0;

    //get number of events from xml
    cSetting = fSettingsMap.find ( "Nevents" );
    size_t cEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 100;
   
    uint8_t cSeed = pExpectedStub.first;
    int cBend = pExpectedStub.second; 

    auto& cThisHitCheckContainer = fHitCheckContainer.at(pBoard->getIndex());
    auto& cThisStubCheckContainer = fStubCheckContainer.at(pBoard->getIndex());
    
    const std::vector<Event*>& cEvents = this->GetEvents ( pBoard );
    LOG (DEBUG) << BOLDMAGENTA << "Read back " << +cEvents.size() << " events from board." << RESET;
    for (auto& cFe : pBoard->fModuleVector)
    {
        auto& cHybridHitCheck = cThisHitCheckContainer->at(cFe->getIndex());
        auto& cHybridStubCheck = cThisStubCheckContainer->at(cFe->getIndex());  
    

        auto& cCic = static_cast<OuterTrackerModule*>(cFe)->fCic;
        auto cFeId = cFe->getFeId();
        TH2D* cMatchedStubs = static_cast<TH2D*> ( getHist ( cFe, "MatchedStubs" ) );
        TH2D* cAllStubs = static_cast<TH2D*> ( getHist ( cFe, "Stubs" ) );
            
        for (auto& cChip : cFe->fReadoutChipVector) 
        {
            auto cChipId = cChip->getChipId();
            if( std::find(pChipIds.begin(), pChipIds.end(), cChipId) == pChipIds.end() )
                continue;

            TH2D* cAllHits = static_cast<TH2D*> ( getHist ( cChip, "Hits_perFe" ) );
            TH2D* cMatchedHits = static_cast<TH2D*> ( getHist ( cChip, "MatchedHits_perFe" ) );
            TProfile2D* cMatchedHitsEye = static_cast<TProfile2D*> ( getHist ( cChip, "MatchedHits_eye" ) );

            // container for this chip 
            auto& cReadoutChipHitCheck = cHybridHitCheck->at(cChip->getIndex());
            auto& cReadoutChipStubCheck = cHybridStubCheck->at(cChip->getIndex());

            std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cChip );
            // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
            uint8_t cBendCode = cBendLUT[ (cBend/2. - (-7.0))/0.5 ]; 
            std::vector<uint8_t> cExpectedHits = static_cast<CbcInterface*>(fReadoutChipInterface)->stubInjectionPattern( cChip, cSeed, cBend ); 
            
            auto cEventIterator = cEvents.begin();
            size_t cEventCounter=0;
            LOG (DEBUG) << BOLDMAGENTA << "CBC" << +cChip->getChipId() << RESET;
            for( size_t cEventIndex=0; cEventIndex < cEventsPerPoint ; cEventIndex++) // for each event 
            {
                uint32_t cPipeline_first=0; 
                uint32_t cBxId_first=0; 
                LOG (DEBUG) << BOLDMAGENTA << "\t..Event" << +cEventIndex << RESET;
            
                for(size_t cTriggerIndex=0; cTriggerIndex <= cTriggerMult; cTriggerIndex++) // cTriggerMult consecutive triggers were sent 
                {
                    auto cEvent = *cEventIterator;
                    auto cBxId = cEvent->BxId(cFe->getFeId());
                    auto cErrorBit = cEvent->Error( cFeId , cChipId );
                    uint32_t cL1Id = cEvent->L1Id( cFeId, cChipId );
                    uint32_t cPipeline = cEvent->PipelineAddress( cFeId, cChipId );
                    cBxId_first = (cTriggerIndex == 0 ) ? cBxId : cBxId_first;
                    cPipeline_first = (cTriggerIndex == 0 ) ? cPipeline : cPipeline_first;
                    //LOG (INFO) << BOLDBLUE << "Chip" << +cChipId << " : Pipeline address " << +cPipeline_first  << " -- bx id is " << +cBxId_first << RESET;

                    //hits
                    auto cHits = cEvent->GetHits( cFeId, cChipId ) ;
                    cAllHits->Fill(cTriggerIndex,cPipeline,cHits.size());
                    for( auto cHit : cHits )
                    {
                        LOG (DEBUG) << BOLDMAGENTA << "\t... hit found in channel " << +cHit << " of readout chip" << +cChipId << RESET; 
                    }
                    size_t cMatched=0;
                    for( auto cExpectedHit : cExpectedHits ) 
                    {
                        bool cMatchFound = std::find(  cHits.begin(), cHits.end(), cExpectedHit) != cHits.end();
                        cMatched += cMatchFound;
                        cMatchedHits->Fill(cTriggerIndex,cPipeline, static_cast<int>(cMatchFound) );
                        cMatchedHitsEye->Fill(fPhaseTap, cTriggerIndex, static_cast<int>(cMatchFound) );
                    }
                    if( cMatched == cExpectedHits.size() )
                    {
                        auto& cOcc = cReadoutChipHitCheck->getSummary<int>();
                        cOcc += static_cast<int>(cMatched == cExpectedHits.size());
                    }
                    //LOG (DEBUG) << BOLDMAGENTA << +cMatched << " matched hits found in readout chip" << +cChipId << " [ " << +cExpectedHits.size() << " expected.]" << RESET; 
                    
                    //stubs
                    auto cStubs = cEvent->StubVector( cFeId, cChipId );
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
                    LOG (DEBUG) << BOLDMAGENTA << "\t\t...Trigger number " << +cTriggerIndex <<   " " << +cMatched << " matched hits found [ " << +cHits.size() << " in total]. " <<  +cNmatchedStubs << " matched stubs." <<RESET; 
                    
                    cEventIterator++;
                }
            }
        }
   }
}
// check hits and stubs using noise
void DataChecker::DataCheck(std::vector<uint8_t> pChipIds, uint8_t pSeed , int pBend)
{
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
    uint16_t cDefaultStubLatency=50;

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
    cSetting = fSettingsMap.find ( "TargetVcth" );
    uint16_t cTargetThreshold = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 580  ; 

    // get number of attempts 
    cSetting = fSettingsMap.find ( "Attempts" );
    size_t cAttempts = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10  ; 

    // get injected stub from xmls 
    cSetting = fSettingsMap.find ( "ManualPhaseAlignment" );
    if( ( cSetting != std::end ( fSettingsMap ) ) )
    {
        fPhaseTap = cSetting->second ; 
        for (auto cBoard : this->fBoardVector)
        {
            for (auto& cFe : cBoard->fModuleVector)
            {
                auto& cCic = static_cast<OuterTrackerModule*>(cFe)->fCic;
                for(auto cChipId : pChipIds )
                {
                    bool cConfigured = fCicInterface->SetStaticPhaseAlignment(  cCic , cChipId ,  0 , fPhaseTap);
                }
            }
        }
    }

    // get number of attempts 
    cSetting = fSettingsMap.find ( "Mode" );
    uint8_t cMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0  ; 

    // if TP is used - enable it 
    if(!pWithNoise)
    {
        cSetting = fSettingsMap.find ( "PulseShapePulseAmplitude" );
        fTPconfig.tpAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 100; 

        // set-up for TP
        fAllChan = true;
        fMaskChannelsFromOtherGroups = !this->fAllChan;
        this->enableTestPulse(true);
        this->SetTestPulse(true);
        setSameGlobalDac("TestPulsePotNodeSel",  0xFF-fTPconfig.tpAmplitude );
        cStub.second = 0; // for now - with TP can only test bend code of 0 
    }

    // configure FE chips so that stubs are detected [i.e. make sure HIP
    // suppression is off ] 
    for (auto cBoard : this->fBoardVector)
    {
        for (auto& cFe : cBoard->fModuleVector)
        {
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
            //configure CBCs 
            for (auto& cChip : cFe->fReadoutChipVector)
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
        fBeBoardInterface->ChipReSync ( cBoard );
    }

    // generate stubs in exactly chips with IDs that match pChipIds
    size_t cSeedIndex=0;
    std::vector<int> cExpectedHits(0);
    for (auto cBoard : this->fBoardVector)
    {   
        for (auto& cFe : cBoard->fModuleVector)
        {
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                if( std::find(pChipIds.begin(), pChipIds.end(), cChip->getChipId()) != pChipIds.end()  ) 
                {
                    std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cChip );
                    // both stub and bend are in units of half strips 
                    double cBend_strips = cStub.second/2.;
                    // if using TP then always inject a stub with bend 0 .. 
                    // later will use offset window to modify bend [ should probably put this in inject stub ]
                    uint8_t cBend_halfStrips = (pWithNoise) ? cStub.second : 0 ; 
                    static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( cChip , {cStub.first} , {cBend_halfStrips}, pWithNoise );
                    // each bend code is stored in this vector - bend encoding start at -7 strips, increments by 0.5 strips
                    uint8_t cBendCode = cBendLUT[ (pBend/2. - (-7.0))/0.5 ]; 
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
                    static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( cChip, "VCth" , cTargetThreshold);
                }
                else
                    static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( cChip, true);
            } 
        }
    }

    // configure trigger generation in firmware 
    //static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTriggerFSM(0,cTriggerRate,3,0,cDefaultStubLatency);
    for (auto cBoard : this->fBoardVector)
    {
        uint16_t cBoardTriggerMult = fBeBoardInterface->ReadBoardReg (cBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
        uint16_t cBoardTriggerRate = fBeBoardInterface->ReadBoardReg (cBoard, "fc7_daq_cnfg.fast_command_block.user_trigger_frequency");
        LOG (DEBUG) << BOLDBLUE << "Trigger rate is set to " << +cBoardTriggerRate << " kHz" << RESET ; 
        LOG (DEBUG) << BOLDBLUE << "Trigger multiplicity is set to " << +cBoardTriggerMult << " consecutive triggers per L1A." << RESET ; 
        if( cConfigureTrigger && pWithNoise)
        {
            LOG (DEBUG) << BOLDBLUE << "Modifying trigger rate to be " << +cTriggerRate << " kHz for DataTest" << RESET;
            fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_cnfg.fast_command_block.user_trigger_frequency", cTriggerRate);
        }
        if( cConfigureTriggerMult )
        {
            LOG (DEBUG) << BOLDBLUE << "Modifying trigger multiplicity to be " << +(1+cTriggerMult) << " consecutive triggers per L1A for DataTest" << RESET;
            fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", cTriggerMult);
        }
        
        // using noise injection 
        if( pWithNoise )
        {
            this->ReadNEvents ( cBoard , cEventsPerPoint);
            this->matchEvents( cBoard , pChipIds ,cStub);
        }
        // using charge injection 
        else if(!pWithNoise)
        {
            // configure test pulse trigger 
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTestPulseFSM(fTPconfig.firmwareTPdelay ,fTPconfig.tpDelay ,fTPconfig.tpSequence, fTPconfig.tpFastReset );
            uint16_t cTPdelay=0;
            this->setSameDacBeBoard(cBoard, "TestPulseDelay", cTPdelay);
            // set trigger latency 
            uint16_t cLatency = fTPconfig.tpDelay+1;
            this->setSameDacBeBoard(cBoard, "TriggerLatency", cLatency);
            // set stub latency 
            uint16_t cStubLatency = cLatency - 1*cStubDelay ;
            fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_cnfg.readout_block.global.common_stubdata_delay", cStubLatency);
            LOG (DEBUG) << BOLDBLUE << "Latency set to " << +cLatency << "...\tStub latency set to " << +cStubLatency << RESET;
            fBeBoardInterface->ChipReSync ( cBoard );
            // zero containers
            this->zeroContainers();
            // read N events and compare hits and stubs to injected stub 
            for( size_t cAttempt=0; cAttempt < cAttempts ; cAttempt++)
            {
                // send a resync
                fBeBoardInterface->ChipReSync ( cBoard );
                this->ReadNEvents ( cBoard , cEventsPerPoint);
                this->matchEvents( cBoard , pChipIds ,cStub);
            }
        }
        this->print(pChipIds);
        

        // and set it back to what it was 
        if( cConfigureTrigger )
            fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_cnfg.fast_command_block.user_trigger_frequency", cBoardTriggerRate);
        if( cConfigureTriggerMult )
            fBeBoardInterface->WriteBoardReg (cBoard, "fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity", cBoardTriggerMult);
    }

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
    for (auto cBoard : this->fBoardVector)
    {   
        auto& cThresholdsThisBoard = fThresholds.at(cBoard->getIndex());
        auto& cLogicThisBoard = fLogic.at(cBoard->getIndex());
        auto& cHIPsThisBoard = fHIPs.at(cBoard->getIndex());
        for (auto& cFe : cBoard->fModuleVector)
        {
            auto& cThresholdsThisHybrid = cThresholdsThisBoard->at(cFe->getIndex());
            auto& cLogicThisHybrid = cLogicThisBoard->at(cFe->getIndex());
            auto& cHIPsThisHybrid = cHIPsThisBoard->at(cFe->getIndex());
            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
            for (auto& cChip : cFe->fReadoutChipVector)
            {
                static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( cChip, false);
                // set offsets back to default value 
                fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset12", (0 << 4) | (0 << 0) );
                fReadoutChipInterface->WriteChipReg ( cChip, "CoincWind&Offset34", (0 << 4) | (0 << 0) );

                LOG (DEBUG) << BOLDBLUE << "Setting threshold on CBC" << +cChip->getChipId() << " back to " << +cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() << RESET ;
                static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( cChip, "VCth" , cThresholdsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( cChip, "Pipe&StubInpSel&Ptwidth" , cLogicThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
                static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( cChip, "HIP&TestMode" , cHIPsThisHybrid->at(cChip->getIndex())->getSummary<uint16_t>() );
            }
        }
        //
        LOG (DEBUG) << BOLDBLUE << "Re-loading original coonfiguration of fast command block from hardware description file [.xml] " << RESET;
        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureFastCommandBlock(cBoard);
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
        for (auto cBoard : this->fBoardVector)
        {
            for (auto& cFe : cBoard->fModuleVector)
            {
                auto& cCic = static_cast<OuterTrackerModule*>(cFe)->fCic;
                //fCicInterface->ResetPhaseAligner(cCic);
                for(auto cChipId : pChipIds )
                {
                    bool cConfigured = fCicInterface->SetStaticPhaseAlignment(  cCic , cChipId ,  0 , cPhase);
                    //fCicInterface->ResetPhaseAligner(cCic);
                    //LOG (INFO) << BOLDBLUE << "Checking Reset/Resync for CIC on hybrid " << +cFe->getFeId() << RESET;
                    // check if a resync is needed
                    //fCicInterface->CheckReSync( static_cast<OuterTrackerModule*>(cFe)->fCic); 
                }
            }
            // re-do back-end alignment
            //cBackEndAligner.L1Alignment2S(cBoard);    
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
