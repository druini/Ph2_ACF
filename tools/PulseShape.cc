#include "PulseShape.h"


void PulseShape::Initialize()
{
    fNCbc = 0;

    std::cerr << "void PulseShape::Initialize()"  ;

    for ( auto& cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();
        std::cerr << "cBoardId = " << cBoardId ;
        fDelayAfterPulse = fBeBoardInterface->ReadBoardReg (cBoard, getDelAfterTPString ( cBoard->getBoardType() ) );

        for ( auto& cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();
            std::cerr << "cFeId = " << cFeId ;

            for ( auto& cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();
                std::cerr << "cCbcId = " << cCbcId ;
                fNCbc++;
                // Create the Canvas to draw
                TCanvas* ctmpCanvas = new TCanvas ( Form ( "c_online_canvas_fe%dcbc%d", cFeId, cCbcId ), Form ( "FE%dCBC%d  Online Canvas", cFeId, cCbcId ) );
                ctmpCanvas->Divide ( 2, 1 );
                fCanvasMap[cCbc] = ctmpCanvas;

                //should set the canvas frames sane!
                int cLow = ( fDelayAfterPulse - 1 ) * 25;
                int cHigh = ( fDelayAfterPulse + 8 ) * 25;
                TH2I* cFrame = new TH2I ( "cFrame", "PulseShape; Delay [ns]; Amplitude [VCth]", 350, cLow, cHigh, 255, 0, 255 );
                cFrame->SetStats ( false );
                ctmpCanvas->cd ( 2 );
                cFrame->Draw( );
                bookHistogram ( cCbc, "frame", cFrame );
                std::cerr << "Initializing map fCanvasMap[" << Form ( "0x%x", cCbc ) << "] = " << Form ( "0x%x", ctmpCanvas ) ;
                // Create Multigraph Object for each CBC
                TString cName =  Form ( "g_cbc_pulseshape_MultiGraph_Fe%dCbc%d", cFeId, cCbcId );
                TObject* cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TMultiGraph* cMultiGraph = new TMultiGraph();
                cMultiGraph->SetName ( cName );
                bookHistogram ( cCbc, "cbc_pulseshape", cMultiGraph );
                cName = Form ( "f_cbc_pulse_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;
            }

        }
    }

    parseSettings();
    LOG (INFO) << "Histograms and Settings initialised." ;
}

void PulseShape::ScanTestPulseDelay ( uint8_t pStepSize )
{


    // setSystemTestPulse(fTPAmplitude, fChannelId);
    // enableChannel(fChannelId);
    setSystemTestPulse ( fTPAmplitude/*, 0 */ );
    toggleTestGroup (true);
    // initialize the historgram for the channel map
    //should set the histogram boardes frames sane (from config file)!
    int cCoarseDefault = fDelayAfterPulse;
    int cLow = ( cCoarseDefault - 1 ) * 25;
    int cHigh = ( cCoarseDefault + 8 ) * 25;

    for ( uint32_t cTestPulseDelay = cLow ; cTestPulseDelay < cHigh; cTestPulseDelay += fStepSize )
    {
        setDelayAndTesGroup ( cTestPulseDelay );
        ScanVcth ( cTestPulseDelay );
    }

    this->fitGraph ( cLow );
    updateHists ( "cbc_pulseshape", true );
    toggleTestGroup (false);

}

void PulseShape::ScanVcth ( uint32_t pDelay )
{
    for ( auto& cChannelVector : fChannelMap )
        for ( auto& cChannel : cChannelVector.second )
            cChannel->initializeHist ( pDelay, "Delay" );


    uint8_t cVcth = ( fHoleMode ) ?  0xFF :  0x00;
    int cStep = ( fHoleMode ) ? -10 : +10;
    uint32_t cAllOneCounter = 0;
    bool cAllOne = false;
    bool cNonZero = false;
    bool cSaturate = false;
    uint8_t cDoubleVcth;

    // Adaptive VCth loop
    while ( 0x00 <= cVcth && cVcth <= 0xFF )
    {
        if ( cAllOne ) break;

        if ( cVcth == cDoubleVcth )
        {
            cVcth +=  cStep;
            continue;
        }

        // if ( cAllOne ) break;
        //CbcRegWriter cWriter ( fCbcInterface, "VCth", cVcth );
        //this->accept ( cWriter );

        // then we take fNEvents
        uint32_t cN = 1;
        uint32_t cNthAcq = 0;
        int cNHits = 0;

        // Take Data for all Modules
        for ( BeBoard* pBoard : fBoardVector )
        {
            for (Module* cFe : pBoard->fModuleVector)
                fCbcInterface->WriteBroadcast (cFe, "VCth", cVcth);

            //fBeBoardInterface->Start( pBoard );
            //while ( cN <= fNevents )
            //{
            fBeBoardInterface->ReadNEvents ( pBoard, fNevents );
            const std::vector<Event*>& events = fBeBoardInterface->GetEvents ( pBoard );

            for ( auto& cEvent : events )
                cNHits += fillVcthHist ( pBoard, cEvent, cVcth );

            cNthAcq++;

            //}
            //fBeBoardInterface->Stop( pBoard );
            if ( !cNonZero && cNHits != 0 )
            {
                cNonZero = true;
                cDoubleVcth = cVcth;
                int cBackStep = 2 * cStep;

                if ( int ( cVcth ) - cBackStep > 255 ) cVcth = 255;
                else if ( int ( cVcth ) - cBackStep < 0 ) cVcth = 0;
                else cVcth -= cBackStep;

                cStep /= 10;
                continue;
            }

            if ( cNHits > 0.95 * fNCbc * fNevents * findChannelsInTestGroup ( fTestGroup ).size() )
                cAllOneCounter++;

            if ( cAllOneCounter > 6 ) cAllOne = true;

            if ( cAllOne )

                break;

            cVcth += cStep;
            updateHists ( "", false );

            if ( fHoleMode && cVcth >= 0xFE && cNHits != 0 )
            {
                cSaturate = true;
                break;
            }

            if ( !fHoleMode && cVcth <= 0x01 && cNHits != 0 )
            {
                cSaturate = true;
                break;
            }
        }
    }

    for ( auto& cChannelVector : fChannelMap )
    {
        for ( auto& cChannel : cChannelVector.second )
        {
            if ( fFitHist ) cChannel->fitHist ( fNevents, fHoleMode, pDelay, "Delay", fResultFile );
            else cChannel->differentiateHist ( fNevents, fHoleMode, pDelay, "Delay", fResultFile );

            if ( !cSaturate ) cChannel->setPulsePoint ( pDelay, cChannel->getPedestal() );
            else cChannel->setPulsePoint ( pDelay, 255 );
        }

    }

    updateHists ( "", true );
    updateHists ( "cbc_pulseshape", false );
}




//////////////////////////////////////      PRIVATE METHODS     /////////////////////////////////////////////


void PulseShape::fitGraph ( int pLow )
{
    for ( auto& cCbc : fChannelMap )
    {
        for ( auto& cChannel : cCbc.second )
        {
            TString cName = Form ( "f_cbc_pulse_Fe%dCbc%d_Channel%d", cCbc.first->getFeId(), cCbc.first->getCbcId(), cChannel->fChannelId );
            TObject* cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            //TF1* cPulseFit = new TF1( cName, pulseshape, ( fDelayAfterPulse - 1 ) * 25, ( fDelayAfterPulse + 6 ) * 25, 6 );
            TF1* cPulseFit = new TF1 ( cName, pulseshape, 5000, 5300, 6 );

            cPulseFit->SetParNames ( "Amplitude", "t_0", "tau", "Amplitude offset", "Rel. negative pulse amplitude", "Delta t" );
            //"scale_par"
            cPulseFit->SetParLimits ( 0, 160, 2000 );
            cPulseFit->SetParameter ( 0, 250 );
            //"offset"
            cPulseFit->SetParLimits ( 1, 5000, 5300 );
            cPulseFit->SetParameter ( 1, 5025 );
            //"time_constant"
            cPulseFit->SetParLimits ( 2, 25, 75 );
            cPulseFit->SetParameter ( 2, 50 );
            //"y_offset"
            cPulseFit->SetParLimits ( 3, 0, 200 );
            cPulseFit->SetParameter ( 3, 50 );
            //"Relative amplitude of negative pulse"
            cPulseFit->SetParameter ( 4, 0.5 );
            cPulseFit->SetParLimits ( 4, 0, 1 );
            //delta t
            cPulseFit->FixParameter ( 5, 50 );

            cChannel->fPulse->Fit ( cPulseFit, "R+S" );
            TString cDirName = "PulseshapeFits";
            TDirectory* cDir = dynamic_cast< TDirectory* > ( gROOT->FindObject ( cDirName ) );

            if ( !cDir ) cDir = fResultFile->mkdir ( cDirName );

            fResultFile->cd ( cDirName );
            cChannel->fPulse->Write ( cChannel->fPulse->GetName(), TObject::kOverwrite );
            cPulseFit->Write ( cPulseFit->GetName(), TObject::kOverwrite );
            fResultFile->cd();
        }

    }
}




std::vector<uint32_t> PulseShape::findChannelsInTestGroup ( uint32_t pTestGroup )
{
    std::vector<uint32_t> cChannelVector;

    for ( int idx = 0; idx < 16; idx++ )
    {
        int ctemp1 = idx * 16  + pTestGroup * 2 + 1 ;
        int ctemp2 = ctemp1 + 1;

        if ( ctemp1 < 254 ) cChannelVector.push_back ( ctemp1 );

        if ( ctemp2 < 254 )  cChannelVector.push_back ( ctemp2 );
    }

    return cChannelVector;
}

void PulseShape::toggleTestGroup (bool pEnable )
{
    std::vector<std::pair<std::string, uint8_t> > cRegVec;
    uint8_t cDisableValue = fHoleMode ? 0x00 : 0xFF;
    uint8_t cValue = pEnable ? fOffset : cDisableValue;

    for ( auto& cChannel : fChannelVector )
    {
        TString cRegName = Form ( "Channel%03d", cChannel );
        cRegVec.push_back ( std::make_pair ( cRegName.Data(), cValue ) );
    }

    //CbcMultiRegWriter cWriter ( fCbcInterface, cRegVec );
    //this->accept ( cWriter );
    for (BeBoard* cBoard : fBoardVector)
    {
        for (Module* cFe : cBoard->fModuleVector)
            fCbcInterface->WriteBroadcastMultReg (cFe, cRegVec);
    }
}

void PulseShape::setDelayAndTesGroup ( uint32_t pDelay )
{
    uint8_t cCoarseDelay = floor ( pDelay  / 25 );
    uint8_t cFineDelay = ( cCoarseDelay * 25 ) + 24 - pDelay;

    LOG (INFO) << "cFineDelay: " << +cFineDelay ;
    LOG (INFO) << "cCoarseDelay: " << +cCoarseDelay ;
    LOG (INFO) << "Current Time: " << +pDelay ;

    for (auto& cBoard : fBoardVector)
    {
        //potentially have to reset the IC FW commissioning cycle state machine?
        fBeBoardInterface->WriteBoardReg (cBoard, getDelAfterTPString (cBoard->getBoardType() ) , cCoarseDelay);
    }

    CbcRegWriter cWriter ( fCbcInterface, "SelTestPulseDel&ChanGroup", to_reg ( cFineDelay, fTestGroup ) );
    this->accept ( cWriter );

}

uint32_t PulseShape::fillVcthHist ( BeBoard* pBoard, Event* pEvent, uint32_t pVcth )
{
    uint32_t cHits = 0;

    // Loop over Events from this Acquisition
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cCbc : cFe->fCbcVector )
        {
            //  get histogram to fill
            auto cChannelVector = fChannelMap.find ( cCbc );

            if ( cChannelVector == std::end ( fChannelMap ) ) LOG (INFO) << "Error, no channel vector mapped to this CBC ( " << +cCbc->getCbcId() << " )" ;
            else
            {
                for ( auto& cChannel : cChannelVector->second )
                {
                    if ( pEvent->DataBit ( cFe->getFeId(), cCbc->getCbcId(), cChannel->fChannelId - 1 ) )
                    {
                        cChannel->fillHist ( pVcth );
                        cHits++;
                    }
                }
            }
        }

        return cHits;
    }
}

