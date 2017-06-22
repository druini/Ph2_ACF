#include "PedeNoise.h"


void PedeNoise::Initialise()
{
    //is to be called after system controller::ReadHW, ReadSettings
    // populates all the maps
    // create the canvases


    fPedestalCanvas = new TCanvas ( "Pedestal & Noise", "Pedestal & Noise", 670, 0, 650, 650 );
    //fFeSummaryCanvas = new TCanvas ( "Noise for each FE", "Noise for each FE", 0, 670, 650, 650 );
    fNoiseCanvas = new TCanvas ( "Final SCurves, Strip Noise", "Final SCurves, Noise", 0, 0, 650, 650 );


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
                uint32_t cCbcId = cCbc->getCbcId();
                cCbcCount++;

                if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;

                // populate the channel vector
                std::vector<Channel> cChanVec;

                for ( uint8_t cChan = 0; cChan < NCHANNELS; cChan++ )
                    cChanVec.push_back ( Channel ( cBoardId, cFeId, cCbcId, cChan ) );

                fCbcChannelMap[cCbc] = cChanVec;

                // the fits are initialized when I fit!

                TString cHistname;
                TH1F* cHist;

                // for noise maps etc.

                cHistname = Form ( "Fe%dCBC%d_Offsets", cFe->getFeId(), cCbc->getCbcId() );
                cHist = new TH1F ( cHistname, cHistname, NCHANNELS, -0.5, 253.5 );
                bookHistogram ( cCbc, "Cbc_Offsets", cHist );

                cHistname = Form ( "Fe%dCBC%d_Noise", cFe->getFeId(), cCbc->getCbcId() );
                cHist = new TH1F ( cHistname, cHistname, 200, 0, 20 );
                bookHistogram ( cCbc, "Cbc_Noise", cHist );

                cHistname = Form ( "Fe%dCBC%d_StripNoise", cFe->getFeId(), cCbc->getCbcId() );
                cHist = new TH1F ( cHistname, cHistname, NCHANNELS, -0.5, 253.5 );
                cHist->SetMaximum (10);
                cHist->SetMinimum (0);
                bookHistogram ( cCbc, "Cbc_Stripnoise", cHist );

                cHistname = Form ( "Fe%dCBC%d_Pedestal", cFe->getFeId(), cCbc->getCbcId() );
                cHist = new TH1F ( cHistname, cHistname, 2048, -0.5, 1023.5 );
                bookHistogram ( cCbc, "Cbc_Pedestal", cHist );

                cHistname = Form ( "Fe%dCBC%d_Noise_even", cFe->getFeId(), cCbc->getCbcId() );
                cHist = new TH1F ( cHistname, cHistname, NCHANNELS / 2, -0.5, 126.5 );
                cHist->SetMaximum (10);
                cHist->SetMinimum (0);
                bookHistogram ( cCbc, "Cbc_Noise_even", cHist );

                cHistname = Form ( "Fe%dCBC%d_Noise_odd", cFe->getFeId(), cCbc->getCbcId() );
                cHist = new TH1F ( cHistname, cHistname, NCHANNELS / 2, -0.5, 126.5 );
                cHist->SetLineColor ( 2 );
                cHist->SetMaximum (10);
                cHist->SetMinimum (0);
                bookHistogram ( cCbc, "Cbc_noise_odd", cHist );

                cHistname = Form ( "Fe%dCBC%d_Occupancy", cFe->getFeId(), cCbc->getCbcId() );
                cHist = new TH1F ( cHistname, cHistname, NCHANNELS, -0.5, 253.5 );
                cHist->SetLineColor ( 31 );
                cHist->SetMaximum (1);
                cHist->SetMinimum (0);
                bookHistogram ( cCbc, "Cbc_occupancy", cHist );

            }

            TString cNoisehistname =  Form ( "Fe%d_Noise", cFeId );
            TH1F* cNoise = new TH1F ( cNoisehistname, cNoisehistname, 200, 0, 20 );
            bookHistogram ( cFe, "Module_noisehist", cNoise );

            cNoisehistname = Form ( "Fe%d_StripNoise", cFeId );
            TProfile* cStripnoise = new TProfile ( cNoisehistname, cNoisehistname, ( NCHANNELS * cCbcCount ), -0.5, cCbcCount * NCHANNELS - .5 );
            cStripnoise->SetMinimum (0);
            cStripnoise->SetMaximum (15);
            bookHistogram ( cFe, "Module_Stripnoise", cStripnoise );
        }

        fNCbc = cCbcCount;
        fNFe = cFeCount;
    }

    uint32_t cPads = ( cCbcIdMax > cCbcCount ) ? cCbcIdMax : cCbcCount;



    fNoiseCanvas->DivideSquare ( 2 * cPads );
    fPedestalCanvas->DivideSquare ( 2 * cPads );
    //fFeSummaryCanvas->DivideSquare ( fNFe );

    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    cSetting = fSettingsMap.find ( "FitSCurves" );
    fFitted = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    //cSetting = fSettingsMap.find ( "TestPulseAmplitude" );
    //fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;


    LOG (INFO) << "Created Object Maps and parsed settings:" ;
    LOG (INFO) << "	Nevents = " << fEventsPerPoint ;
    LOG (INFO) << "	FitSCurves = " << int ( fFitted ) ;

    if (fType == ChipType::CBC3)
        LOG (INFO) << BOLDBLUE << "Chip Type determined to be " << BOLDRED << "CBC3" << RESET;
    else
        LOG (INFO) << BOLDBLUE << "Chip Type determined to be " << BOLDRED << "CBC2" << RESET;

}

