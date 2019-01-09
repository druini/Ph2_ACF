#include "Calibration.h"

//initialize the static member
//std::map<Cbc*, uint16_t> Calibration::fVplusMap;

Calibration::Calibration() :
    Tool(),
    fVplusMap(),
    fVplusCanvas (nullptr),
    fOffsetCanvas (nullptr),
    fOccupancyCanvas (nullptr),
    fNCbc (0),
    fNFe (0)
{
}

Calibration::~Calibration()
{

}

void Calibration::Initialise ( bool pAllChan, bool pDisableStubLogic )
{
    fDisableStubLogic = pDisableStubLogic;
    // Initialize the TestGroups
    this->MakeTestGroups ( pAllChan );
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

    if ( fTestPulseAmplitude == 0 ) fTestPulse = 0;
    else fTestPulse = 1;

    // Canvases
    //fVplusCanvas = new TCanvas ( "VPlus", "VPlus", 515, 0, 500, 500 );
    fOffsetCanvas = new TCanvas ( "Offset", "Offset", 10, 0, 500, 500 );
    fOccupancyCanvas = new TCanvas ( "Occupancy", "Occupancy", 10, 525, 500, 500 );

    // count FEs & CBCs
    uint32_t cCbcCount = 0;
    uint32_t cCbcIdMax = 0;
    uint32_t cFeCount = 0;


    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();
            cFeCount++;
            fType = cFe->getChipType();

            for ( auto cCbc : cFe->fCbcVector )
            {
                //if it is a CBC3, disable the stub logic for this procedure
                if (cCbc->getChipType() == ChipType::CBC3 && fDisableStubLogic)
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - thus disabling Stub logic for offset tuning" << RESET ;
                    fStubLogicValue[cCbc] = fCbcInterface->ReadCbcReg (cCbc, "Pipe&StubInpSel&Ptwidth");
                    fHIPCountValue[cCbc] = fCbcInterface->ReadCbcReg (cCbc, "HIP&TestMode");
                    fCbcInterface->WriteCbcReg (cCbc, "Pipe&StubInpSel&Ptwidth", 0x23);
                    fCbcInterface->WriteCbcReg (cCbc, "HIP&TestMode", 0x08);
                }

                uint32_t cCbcId = cCbc->getCbcId();
                cCbcCount++;

                if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;

                fVplusMap.insert ({cCbc, 0});

                TString cName = Form ( "h_VplusValues_Fe%dCbc%d", cFeId, cCbcId );
                TObject* cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TString cTitle;

                if (fType == ChipType::CBC2) cTitle = Form ( "Vplus Values for Test Groups FE%d CBC%d ; Test Group; Vplus", cFeId, cCbcId );
                else if (fType == ChipType::CBC3) cTitle = Form ( "VCth Values for Test Groups FE%d CBC%d ; Test Group; Vth", cFeId, cCbcId );

                TProfile* cHist = new TProfile ( cName, cTitle, 9, -1.5, 7.5 );
                cHist->SetMarkerStyle ( 20 );
                // cHist->SetLineWidth( 2 );
                bookHistogram ( cCbc, "Vplus", cHist );

                cName = Form ( "h_Offsets_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TProfile* cOffsetHist = new TProfile ( cName, Form ( "Offsets FE%d CBC%d ; Channel; Offset", cFeId, cCbcId ), 254, -.5, 253.5  );
                uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;

                for ( int iChan = 0; iChan < NCHANNELS; iChan++ )
                {
                    //suggested B. Schneider
                    int iBin = cOffsetHist->FindBin (iChan);
                    cOffsetHist->SetBinContent ( iBin, cOffset );
                    cOffsetHist->SetBinEntries ( iBin, 1 );
                }

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

    if (fType == ChipType::CBC2)
    {
        LOG (INFO) << "	Nevents = " << fEventsPerPoint ;
        LOG (INFO) << "	Hole Mode = " << fHoleMode ;
        LOG (INFO) << "	TargetVcth = " << int ( fTargetVcth ) ;
        LOG (INFO) << "	TargetOffset = " << int ( fTargetOffset ) ;
        LOG (INFO) << "	TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
    }
    else if (fType == ChipType::CBC3)
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
    // first, set VCth to the target value for each CBC
    ThresholdVisitor cThresholdVisitor (fCbcInterface, fTargetVcth);
    this->accept (cThresholdVisitor);


    bool originalAllChannelFlag = this->fAllChan;
    this->SetTestAllChannels(true);
    std::map<uint16_t, Tool::ModuleOccupancyPerChannelMap> backEndOccupanyPerChannelAtTargetMap;
    std::map<uint16_t, Tool::ModuleGlobalOccupancyMap> backEndOccupanyAtTargetMap;

    setSameLocalDac("ChannelOffset", fTargetOffset);
    
    bitWiseScan("VCth", fEventsPerPoint, 0.56, true, backEndOccupanyPerChannelAtTargetMap, backEndOccupanyAtTargetMap);
    
    setSameLocalDac("ChannelOffset", ( fHoleMode ) ? 0x00 : 0xFF);
    
    float cMeanValue = 0.;
    uint32_t nCbc = 0;

    for (auto& cBoard : fBoardVector)
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fCbcVector )
            {
                uint16_t tmpVthr = (cCbc->getReg("VCth1") + (cCbc->getReg("VCth2")<<8));
                LOG (INFO) << GREEN << "VCth value for BeBoard " << cBoard->getBeId() << " Module " << cFe->getModuleId() << " CBC " << cCbc->getCbcId() << " = " << tmpVthr << RESET;
                cMeanValue+=tmpVthr;
                ++nCbc;
            }
        }
    }

    fTargetVcth = static_cast<uint16_t> (cMeanValue / nCbc);
    cThresholdVisitor.setThreshold (fTargetVcth);
    this->accept (cThresholdVisitor);
    LOG (INFO) << BOLDBLUE << "Mean VCth value of all chips is " << fTargetVcth << " - using as TargetVcth value for all chips!" << RESET;
    this->SetTestAllChannels(originalAllChannelFlag);

}