void PulseShape::parseSettings()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;

    cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )  fHoleMode = cSetting->second;
    else fHoleMode = 1;

    cSetting = fSettingsMap.find ( "Vplus" );

    if ( cSetting != std::end ( fSettingsMap ) )  fVplus = cSetting->second;
    else fVplus = 0x6F;

    cSetting = fSettingsMap.find ( "TPAmplitude" );

    if ( cSetting != std::end ( fSettingsMap ) ) fTPAmplitude = cSetting->second;
    else fTPAmplitude = 0x78;

    cSetting = fSettingsMap.find ( "ChannelOffset" );

    if ( cSetting != std::end ( fSettingsMap ) ) fOffset = cSetting->second;
    else fOffset = 0x05;

    cSetting = fSettingsMap.find ( "TestGroup" );

    if ( cSetting != std::end ( fSettingsMap ) ) fTestGroup = cSetting->second;
    else fTestGroup = 1;

    cSetting = fSettingsMap.find ( "StepSize" );

    if ( cSetting != std::end ( fSettingsMap ) ) fStepSize = cSetting->second;
    else fStepSize = 5;

    cSetting = fSettingsMap.find ( "FitSCurves" );
    fFitHist = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;

    LOG (INFO) << "Parsed the following settings:" ;
    LOG (INFO) << "	Nevents = " << fNevents ;
    LOG (INFO) << "	HoleMode = " << int ( fHoleMode ) ;
    LOG (INFO) << "	Vplus = " << int ( fVplus ) ;
    LOG (INFO) << "	TPAmplitude = " << int ( fTPAmplitude ) ;
    LOG (INFO) << "	ChOffset = " << int ( fOffset ) ;
    LOG (INFO) << "	StepSize = " << int ( fStepSize ) ;
    LOG (INFO) << "	FitSCurves = " << int ( fFitHist ) ;
    LOG (INFO) << "	TestGroup = " << int ( fTestGroup ) ;
}