void PedeNoise::sweepSCurves (uint8_t pTPAmplitude)
{
    uint16_t cStartValue = 0;

    if (pTPAmplitude != 0)
    {
        fTestPulseAmplitude = pTPAmplitude;
        fTestPulse = true;
        //if the test pulse is enabled, I actually want every group to measure it's own midpoint thus I leave the start value at 0 which will make the measureSCurve method check
    }
    else
    {
        fTestPulseAmplitude = pTPAmplitude;
        fTestPulse = false;
        //determine the average midpoint globally and then measure the individual groups precisely
        //this also yields if it's hole mode or not
        cStartValue = this->findPedestal (-1);
    }

    saveInitialOffsets();

    // method to measure one final set of SCurves with the final calibration applied to extract the noise
    // now measure some SCurves
    for ( auto& cTGrpM : fTestGroupChannelMap )
    {
        if (cTGrpM.first != -1)
        {
            // if we want to run with test pulses, we'll have to enable commissioning mode and enable the TP for each test group
            if (fTestPulse)
            {
                LOG (INFO) << RED <<  "Enabling Test Pulse for Test Group " << cTGrpM.first << " with amplitude " << +pTPAmplitude << RESET ;
                setFWTestPulse();
                setSystemTestPulse ( fTestPulseAmplitude, cTGrpM.first, true );
            }

            LOG (INFO) << GREEN << "Measuring Test Group...." << cTGrpM.first << RESET ;
            // this leaves the offset values at the tuned values for cTGrp and disables all other groups
            enableTestGroupforNoise ( cTGrpM.first );

            // now initialize the Scurves
            initializeSCurves ( "SCurves", fTestPulseAmplitude, cTGrpM.first );

            // measure the SCurves, the false is indicating that I am sweeping Vcth
            measureSCurves ( cTGrpM.first, cStartValue );

            // now process the measured SCuvers, true indicates that I am drawing, the TGraphErrors with Vcth vs Vplus are also filled
            bool cFillSummary = (fTestPulse) ? false : true;
            processSCurvesNoise ( "SCurves", fTestPulseAmplitude, true, cFillSummary, cTGrpM.first );

            if (fTestPulse)
            {
                LOG (INFO) << RED <<  "Disabling Test Pulse for Test Group " << cTGrpM.first << RESET ;
                setSystemTestPulse ( 0, cTGrpM.first, false );

            }
        }
    }

    LOG (INFO) << BOLDBLUE << "Finished measuring the noise ..."  << RESET ;
    //dont forget to apply the original offsets
    setInitialOffsets();
}