void Calibration::FindOffsets()
{
    
    // just to be sure, configure the correct VCth and VPlus values
    ThresholdVisitor cThresholdVisitor (fCbcInterface, fTargetVcth);
    this->accept (cThresholdVisitor);
    // ok, done, all the offsets are at the starting value, VCth & Vplus are written

    std::map<uint16_t, Tool::ModuleOccupancyPerChannelMap> backEndOccupanyPerChannelAtTargetMap;
    std::map<uint16_t, Tool::ModuleGlobalOccupancyMap> backEndOccupanyAtTargetMap;

    bitWiseScan("ChannelOffset", fEventsPerPoint, 0.56, true, backEndOccupanyPerChannelAtTargetMap, backEndOccupanyAtTargetMap);
    
    // setSameLocalDac("ChannelOffset", ( fHoleMode ) ? 0x00 : 0xFF);

    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fCbcVector )
            {
                TH1F* cOccHist = static_cast<TH1F*> ( getHist ( cCbc, "Occupancy" ) );

                for (int i=0; i<=backEndOccupanyPerChannelAtTargetMap[cBoard->getBeId()][cFe->getModuleId()][cCbc->getCbcId()].size(); ++i)
                {
                    cOccHist->Fill(i, backEndOccupanyPerChannelAtTargetMap[cBoard->getBeId()][cFe->getModuleId()][cCbc->getCbcId()][i]);
                }
            }
        }
    }

}


float Calibration::findCbcOccupancy ( Cbc* pCbc, int pTGroup, int pEventsPerPoint )
{
    TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
    float cOccupancy = cOccHist->GetEntries();
    // return the hitcount divided by the the number of channels and events
    return cOccupancy / ( static_cast<float> ( fTestGroupChannelMap[pTGroup].size() * pEventsPerPoint ) );
}

void Calibration::clearOccupancyHists ( Cbc* pCbc )
{
    TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
    cOccHist->Reset ( "ICESM" );
}

void Calibration::clearVPlusMap()
{
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fCbcVector )
                fVplusMap[cCbc] = 0;
        }
    }
}

void Calibration::updateHists ( std::string pHistname )
{
    // loop the CBCs
    for ( const auto& cCbc : fCbcHistMap )
    {
        // loop the map of string vs TObject
        auto cHist = cCbc.second.find ( pHistname );

        if ( cHist != std::end ( cCbc.second ) )
        {
            // now cHist.second is the Histogram
            //if ( pHistname == "Vplus" )
            //{
            //fVplusCanvas->cd ( cCbc.first->getCbcId() + 1 );
            //TProfile* cTmpProfile = static_cast<TProfile*> ( cHist->second );
            //cTmpProfile->DrawCopy ( "H P0 E" );
            //fVplusCanvas->Update();
            //}

            if ( pHistname == "Offsets" )
            {
                fOffsetCanvas->cd ( cCbc.first->getCbcId() + 1 );
                TH1F* cTmpHist = static_cast<TH1F*> ( cHist->second );
                cTmpHist->DrawCopy ("hist");
                fOffsetCanvas->Update();
            }

            if ( pHistname == "Occupancy" )
            {
                fOccupancyCanvas->cd ( cCbc.first->getCbcId() + 1 );
                TProfile* cTmpProfile = static_cast<TProfile*> ( cHist->second );
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