void PulseShape::setSystemTestPulse ( uint8_t pTPAmplitude )
{

    std::vector<std::pair<std::string, uint8_t>> cRegVec;
    fChannelVector = findChannelsInTestGroup ( fTestGroup );
    uint8_t cRegValue =  to_reg ( 0, fTestGroup );
    cRegVec.push_back ( std::make_pair ( "SelTestPulseDel&ChanGroup",  cRegValue ) );

    //set the value of test pulsepot registrer and MiscTestPulseCtrl&AnalogMux register
    if ( fHoleMode )
        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0xD1 ) );
    else
        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0x61 ) );

    cRegVec.push_back ( std::make_pair ( "TestPulsePot", pTPAmplitude ) );
    cRegVec.push_back ( std::make_pair ( "Vplus",  fVplus ) );

    CbcMultiRegWriter cWriter ( fCbcInterface, cRegVec );
    this->accept ( cWriter );

    for ( auto& cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto& cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto& cCbc : cFe->fCbcVector )
            {
                std::vector<Channel*> cChannelVector;
                uint32_t cCbcId = cCbc->getCbcId();
                int cMakerColor = 1;

                for ( auto& cChannelId : fChannelVector )
                {
                    Channel* cChannel = new Channel ( cBoardId, cFeId, cCbcId, cChannelId );
                    TString cName =  Form ( "g_cbc_pulseshape_Fe%dCbc%d_Channel%d", cFeId, cCbcId, cChannelId );
                    cChannel->initializePulse ( cName );
                    cChannelVector.push_back ( cChannel );
                    cChannel->fPulse->SetMarkerColor ( cMakerColor );
                    cMakerColor++;
                    TMultiGraph* cTmpGraph = static_cast<TMultiGraph*> ( getHist ( cCbc, "cbc_pulseshape" ) );
                    cTmpGraph->Add ( cChannel->fPulse, "lp" );
                }

                fChannelMap[cCbc] = cChannelVector;
            }
        }
    }

}