void PedeNoise::measureNoise (uint8_t pTPAmplitude)
{
    this->sweepSCurves (pTPAmplitude);

    // now plot the histogram with the noise

    // instead of looping over the Histograms and finding everything according to the CBC from the map, just loop the CBCs

    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            // here get the per FE histograms
            TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( cFe, "Module_noisehist" ) );
            TProfile* cTmpProfile = dynamic_cast<TProfile*> ( getHist ( cFe, "Module_Stripnoise" ) );

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = static_cast<int> ( cCbc->getCbcId() );

                // here get the per-CBC histograms

                TH1F* cNoiseHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Noise" ) );
                TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Pedestal" ) );
                TH1F* cStripHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Stripnoise" ) );
                TH1F* cEvenHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Noise_even" ) );
                TH1F* cOddHist   = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_noise_odd" ) );

                LOG (INFO) << BOLDRED << "Average noise on FE " << +cCbc->getFeId() << " CBC " << +cCbc->getCbcId() << " : " << cNoiseHist->GetMean() << " ; RMS : " << cNoiseHist->GetRMS() << " ; Pedestal : " << cPedeHist->GetMean() << " VCth units." << RESET ;

                fNoiseCanvas->cd ( fNCbc + cCbc->getCbcId() + 1 );
                //cStripHist->DrawCopy();
                cEvenHist->DrawCopy();
                cOddHist->DrawCopy ( "same" );

                fPedestalCanvas->cd ( cCbc->getCbcId() + 1 );
                cNoiseHist->DrawCopy();

                fPedestalCanvas->cd ( fNCbc + cCbc->getCbcId() + 1 );
                cPedeHist->DrawCopy();
                fNoiseCanvas->Update();
                fPedestalCanvas->Update();
                // here add the CBC histos to the module histos
                cTmpHist->Add ( cNoiseHist );

                for ( int cChannel = 0; cChannel < NCHANNELS; cChannel++ )
                {
                    //edit suggested by B. Schneider
                    int iBin = cStripHist->FindBin (cChannel);

                    if ( cStripHist->GetBinContent ( iBin ) > 0 && cStripHist->GetBinContent ( iBin ) < 255 ) cTmpProfile->Fill ( cCbcId * 254 + cChannel, cStripHist->GetBinContent ( iBin ) );

                }
            }

            //now apply

            //fFeSummaryCanvas->cd ( cFeId + 1 );
            //cTmpHist->DrawCopy();
            //fFeSummaryCanvas->cd ( fNFe + cFeId + 1 );
            //cTmpProfile->DrawCopy();
            //fFeSummaryCanvas->Update();
        }
    }
}


void PedeNoise::Validate ( uint32_t pNoiseStripThreshold, uint32_t pMultiple )
{
    LOG (INFO) << "Validation: Taking Data with " << fEventsPerPoint* pMultiple << " random triggers!" ;

    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        //increase threshold to supress noise
        setThresholdtoNSigma (cBoard, 5);

        //take data
        for (uint32_t cAcq = 0; cAcq < pMultiple; cAcq++)
        {
            ReadNEvents (cBoard, fEventsPerPoint );

            //analyze
            const std::vector<Event*>& events = GetEvents ( cBoard );

            fillOccupancyHist (cBoard, events);
        }

        //now I've filled the histogram with the occupancy
        //let's say if there is more than 1% noise occupancy, we consider the strip as noise and thus set the offset to either 0 or FF
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fCbcVector )
            {
                //get the histogram for the occupancy
                TH1F* cHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_occupancy" ) );
                cHist->Scale (1 / (fEventsPerPoint * 100.) );
                TLine* line = new TLine (0, pNoiseStripThreshold * 0.001, NCHANNELS, pNoiseStripThreshold * 0.001);

                //as we are at it, draw the plot
                fNoiseCanvas->cd ( cCbc->getCbcId() + 1 );
                gPad->SetLogy (1);
                cHist->DrawCopy();
                line->Draw ("same");
                fNoiseCanvas->Modified();
                fNoiseCanvas->Update();
                RegisterVector cRegVec;

                for (uint32_t iChan = 0; iChan < NCHANNELS; iChan++)
                {
                    // suggested B. Schneider
                    int iBin = cHist->FindBin (iChan);

                    if (cHist->GetBinContent (iBin) > double ( pNoiseStripThreshold * 0.001 ) ) // consider it noisy
                    {
                        TString cRegName = Form ( "Channel%03d", iChan + 1 );
                        uint8_t cValue = fHoleMode ? 0x00 : 0xFF;
                        cRegVec.push_back ({cRegName.Data(), cValue });
                        LOG (INFO) << RED << "Found a noisy channel on CBC " << +cCbc->getCbcId() << " Channel " << iChan  << " with an occupancy of " << cHist->GetBinContent (iChan) << "; setting offset to " << +cValue << RESET ;
                    }

                }

                //Write the changes
                fCbcInterface->WriteCbcMultReg (cCbc, cRegVec);
            }
        }

        setThresholdtoNSigma (cBoard, 0);
    }

    for ( auto& cCbc : fCbcChannelMap )
    {
        bool cFirst = true;
        TString cOption;

        for (auto& cChan : cCbc.second)
        {

            if ( cFirst )
            {
                cOption = "C" ;
                cFirst = false;
            }
            else cOption = "C same";

            fNoiseCanvas->cd ( cCbc.first->getCbcId() + 1 );
            gPad->SetLogy (0);
            cChan.fScurve->DrawCopy ( cOption );
        }

    }

    fNoiseCanvas->Update();
}

