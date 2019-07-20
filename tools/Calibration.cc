#include "Calibration.h"
#include "../Utils/CBCChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"

//initialize the static member
//std::map<Chip*, uint16_t> Calibration::fVplusMap;

Calibration::Calibration() :
    Tool            (),
    fVplusMap       (),
    fVplusCanvas    (nullptr),
    fOffsetCanvas   (nullptr),
    fOccupancyCanvas(nullptr),
    fNCbc           (0),
    fNFe            (0)
{
}

Calibration::~Calibration()
{
    // delete fOffsetCanvas;
    // delete fOccupancyCanvas;
}

void Calibration::Initialise ( bool pAllChan, bool pDisableStubLogic )
{
    fDisableStubLogic = pDisableStubLogic;
    // Initialize the TestGroups

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
    // This new it is dangerous since it may cause a memory leak!!! Please handle it //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
    fChannelGroupHandler = new CBCChannelGroupHandler();
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    this->fAllChan = pAllChan;
    
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "HoleMode" );
    fHoleMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "TargetVcth" );
    fTargetVcth = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x78;
    cSetting = fSettingsMap.find ( "TargetOffset" );
    fTargetOffset = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x80;
    cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    cSetting = fSettingsMap.find ( "TestPulseAmplitude" );
    fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    cSetting = fSettingsMap.find ( "VerificationLoop" );
    fCheckLoop = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "MaskChannelsFromOtherGroups" );
    fMaskChannelsFromOtherGroups = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "SkipMaskedChannels" );
    fSkipMaskedChannels = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;

    this->SetSkipMaskedChannels( fSkipMaskedChannels );


    if ( fTestPulseAmplitude == 0 ) fTestPulse = 0;
    else fTestPulse = 1;

    // Canvases
    //fVplusCanvas = new TCanvas ( "VPlus", "VPlus", 515, 0, 500, 500 );
    fOffsetCanvas    = new TCanvas ( "Offset", "Offset", 10, 0, 500, 500 );
    fOccupancyCanvas = new TCanvas ( "Occupancy", "Occupancy", 10, 525, 500, 500 );

    // count FEs & CBCs
    uint32_t cCbcCount = 0;
    uint32_t cCbcIdMax = 0;
    uint32_t cFeCount = 0;


    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();
            cFeCount++;
            fType = cFe->getFrontEndType();

            for ( auto cCbc : cFe->fReadoutChipVector )
            {
                LOG (INFO) << BOLDBLUE << "CBC" << +cCbc->getChipId() << "[ " << +cCbcIdMax << "]" << RESET; 
                //if it is a CBC3, disable the stub logic for this procedure
                if (cCbc->getFrontEndType() == FrontEndType::CBC3 && fDisableStubLogic)
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - thus disabling Stub logic for offset tuning" << RESET ;
                    fStubLogicValue[cCbc] = fReadoutChipInterface->ReadChipReg (cCbc, "Pipe&StubInpSel&Ptwidth");
                    fHIPCountValue[cCbc] = fReadoutChipInterface->ReadChipReg (cCbc, "HIP&TestMode");
                    fReadoutChipInterface->WriteChipReg (cCbc, "Pipe&StubInpSel&Ptwidth", 0x23);
                    fReadoutChipInterface->WriteChipReg (cCbc, "HIP&TestMode", 0x08);
                }

                uint32_t cCbcId = cCbc->getChipId();
                cCbcCount++;

                if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;
                fVplusMap.insert ({cCbc, 0});
                TString cName = Form ( "h_VplusValues_Fe%dCbc%d", cFeId, cCbcId );
                TObject* cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TString cTitle;

                if (fType == FrontEndType::CBC3) cTitle = Form ( "VCth Values for Test Groups FE%d CBC%d; Vth", cFeId, cCbcId );

                TH1I* cHist = new TH1I ( cName, cTitle, 1, 0, 1 );
                cHist->SetMarkerStyle ( 20 );
                // cHist->SetLineWidth( 2 );
                bookHistogram ( cCbc, "Vplus", cHist );

                cName = Form ( "h_Offsets_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TH1I* cOffsetHist = new TH1I ( cName, Form ( "Offsets FE%d CBC%d ; Channel; Offset", cFeId, cCbcId ), 254, -.5, 253.5  );

                bookHistogram ( cCbc, "Offsets", cOffsetHist );

                cName = Form ( "h_Occupancy_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TH1F* cOccHist = new TH1F ( cName, Form ( "Occupancy FE%d CBC%d ; Channel; Occupancy", cFeId, cCbcId ), 254, -.5, 253.5 );
                bookHistogram ( cCbc, "Occupancy", cOccHist );
            }
        }

        fNCbc = cCbcCount;
        fNFe = cFeCount;
    }

    uint32_t cPads = ( cCbcIdMax > cCbcCount ) ? cCbcIdMax : cCbcCount;

    //fVplusCanvas->DivideSquare ( cPads );
    fOffsetCanvas->DivideSquare ( cPads );
    fOccupancyCanvas->DivideSquare ( cPads );


    LOG (INFO) << "Created Object Maps and parsed settings:" ;
    
    if (fType == FrontEndType::CBC3)
    {
        fHoleMode = 0;
        fTargetOffset = 0x80;
        fTargetVcth = 0x0000;
        LOG (INFO) << "	Nevents = " << fEventsPerPoint ;
        LOG (INFO) << "	TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
        LOG (INFO) << "  Target Vcth determined algorithmically for CBC3";
        LOG (INFO) << "  Target Offset fixed to half range (0x80) for CBC3";
    }

}