void PulseShape::updateHists ( std::string pHistName, bool pFinal )
{
    for ( auto& cCanvas : fCanvasMap )
    {
        cCanvas.second->cd();

        if ( pHistName == "" )
        {
            // now iterate over the channels in the channel map and draw
            auto cChannelVector = fChannelMap.find ( static_cast<Ph2_HwDescription::Cbc*> ( cCanvas.first ) );

            if ( cChannelVector == std::end ( fChannelMap ) ) LOG (INFO) << "Error, no channel mapped to this CBC ( " << +cCanvas.first << " )" ;
            else
            {
                TString cOption = "P";

                for ( auto& cChannel : cChannelVector->second )
                {
                    cCanvas.second->cd ( 1 );
                    cChannel->fScurve->Draw ( cOption );
                    cOption = "p same";
                }
            }
        }

        if ( pHistName == "" && pFinal )
        {
            // now iterate over the channels in the channel map and draw
            auto cChannelVector = fChannelMap.find ( static_cast<Ph2_HwDescription::Cbc*> ( cCanvas.first ) );

            if ( cChannelVector == std::end ( fChannelMap ) ) LOG (INFO) << "Error, no channel mapped to this CBC ( " << +cCanvas.first << " )" ;
            else
            {
                TString cOption = "P";

                for ( auto& cChannel : cChannelVector->second )
                {
                    cCanvas.second->cd ( 1 );
                    cChannel->fScurve->Draw ( cOption );
                    cOption = "p same";

                    if ( fFitHist ) cChannel->fFit->Draw ( "same" );
                    else cChannel->fDerivative->Draw ( "same" );
                }
            }
        }
        else if ( pHistName == "cbc_pulseshape" )
        {
            auto cChannelVector = fChannelMap.find ( static_cast<Ph2_HwDescription::Cbc*> ( cCanvas.first ) );

            if ( cChannelVector == std::end ( fChannelMap ) ) LOG (INFO) << "Error, no channel mapped to this CBC ( " << +cCanvas.first << " )" ;
            else
            {
                TH2I* cTmpFrame = static_cast<TH2I*> ( getHist ( static_cast<Ph2_HwDescription::Cbc*> ( cCanvas.first ), "frame" ) );
                cCanvas.second->cd ( 2 );
                cTmpFrame->Draw( );
                TString cOption = "P same";

                for ( auto& cChannel : cChannelVector->second )
                {
                    cCanvas.second->cd ( 2 );
                    cChannel->fPulse->Draw ( cOption );
                    cOption = "P same";
                }

            }
        }
        else if ( pHistName == "cbc_pulseshape" && pFinal )
        {
            auto cChannelVector = fChannelMap.find ( static_cast<Ph2_HwDescription::Cbc*> ( cCanvas.first ) );

            if ( cChannelVector == std::end ( fChannelMap ) ) LOG (INFO) << "Error, no channel mapped to this CBC ( " << +cCanvas.first << " )" ;
            else
            {
                cCanvas.second->cd ( 2 );
                TMultiGraph* cMultiGraph = static_cast<TMultiGraph*> ( getHist ( static_cast<Ph2_HwDescription::Cbc*> ( cCanvas.first ), "cbc_pulseshape" ) );
                cMultiGraph->Draw ( "A" );
                cCanvas.second->Modified();
            }
        }

        cCanvas.second->Update();
    }

    //#ifdef __HTTP__
    //fHttpServer->ProcessRequests();
    //#endif
}

double pulseshape ( double* x, double* par )
{
    double xx = x[0];
    double val = par[3];

    if ( xx > par[1] )
        val += par[0] * ( xx - par[1] ) / par[2] * exp ( - ( ( xx - par[1] ) / par[2] ) );

    if ( xx > par[1] + par[5] )
        val -= par[0] * par[4] * ( xx - par[1] - par[5] ) / par[2]  * exp ( - ( ( xx - par[1] - par[5] ) / par[2] ) );

    return val;
}