double PedeNoise::getPedestal (Cbc* pCbc)
{
    TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( pCbc, "Cbc_Pedestal" ) );
    LOG (INFO) << "Pedestal on CBC " << +pCbc->getCbcId() << " is " << cPedeHist->GetMean() << " VCth units.";
    return cPedeHist->GetMean();
}
double PedeNoise::getPedestal (Module* pFe)
{
    double cPedestal = 0;

    for (auto cCbc : pFe->fCbcVector)
    {
        TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Pedestal" ) );
        cPedestal += cPedeHist->GetMean();
        LOG (INFO) << "Pedestal on CBC " << +cCbc->getCbcId() << " is " << cPedeHist->GetMean() << " VCth units.";
    }

    cPedestal /= pFe->fCbcVector.size();

    LOG (INFO) << "Pedestal on Module " << +pFe->getFeId() << " is " << cPedestal << " VCth units.";
    return cPedestal;
}

double PedeNoise::getNoise (Cbc* pCbc)
{
    TH1F* cNoiseHist = dynamic_cast<TH1F*> ( getHist ( pCbc, "Cbc_Noise" ) );
    return cNoiseHist->GetMean();
}
double PedeNoise::getNoise (Module* pFe)
{
    TH1F* cNoiseHist = dynamic_cast<TH1F*> (getHist (pFe, "Module_noisehist") );
    return cNoiseHist->GetMean();
}

//////////////////////////////////////      PRIVATE METHODS     /////////////////////////////////////////////


void PedeNoise::enableTestGroupforNoise ( int  pTGrpId )
{
    uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;

    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                TH1F* cOffsets = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Offsets" ) );

                RegisterVector cRegVec;

                // iterate the groups (first is ID, second is vec<uint8_t>)
                for ( auto& cGrp : fTestGroupChannelMap )
                {
                    // if grpid = -1, do nothing (all channels)
                    if ( cGrp.first == -1 ) continue;

                    // if the group is not my current grout
                    if ( cGrp.first != pTGrpId )
                    {
                        // iterate the channels and push back 0 or FF
                        for ( auto& cChan : cGrp.second )
                        {
                            TString cRegName = Form ( "Channel%03d", cChan + 1 );
                            cRegVec.push_back ( { cRegName.Data(), cOffset } );
                        }
                    }
                    // if it is the current group, get the original offset values
                    else if ( cGrp.first == pTGrpId )
                    {
                        // iterate over the channels in the test group and find the corresponding offset in the original offset map
                        for ( auto& cChan : cGrp.second )
                        {
                            //suggested B. Schneider
                            int iBin = cOffsets->FindBin (cChan);
                            uint8_t cEnableOffset = cOffsets->GetBinContent ( iBin );
                            TString cRegName = Form ( "Channel%03d", cChan + 1 );
                            cRegVec.push_back ( { cRegName.Data(), cEnableOffset } );
                        }
                    }
                }

                // now I should have 0 or FF as offset for all channels except the one in my test group
                // this now needs to be written to the CBCs
                fCbcInterface->WriteCbcMultReg ( cCbc, cRegVec );
            }
        }
    }

    LOG (INFO) << "Disabling all TGroups except " << pTGrpId << " ! " ;
}