void Calibration::FindVplus()
{
    LOG (INFO) << BOLDBLUE << "Identifying optimal Vplus for CBC..." << RESET;
    // first, set VCth to the target value for each CBC
    ThresholdVisitor cThresholdVisitor (fReadoutChipInterface, fTargetVcth);
    this->accept (cThresholdVisitor);
    LOG (INFO) << BOLDBLUE << "... after the visitor..." << RESET;

    bool originalAllChannelFlag = this->fAllChan;
    this->SetTestAllChannels(true);
    // std::map<uint16_t, Tool::ModuleOccupancyPerChannelMap> backEndOccupanyPerChannelAtTargetMap;
    // std::map<uint16_t, Tool::ModuleGlobalOccupancyMap> backEndOccupanyAtTargetMap;

    setSameLocalDac("ChannelOffset", fTargetOffset);
    
    // bitWiseScan("VCth", fEventsPerPoint, 0.56, true, backEndOccupanyPerChannelAtTargetMap, backEndOccupanyAtTargetMap);
    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory   theDetectorFactory;
    theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("VCth", fEventsPerPoint, 0.56);

    setSameLocalDac("ChannelOffset", ( fHoleMode ) ? 0x00 : 0xFF);
    
    float cMeanValue = 0.;
    uint32_t nCbc = 0;

    for (auto& cBoard : fBoardVector)
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fReadoutChipVector )
            {

                TH1I* vPlusHist = static_cast<TH1I*> ( getHist ( cCbc, "Vplus" ) );
                uint16_t tmpVthr = (cCbc->getReg("VCth1") + (cCbc->getReg("VCth2")<<8));
                vPlusHist->Fill(0.,tmpVthr);
                LOG (INFO) << GREEN << "VCth value for BeBoard " << +cBoard->getBeId() << " Module " << +cFe->getModuleId() << " CBC " << +cCbc->getChipId() << " = " << tmpVthr << RESET;
                cMeanValue+=tmpVthr;
                ++nCbc;
            }
        }
    }

    this->HttpServerProcess();

    fTargetVcth = static_cast<uint16_t> (cMeanValue / nCbc);
    cThresholdVisitor.setThreshold (fTargetVcth);
    this->accept (cThresholdVisitor);
    LOG (INFO) << BOLDBLUE << "Mean VCth value of all chips is " << fTargetVcth << " - using as TargetVcth value for all chips!" << RESET;
    this->SetTestAllChannels(originalAllChannelFlag);

}