void PedeNoise::processSCurvesNoise ( TString pParameter, uint16_t pValue, bool pDraw, bool pFillSummary, int  pTGrpId )
{

    // First fitHits for every Channel, then extract the midpoint and noise and fill it in fVplusVcthGraphMap
    for ( auto& cCbc : fCbcChannelMap )
    {

        TH1F* cNoiseHist = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "Cbc_Noise" ) );
        TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "Cbc_Pedestal" ) );
        TH1F* cStripHist = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "Cbc_Stripnoise" ) );
        TH1F* cEvenHist  = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "Cbc_Noise_even" ) );
        TH1F* cOddHist   = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "Cbc_noise_odd" ) );

        // Loop the Channels
        bool cFirst = true;
        TString cOption;

        std::vector<uint8_t> cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

        for ( auto& cChanId : cTestGrpChannelVec )
        {
            //for ( auto& cChan : cCbc.second )
            Channel cChan = cCbc.second.at ( cChanId );

            // Fit or Differentiate
            if ( fFitted ) cChan.fitHist ( fEventsPerPoint, fHoleMode, pValue, pParameter, fResultFile );
            else cChan.differentiateHist ( fEventsPerPoint, fHoleMode, pValue, pParameter, fResultFile );


            // instead of the code below, use a histogram to histogram the noise
            if ( cChan.getNoise() == 0 || cChan.getNoise() > 1023 ) LOG (INFO) << RED << "Error, SCurve Fit for Fe " << int ( cCbc.first->getFeId() ) << " Cbc " << int ( cCbc.first->getCbcId() ) << " Channel " << int ( cChan.fChannelId ) << " did not work correctly! Noise " << cChan.getNoise() << RESET ;

            if (pFillSummary)
            {
                cNoiseHist->Fill ( cChan.getNoise() );
                cPedeHist->Fill ( cChan.getPedestal() );

                // Even and odd channel noise
                if ( ( int ( cChan.fChannelId ) % 2 ) == 0 )
                    cEvenHist->Fill ( int ( cChan.fChannelId / 2 ), cChan.getNoise() );
                else
                    cOddHist->Fill ( int ( cChan.fChannelId / 2.0 ), cChan.getNoise() );

                cStripHist->Fill ( cChan.fChannelId, cChan.getNoise() );
            }

            //Draw
            if ( pDraw )
            {
                if ( cFirst )
                {
                    cOption = "P" ;
                    cFirst = false;
                }
                else cOption = "P same";

                fNoiseCanvas->cd ( cCbc.first->getCbcId() + 1 );
                cChan.fScurve->SetLineColor (pTGrpId + 1);
                cChan.fScurve->SetMarkerColor (pTGrpId + 1);
                cChan.fScurve->DrawCopy ( cOption );

                //if ( fFitted )
                //cChan.fFit->DrawCopy ( "same" );
                //else cChan.fDerivative->DrawCopy ( "same" );
            }
        }

        if ( pDraw )
            fNoiseCanvas->Update();
    }

}

void PedeNoise::saveInitialOffsets()
{
    LOG (INFO) << "Initializing map with original Offsets for later ... " ;

    // save the initial offsets for Noise measurement in a map
    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // map to instert in fOffsetMap
                // <cChan, Offset>
                // std::map<uint8_t, uint8_t> cCbcOffsetMap;
                TH1F* cOffsetHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Offsets" ) );

                for ( uint8_t cChan = 0; cChan < NCHANNELS; cChan++ )
                {
                    TString cRegName = Form ( "Channel%03d", cChan + 1 );
                    uint8_t cOffset = cCbc->getReg ( cRegName.Data() );
                    //suggested B. Schneider
                    int iBin = cOffsetHist->FindBin (cChan);
                    cOffsetHist->SetBinContent ( iBin, cOffset );
                    // cCbcOffsetMap[cChan] = cOffset;
                    // LOG(INFO) << "DEBUG Original Offset for CBC " << cCbcId << " channel " << +cChan << " " << +cOffset ;
                }

                // fOffsetMap[cCbc] = cCbcOffsetMap;
            }
        }
    }
}