void Calibration::FindOffsets()
{
    LOG (INFO) << BOLDBLUE << "Finding offsets..." << RESET;
    // just to be sure, configure the correct VCth and VPlus values
    ThresholdVisitor cThresholdVisitor (fReadoutChipInterface, fTargetVcth);
    this->accept (cThresholdVisitor);
    // ok, done, all the offsets are at the starting value, VCth & Vplus are written

    // std::map<uint16_t, Tool::ModuleOccupancyPerChannelMap> backEndOccupanyPerChannelAtTargetMap;
    // std::map<uint16_t, Tool::ModuleGlobalOccupancyMap> backEndOccupanyAtTargetMap;

    // bitWiseScan("ChannelOffset", fEventsPerPoint, 0.56, true, backEndOccupanyPerChannelAtTargetMap, backEndOccupanyAtTargetMap);
    

    DetectorDataContainer     theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory   theDetectorFactory;
    theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("ChannelOffset", fEventsPerPoint, 0.56);

    // std::map<uint16_t, ModuleOccupancyPerChannelMap> backEndOccupancyPerChannelMap;
    // std::map<uint16_t, ModuleGlobalOccupancyMap > backEndCbcOccupanyMap;
    // float globalOccupancy=0;
    // setSameLocalDac("ChannelOffset", ( fHoleMode ) ? 0x00 : 0xFF);

    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fReadoutChipVector )
            {
                TH1F* cOccHist = static_cast<TH1F*> ( getHist ( cCbc, "Occupancy" ) );
                TH1I* cOffsetHist = static_cast<TH1I*> ( getHist ( cCbc, "Offsets" ) );
                int cMeanOffset=0;
                for ( int cChannel=0; cChannel<254; ++cChannel)
                {
                    cOccHist->Fill(cChannel, theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cCbc->getChipId())->getChannel<Occupancy>(cChannel).fOccupancy);
                    // cOccHist->Fill(cChannel, backEndOccupanyPerChannelAtTargetMap[cBoard->getBeId()][cFe->getModuleId()][cCbc->getChipId()][cChannel]);
                    std::string cRegName = Form ( "Channel%03d", cChannel + 1 );
                    cOffsetHist->Fill ( cChannel, (uint16_t)cCbc->getReg(cRegName) );
                    cMeanOffset += cCbc->getReg(cRegName);
                }
                LOG (INFO) << BOLDRED << "Mean offset on CBC" << +cCbc->getChipId() << " is : " << (cMeanOffset)/(double)NCHANNELS << " Vcth units." << RESET;
            }
        }
    }

    updateHists ( "Occupancy" );
    updateHists ( "Offsets" );
    
    this->HttpServerProcess();
}


// float Calibration::findCbcOccupancy ( Chip* pCbc, int pTGroup, int pEventsPerPoint )
// {
//     TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
//     float cOccupancy = cOccHist->GetEntries();
//     // return the hitcount divided by the the number of channels and events
//     return cOccupancy / ( static_cast<float> ( fTestGroupChannelMap[pTGroup].size() * pEventsPerPoint ) );
// }

void Calibration::clearOccupancyHists ( Chip* pCbc )
{
    TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
    cOccHist->Reset ( "ICESM" );
}

void Calibration::updateHists ( std::string pHistname )
{
    // loop the CBCs
    for ( const auto& cCbc : fChipHistMap )
    {
        // loop the map of string vs TObject
        auto cHist = cCbc.second.find ( pHistname );

        if ( cHist != std::end ( cCbc.second ) )
        {
            if ( pHistname == "Offsets" )
            {
                fOffsetCanvas->cd ( cCbc.first->getChipId() + 1 );
                TH1F* cTmpHist = static_cast<TH1F*> ( cHist->second );
                cTmpHist->DrawCopy ("hist");
                fOffsetCanvas->Update();
            }

            if ( pHistname == "Occupancy" )
            {
                fOccupancyCanvas->cd ( cCbc.first->getChipId() + 1 );
                TH1F* cTmpProfile = static_cast<TH1F*> ( cHist->second );
                cTmpProfile->DrawCopy ();
                fOccupancyCanvas->Update();
            }
        }
        else LOG (INFO) << "Error, could not find Histogram with name " << pHistname ;
    }

}


void Calibration::writeObjects()
{
    this->SaveResults();
    fResultFile->cd();
    // Save hist maps for CBCs

    //Tool::SaveResults();

    // save canvases too
    //fVplusCanvas->Write ( fVplusCanvas->GetName(), TObject::kOverwrite );
    fOffsetCanvas->Write ( fOffsetCanvas->GetName(), TObject::kOverwrite );
    fOccupancyCanvas->Write ( fOccupancyCanvas->GetName(), TObject::kOverwrite );
    fResultFile->Flush();
}

// State machine control functions

void Calibration::ConfigureCalibration()
{  
    CreateResultDirectory ( "Results/Run_Calibration" );
    InitResultFile ( "CalibrationResults" );
}

void Calibration::Start(int currentRun)
{
    Initialise ( true, true );
    FindVplus();
    FindOffsets();
}

void Calibration::Stop()
{
    writeObjects();
    dumpConfigFiles();

    SaveResults();
    CloseResultFile();
    Destroy();
}

void Calibration::Pause()
{
}

void Calibration::Resume()
{
}