void PedeNoise::setInitialOffsets()
{
    LOG (INFO) << "Re-applying the original offsets for all CBCs" ;

    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // first, find the offset Histogram for this CBC
                TH1F* cOffsetHist = static_cast<TH1F*> ( getHist ( cCbc, "Cbc_Offsets" ) );

                //also write to CBCs
                RegisterVector cRegVec;

                for ( int iChan = 0; iChan < NCHANNELS; iChan++ )
                {
                    //suggested B. Schneider
                    int iBin = cOffsetHist->FindBin (iChan);
                    uint8_t cOffset = cOffsetHist->GetBinContent ( iBin );
                    cCbc->setReg ( Form ( "Channel%03d", iChan + 1 ), cOffset );
                    cRegVec.push_back ({ Form ( "Channel%03d", iChan + 1 ), cOffset } );
                    //LOG(INFO) << GREEN << "Offset for CBC " << cCbcId << " Channel " << iChan << " : 0x" << std::hex << +cOffset << std::dec << RESET ;
                }

                fCbcInterface->WriteCbcMultReg (cCbc, cRegVec);
            }
        }
    }
}

void PedeNoise::setThresholdtoNSigma (BeBoard* pBoard, uint32_t pNSigma)
{
    for ( auto cFe : pBoard->fModuleVector )
    {
        uint32_t cFeId = cFe->getFeId();

        for ( auto cCbc : cFe->fCbcVector )
        {
            uint32_t cCbcId = cCbc->getCbcId();
            TH1F* cNoiseHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Noise" ) );
            TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Pedestal" ) );

            uint16_t cPedestal = round (cPedeHist->GetMean() );
            uint16_t cNoise =  round (cNoiseHist->GetMean() );
            int cDiff = fHoleMode ? pNSigma * cNoise : -pNSigma * cNoise;
            uint16_t cValue = cPedestal + cDiff;


            if (pNSigma > 0) LOG (INFO) << "Changing Threshold on CBC " << +cCbcId << " by " << cDiff << " to " << cPedestal + cDiff << " VCth units to supress noise!" ;
            else LOG (INFO) << "Changing Threshold on CBC " << +cCbcId << " back to the pedestal at " << +cPedestal ;

            ThresholdVisitor cThresholdVisitor (fCbcInterface, cValue);
            cCbc->accept (cThresholdVisitor);
        }
    }
}

void PedeNoise::fillOccupancyHist (BeBoard* pBoard, const std::vector<Event*>& pEvents)
{
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cCbc : cFe->fCbcVector )
        {
            //get the histogram for the occupancy
            TH1F* cHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_occupancy" ) );

            for (auto& cEvent : pEvents)
            {
                //for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
                //{
                //if ( cEvent->DataBit ( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
                //cHist->Fill (cId);
                //}
                //experimental
                std::vector<uint32_t> cHits = cEvent->GetHits (cCbc->getFeId(), cCbc->getCbcId() );

                for (auto cHit : cHits)
                    cHist->Fill (cHit);
            }
        }
    }

}

void PedeNoise::writeObjects()
{
    this->SaveResults();
    // just use auto iterators to write everything to disk
    // this is the old method before Tool class was cool
    fResultFile->cd();

    // Save canvasses too
    fNoiseCanvas->Write ( fNoiseCanvas->GetName(), TObject::kOverwrite );
    fPedestalCanvas->Write ( fPedestalCanvas->GetName(), TObject::kOverwrite );
    //fFeSummaryCanvas->Write ( fFeSummaryCanvas->GetName(), TObject::kOverwrite );
    fResultFile->Flush();
}
