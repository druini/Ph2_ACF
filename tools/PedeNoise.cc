#include "PedeNoise.h"


PedeNoise::PedeNoise() :
    Tool(),
    fNoiseCanvas (nullptr),
    fPedestalCanvas (nullptr),
    fFeSummaryCanvas (nullptr),
    fNormHist (nullptr),
    fThresholdMap(),
    fHitCountMap(),
    fNCbc (0),
    fNFe (0),
    fHoleMode (false),
    fTestPulse (false),
    fFitted (false),
    fTestPulseAmplitude (0),
    fEventsPerPoint (0)
    // fSkipMaskedChannels(0)
{
}

PedeNoise::~PedeNoise()
{
}

void PedeNoise::Initialise (bool pAllChan, bool pDisableStubLogic)
{
    fDisableStubLogic = pDisableStubLogic;
    this->MakeTestGroups ( pAllChan );
    fAllChan = pAllChan;

    auto maskSkipSetting = fSettingsMap.find ( "SkipMaskedChannels" );
    fSkipMaskedChannels = ( maskSkipSetting != std::end ( fSettingsMap ) ) ? maskSkipSetting->second : false;


    //is to be called after system controller::InitialiseHW, InitialiseSettings
    // populates all the maps
    // create the canvases


    fPedestalCanvas = new TCanvas ( "Pedestal & Noise", "Pedestal & Noise", 670, 0, 650, 650 );
    //fFeSummaryCanvas = new TCanvas ( "Noise for each FE", "Noise for each FE", 0, 670, 650, 650 );
    fNoiseCanvas = new TCanvas ( "Final SCurves, Strip Noise", "Final SCurves, Noise", 0, 0, 650, 650 );


    uint16_t cStartValue = 0x000;

    uint32_t cCbcCount = 0;
    uint32_t cFeCount = 0;
    uint32_t cCbcIdMax = 0;

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

                TString cHistname;
                TH1F* cHist;


                //fo save the original offsets
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

                // initialize the hitcount and threshold map
                fThresholdMap[cCbc] = cStartValue;
                fHitCountMap[cCbc] = 0;
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
    LOG (INFO) << " FitSCurves = " << int ( fFitted ) ;

    if (fType == ChipType::CBC3)
        LOG (INFO) << BOLDBLUE << "Chip Type determined to be " << BOLDRED << "CBC3" << RESET;
    else
        LOG (INFO) << BOLDBLUE << "Chip Type determined to be " << BOLDRED << "CBC2" << RESET;

    fNormHist = new TH2F ("normalizationHistogram", "normalizationHistogram", NCHANNELS, -0.5, 253.5, 1024, -0.5, 1023.5);
    //fNormHist->Sumw2();

    for (int cBinX = 1; cBinX <= fNormHist->GetNbinsX(); cBinX++)
    {
        for (int cBinY = 1; cBinY <= fNormHist->GetNbinsY(); cBinY++)
            fNormHist->SetBinContent (cBinX, cBinY, fEventsPerPoint);
    }

    //determine the occupancy at Threshold = 0 to see if it is hole mode or not
    ThresholdVisitor cThresholdVisitor (fCbcInterface, cStartValue);
    this->accept (cThresholdVisitor);

    for ( BeBoard* pBoard : fBoardVector )
        this->measureOccupancy (pBoard, -1);

    cSetting = fSettingsMap.find ("HoleMode");

    if ( cSetting != std::end (fSettingsMap) )
    {
        bool cHoleModeFromSettings = cSetting->second;
        bool cHoleModeFromOccupancy = true;

        fHoleMode = cHoleModeFromSettings;
        std::string cMode = (fHoleMode) ? "Hole Mode" : "Electron Mode";

        for (auto& cCbc : fThresholdMap)
        {
            std::stringstream ss;
            float cOccupancy = fHitCountMap[cCbc.first] / float (fEventsPerPoint * NCHANNELS);
            cHoleModeFromOccupancy = (cOccupancy == 0) ? false :  true;

            if (cHoleModeFromOccupancy != cHoleModeFromSettings)
                ss << BOLDRED << "Be careful: " << RESET << "operation mode from settings does not correspond to the one found by measuring occupancy. Using the one from settings (" << BOLDYELLOW << cMode << RESET << ")";
            else
                ss << BOLDBLUE << "Measuring Occupancy @ Threshold " << BOLDRED << cCbc.second << BOLDBLUE << ": " << BOLDRED << cOccupancy << BOLDBLUE << ", thus assuming " << BOLDYELLOW << cMode << RESET << " (consistent with the settings file)";

            LOG (INFO) << ss.str();
        }
    }
    else
    {
        for (auto& cCbc : fThresholdMap)
        {
            float cOccupancy = fHitCountMap[cCbc.first] / float (fEventsPerPoint * NCHANNELS);
            fHoleMode = (cOccupancy == 0) ? false :  true;
            std::string cMode = (fHoleMode) ? "Hole Mode" : "Electron Mode";
            std::stringstream ss;
            ss << BOLDBLUE << "Measuring Occupancy @ Threshold " << BOLDRED << cCbc.second << BOLDBLUE << ": " << BOLDRED << cOccupancy << BOLDBLUE << ", thus assuming " << BOLDYELLOW << cMode << RESET;
            LOG (INFO) << ss.str();
        }
    }
}


std::string PedeNoise::sweepSCurves (uint8_t pTPAmplitude)
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
        cStartValue = this->findPedestal (-1);
    }

    // now initialize the Scurve histogram
    std::string cHistogramname = Form ("SCurves_TP%d", fTestPulseAmplitude);

    for (auto& cCbc : fHitCountMap)
    {
        TString cHistname = Form ( "Fe%dCBC%d_Scurves_TP%d", cCbc.first->getFeId(), cCbc.first->getCbcId(), fTestPulseAmplitude );
        TH2F* cHist = new TH2F ( cHistname, cHistname, NCHANNELS, -0.5, 253.5, 1024, -0.5, 1023.5 );
        cHist->Sumw2();
        bookHistogram ( cCbc.first, cHistogramname, cHist );

        fNoiseCanvas->cd ( cCbc.first->getCbcId() + 1 );
        cHist->Draw ( "colz2" );

    }

    saveInitialOffsets();

    // method to measure one final set of SCurves with the final calibration applied to extract the noise
    // now measure some SCurves
    for ( auto& cTGrpM : fTestGroupChannelMap )
    {
        // if we want to run with test pulses, we'll have to enable commissioning mode and enable the TP for each test group
        if (fTestPulse && cTGrpM.first != -1)
        {
            LOG (INFO) << GREEN <<  "Enabling Test Pulse for Test Group " << cTGrpM.first << " with amplitude " << +pTPAmplitude << RESET ;
            setFWTestPulse();
            setSystemTestPulse ( fTestPulseAmplitude, cTGrpM.first, true, fHoleMode );
        }
        else if (fTestPulse && cTGrpM.first == -1)
        {
            fTestPulse = false;
            LOG (INFO) << RED <<  "Test groups disabled. Can't enable Test Pulse for Test Group " << cTGrpM.first << RESET ;
        }

        if (cTGrpM.first != -1 || fAllChan)
        {

            LOG (INFO) << GREEN << "Measuring Test Group...." << cTGrpM.first << RESET ;
            // this leaves the offset values at the tuned values for cTGrp and disables all other groups
            enableTestGroupforNoise ( cTGrpM.first );

            // measure the SCurves, the false is indicating that I am sweeping Vcth
            measureSCurves ( cTGrpM.first, cHistogramname, cStartValue );

            for (auto& cCbc : fHitCountMap)
            {
                TH2F* cSCurveHist = dynamic_cast<TH2F*> (this->getHist (cCbc.first, cHistogramname) );
                fNoiseCanvas->cd (cCbc.first->getCbcId() + 1);
                double cMean = cSCurveHist->GetMean (2);
                TH1D* cTmp = cSCurveHist->ProjectionY();
                cSCurveHist->GetYaxis()->SetRangeUser ( cTmp->GetBinCenter (cTmp->FindFirstBinAbove (0) ) - 10, cTmp->GetBinCenter (cTmp->FindLastBinAbove (0.99) ) + 10 );
                //cSCurveHist->GetYaxis()->SetRangeUser (cMean - 30, cMean + 30);
                cSCurveHist->Draw ("colz2");
            }

            fNoiseCanvas->Modified();
            fNoiseCanvas->Update();

        }

        if (fTestPulse)
        {
            LOG (INFO) << RED <<  "Disabling Test Pulse for Test Group " << cTGrpM.first << RESET ;
            setSystemTestPulse ( 0, cTGrpM.first, false, fHoleMode );

        }
    }

    //dont forget to apply the original offsets
    setInitialOffsets();
    //now I should porcess the SCurves and differentiate the histogram
    processSCurves (cHistogramname);
    LOG (INFO) << BOLDBLUE << "Finished sweeping SCurves..."  << RESET ;
    return cHistogramname;
}

void PedeNoise::measureNoise (uint8_t pTPAmplitude)
{
    std::string cHistName = this->sweepSCurves (pTPAmplitude);
    this->extractPedeNoise (cHistName);
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
        this->HttpServerProcess();
    }
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

uint16_t PedeNoise::findPedestal (int pTGrpId)
{

    ThresholdVisitor cThresholdVisitor (fCbcInterface, 0);
    this->accept (cThresholdVisitor);

    uint16_t cNbits = (fType == ChipType::CBC2) ? 8 : 10;

    // now go over the VTh bits for each CBC, start with the MSB, flip it to one and measure the occupancy
    // VTh is 10 bits on CBC3
    bool cCloseEnough = false;

    for ( int iBit = cNbits - 1 ; iBit >= 0; iBit-- )
    {
        if (!cCloseEnough)
        {
            for (auto& cCbc : fThresholdMap)
            {
                toggleRegBit ( cCbc.second, iBit );
                cThresholdVisitor.setThreshold (cCbc.second);
                cCbc.first->accept (cThresholdVisitor);
                fHitCountMap[cCbc.first] = 0;
            }

            for ( BeBoard* pBoard : fBoardVector )
                this->measureOccupancy (pBoard, pTGrpId);

            // now I know how many hits there are with a Threshold of 0
            for (auto& cCbc : fThresholdMap)
            {
                const std::vector<uint8_t>& cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

                float cOccupancy = fHitCountMap[cCbc.first] / float (fEventsPerPoint * cTestGrpChannelVec.size() );

                LOG (DEBUG) << "IBIT " << +iBit << " DEBUG Setting VTh for CBC " << +cCbc.first->getCbcId() << " to " << +cCbc.second << " (= 0b" << std::bitset<10> ( cCbc.second ) << ") and occupancy " << cOccupancy ;

                if (fabs (cOccupancy - 0.5) < 0.06 )
                {
                    cCloseEnough = true;
                    break;
                }

                if (fHoleMode && cOccupancy < 0.45)
                    toggleRegBit (cCbc.second, iBit);
                else if (!fHoleMode && cOccupancy > 0.56)
                    toggleRegBit (cCbc.second, iBit);

                cThresholdVisitor.setThreshold (cCbc.second);
                cCbc.first->accept (cThresholdVisitor);
            }
        }
    }

    uint16_t cMean = 0;

    for (auto& cCbc : fThresholdMap)
    {
        cMean += cCbc.second;
        //re-set start value to 0 for next iteration
        cCbc.second = 0x000;
    }

    cMean /= fThresholdMap.size();

    LOG (INFO) << BOLDBLUE << "Found Pedestals to be around " << BOLDRED << cMean << " (0x" << std::hex << cMean << std::dec << ", 0b" << std::bitset<10> (cMean) << ")" << BOLDBLUE << " for Test Group " << pTGrpId << RESET;

    return cMean;
}

void PedeNoise::measureSCurves (int pTGrpId, std::string pHistName, uint16_t pStartValue)
{
    int cMinBreakCount = 10;
    const std::vector<uint8_t>& cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

    if (pStartValue == 0) pStartValue = this->findPedestal (pTGrpId);

    bool cAllZero = false;
    bool cAllOne = false;
    int cAllZeroCounter = 0;
    int cAllOneCounter = 0;
    uint16_t cValue = pStartValue;
    int cSign = 1;
    int cIncrement = 0;

    uint16_t cNbits = (fType == ChipType::CBC2) ? 8 : 10;
    uint16_t cMaxValue = (1 << cNbits) - 1;

    //start with the threshold value found above
    ThresholdVisitor cVisitor (fCbcInterface, cValue);

    while (! (cAllZero && cAllOne) )
    {

        for ( BeBoard* pBoard : fBoardVector )
        {
            for (Module* cFe : pBoard->fModuleVector)
            {
                cVisitor.setThreshold (cValue);
                cFe->accept (cVisitor);
            }
            
            ReadNEvents ( pBoard, fEventsPerPoint );

            const std::vector<Event*>& events = GetEvents ( pBoard );

            uint32_t cHitCounter = 0;
            uint32_t cMaxHits=0;

            // Loop over Events from this Acquisition
            for ( auto& ev : events )
            {
                //uint32_t cHitCounter = 0;

                for ( auto cFe : pBoard->fModuleVector )
                {
                    for ( auto cCbc : cFe->fCbcVector )
                    {
                        TH2F* cSCurveHist = dynamic_cast<TH2F*> (this->getHist (cCbc, pHistName) );
                         const uint32_t *cbcMask = cCbc->getCbcmask();

                         // std::cout<< (cbcMask[0]&0x0f) <<std::endl;

                        for ( auto& cChan : cTestGrpChannelVec )
                        {

                            bool isChannelEnabled = true;
                            // if(true){
                            if(fSkipMaskedChannels){
                                if(!( (cbcMask[cChan>>5]>>(cChan&0x1F)) &0x1) ){
                                    isChannelEnabled=false;
                                }
                            }

                            if(isChannelEnabled) ++cMaxHits;

                            if ( ev->DataBit ( cFe->getFeId(), cCbc->getCbcId(), cChan) )
                            {
                                //fill the strip number and the current threshold
                                cSCurveHist->Fill (cChan, cValue);
                                if(isChannelEnabled) ++cHitCounter;
                            }
                        }
                    }
                }
            }

            Counter cCbcCounter;
            pBoard->accept ( cCbcCounter );
            // uint32_t cMaxHits = fEventsPerPoint *   cCbcCounter.getNCbc() * cTestGrpChannelVec.size();

            //now establish if I'm zero or one
            if (cHitCounter == 0) ++cAllZeroCounter;

            if (cHitCounter > 0.98 * cMaxHits ) ++cAllOneCounter;

            // std::cout<<"Threshold = "<< cValue << " MaxHits = "<< cMaxHits <<" HitCounter = "<<cHitCounter<<std::endl;

            //it will either find one or the other extreme first and thus these will be mutually exclusive
            //if any of the two conditions is true, just revert the sign and go the opposite direction starting from startvalue+1
            //check that cAllZero is not yet set, otherwise I'll be reversing signs a lot because once i switch direction, the statement stays true
            if (!cAllZero && cAllZeroCounter == cMinBreakCount )
            {
                cAllZero = true;
                cSign = fHoleMode ? -1 : 1;
                cIncrement = 0;
            }

            if (!cAllOne && cAllOneCounter == cMinBreakCount)
            {
                cAllOne = true;
                cSign = fHoleMode ? 1 : -1;
                cIncrement = 0;
            }

            cIncrement++;
            // following checks if we're not going out of bounds
            if (cSign == 1 && (pStartValue + (cIncrement * cSign) > cMaxValue) )
            {
                std::cout<<"culo1\n";
                if (fHoleMode) cAllZero = true;
                else cAllOne = true;

                cIncrement = 1;
                cSign = -1 * cSign;
            }

            if (cSign == -1 && (pStartValue + (cIncrement * cSign) < 0) )
            {
                std::cout<<"culo2\n";
                if (fHoleMode) cAllOne = true;
                else cAllZero = true;

                cIncrement = 1;
                cSign = -1 * cSign;
            }


            LOG (DEBUG) << "All 0: " << cAllZero << " | All 1: " << cAllOne << " current value: " << cValue << " | next value: " << pStartValue + (cIncrement * cSign) << " | Sign: " << cSign << " | Increment: " << cIncrement << " Hitcounter: " << cHitCounter << " Max hits: " << cMaxHits;
            cValue = pStartValue + (cIncrement * cSign);
        }
    }

    this->HttpServerProcess();
    LOG (INFO) << YELLOW << "Found minimal and maximal occupancy " << cMinBreakCount << " times, SCurves finished! " << RESET ;
}

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

    LOG (INFO) << RED << "Disabling all TGroups except " << pTGrpId << " ! " << RESET;
}

void PedeNoise::processSCurves (std::string pHistName)
{
    for (auto& cCbc : fHitCountMap)
    {
        TH2F* cHist = dynamic_cast<TH2F*> ( getHist ( cCbc.first, pHistName) );
        //cHist->Scale (1 / double_t (fEventsPerPoint) );
        //in order to have proper binomial errors
        cHist->Divide (cHist, fNormHist, 1, 1, "B");
        //do this in any case!
        this->differentiateHist (cCbc.first, pHistName);

        //only do this if requested
        if (fFitted)
            this->fitHist (cCbc.first, pHistName);

    }

    //end of CBC loop
}

void PedeNoise::differentiateHist (Cbc* pCbc, std::string pHistName)
{
    //first get the SCurveHisto and create a differential histo
    TH2F* cHist = dynamic_cast<TH2F*> ( getHist ( pCbc, pHistName) );
    TString cHistname = Form ( "Fe%dCBC%d_Differential_TP%d", pCbc->getFeId(), pCbc->getCbcId(), fTestPulseAmplitude );
    TH2F* cDerivative = new TH2F ( cHistname, cHistname, NCHANNELS, -0.5, 253.5, 1024, 0, 1024 );
    cDerivative->Sumw2();
    bookHistogram ( pCbc, pHistName + "_Diff", cDerivative );

    for (uint16_t cChan = 0; cChan < NCHANNELS; cChan++)
    {
        //if(!fFitted)
        //get a projection
        int iBin = cHist->GetXaxis()->FindBin (cChan);
        TH1D* cProjection = cHist->ProjectionY ("_py", iBin, iBin);

        double_t cDiff;
        double_t cCurrent;
        double_t cPrev;
        bool cActive; // indicates existence of data points
        int cStep = 1;
        int cDiffCounter = 0;

        double cBin = 0;

        if ( fHoleMode )
        {
            cPrev = cProjection->GetBinContent ( cProjection->FindBin ( 0 ) );
            cActive = false;

            for ( cBin = cProjection->FindBin (0); cBin <= cProjection->FindBin (1023); cBin++ )
            {
                //veify that this happens exactly 1023
                cCurrent = cProjection->GetBinContent (cBin);
                cDiff = cPrev - cCurrent;

                if ( cPrev > 0.75 ) cActive = true; // sampling begins

                int iBinDerivative = cDerivative->FindBin (cChan, (cProjection->GetBinCenter (cBin + 1) + cProjection->GetBinCenter (cBin) ) / 2.0);

                if ( cActive ) cDerivative->SetBinContent ( iBinDerivative, cDiff  );

                if ( cActive && cDiff == 0 && cCurrent == 0 ) cDiffCounter++;

                if ( cDiffCounter == 8 ) break;

                cPrev = cCurrent;
            }
        }
        else
        {
            cPrev = cProjection->GetBinContent ( cProjection->FindBin ( 1023 ) );
            cActive = false;

            for ( cBin = cProjection->FindBin (1023); cBin >= cProjection->FindBin ( 0); cBin-- )
            {
                cCurrent = cProjection->GetBinContent (cBin);
                cDiff = cPrev - cCurrent;

                if ( cPrev > 0.75 ) cActive = true; // sampling begins

                int iBinDerivative = cDerivative->FindBin ( cChan, (cProjection->GetBinCenter (cBin - 1 ) + cProjection->GetBinCenter (cBin ) ) / 2.0);

                if ( cActive ) cDerivative->SetBinContent ( iBinDerivative, cDiff  );

                if ( cActive && cDiff == 0 && cCurrent == 0 ) cDiffCounter++;

                if ( cDiffCounter == 8 ) break;

                cPrev = cCurrent;
            }
        }

        //end of channel loop
    }

    //end of CBC loop
}

void PedeNoise::fitHist (Cbc* pCbc, std::string pHistName)
{
    //first get the SCurveHisto and create a differential histo
    TH2F* cHist = dynamic_cast<TH2F*> ( getHist ( pCbc, pHistName) );
    TString cDirName = Form ("FE%dCBC%d/%s_Fits", pCbc->getFeId(), pCbc->getCbcId(), pHistName.c_str() );
    TDirectory* cDir = dynamic_cast<TDirectory*> (gROOT->FindObject (cDirName) );

    if (!cDir) fResultFile->mkdir (cDirName);

    fResultFile->cd (cDirName);
    //since this is a bit of a special situation I need to create a directory for the SCurves and their fits inside the FExCBCx direcotry and make sure they are saved here

    for (uint16_t cChan = 0; cChan < NCHANNELS; cChan++)
    {
        //get a projection
        TString cProjectionName = Form ("%s_Channel%d", cHist->GetName(), cChan);
        int iBin = cHist->GetXaxis()->FindBin (cChan);
        TH1D* cProjection = cHist->ProjectionY (cProjectionName, iBin, iBin);
        double cFirstNon0 ( 0 );
        double cFirst1 ( 0 );
        std::string cFitname = "SCurveFit";
        TF1* cFit = dynamic_cast<TF1*> (gROOT->FindObject (cFitname.c_str() ) );

        if (cFit) delete cFit;

        // Not Hole Mode
        if ( !fHoleMode )
        {
            for ( Int_t cBin = 1; cBin < cProjection->GetNbinsX() - 1; cBin++ )
            {
                double cContent = cProjection->GetBinContent ( cBin );

                if ( !cFirstNon0 )
                {
                    if ( cContent ) cFirstNon0 = cProjection->GetBinCenter ( cBin );
                }
                else if ( cContent > 0.85 )
                {
                    cFirst1 = cProjection->GetBinCenter ( cBin );
                    break;
                }
            }

            cFit = new TF1 ( "SCurveFit", MyErf, cFirstNon0 - 10, cFirst1 + 10, 2 );
        }
        // Hole mode
        else
        {
            for ( Int_t cBin = cProjection->GetNbinsX() - 1; cBin > 1; cBin-- )
            {
                double cContent = cProjection->GetBinContent ( cBin );

                if ( !cFirstNon0 )
                {
                    if ( cContent ) cFirstNon0 = cProjection->GetBinCenter ( cBin );
                }
                else if ( cContent > 0.85 )
                {
                    cFirst1 = cProjection->GetBinCenter ( cBin );
                    break;
                }
            }

            cFit = new TF1 (cFitname.c_str(), MyErf, cFirst1 - 10, cFirstNon0 + 10, 2 );
        }

        // Get rough midpoint & width
        double cMid = ( cFirst1 + cFirstNon0 ) * 0.5;
        double cWidth = ( cFirst1 - cFirstNon0 ) * 0.5;

        cFit->SetParameter ( 0, cMid );
        cFit->SetParameter ( 1, cWidth );

        // Fit
        cProjection->Fit ( cFit, "RQ+" );
        cProjection->SetDirectory (cDir);
        cProjection->Write (cProjection->GetName(), TObject::kOverwrite);
    }
}

void PedeNoise::extractPedeNoise (std::string pHistName)
{
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
                // first the derivative of the scurves
                TH2F* cDerivative = dynamic_cast<TH2F*> (getHist (cCbc, pHistName + "_Diff") );
                //and then everything else
                TH1F* cNoiseHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Noise" ) );
                TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Pedestal" ) );
                TH1F* cStripHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Stripnoise" ) );
                TH1F* cEvenHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Noise_even" ) );
                TH1F* cOddHist   = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_noise_odd" ) );

                // now fill the various histograms
                for (uint16_t cChan = 0; cChan < NCHANNELS; cChan++)
                {
                    //get a projection to contain the derivative of the scurve
                    TH1D* cProjection = cDerivative->ProjectionY ("_py", cChan + 1, cChan + 1);

                    if ( cProjection->GetRMS() == 0 || cProjection->GetRMS() > 1023 ) LOG (INFO) << RED << "Error, SCurve Fit for Fe " << int ( cCbc->getFeId() ) << " Cbc " << int ( cCbc->getCbcId() ) << " Channel " << cChan << " did not work correctly! Noise " << cProjection->GetRMS() << RESET ;

                    cNoiseHist->Fill ( cProjection->GetRMS() );
                    cPedeHist->Fill ( cProjection->GetMean() );

                    // Even and odd channel noise
                    if ( ( int (cChan) % 2 ) == 0 )
                        cEvenHist->Fill ( int ( cChan / 2 ), cProjection->GetRMS() );
                    else
                        cOddHist->Fill ( int ( cChan / 2.0 ), cProjection->GetRMS() );

                    cStripHist->Fill ( cChan, cProjection->GetRMS() );
                }

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

            //end of cbc loop
        }

        //end of Fe loop
        this->HttpServerProcess();
    }

    //end of board loop
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
                    LOG (DEBUG) << "Original Offset for CBC " << cCbcId << " channel " << +cChan << " " << +cOffset ;
                }
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

                if (cCbc->getChipType() == ChipType::CBC3 && fDisableStubLogic)
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - re-enabling stub logic to original value!" << RESET;
                    cRegVec.push_back ({"Pipe&StubInpSel&Ptwidth", fStubLogicValue[cCbc]});
                    cRegVec.push_back ({"HIP&TestMode", fHIPCountValue[cCbc]});
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

void PedeNoise::measureOccupancy (BeBoard* pBoard, int pTGrpId)
{
    ReadNEvents ( pBoard, fEventsPerPoint );

    //now decode the events and measure the occupancy on the chip
    //in the first iteration, check if I'm in hole mode or electron mode
    std::vector<Event*> events = GetEvents ( pBoard );

    for ( auto& cEvent : events )
    {
        for ( auto cFe : pBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fCbcVector )
            {
                std::map<Cbc*, uint32_t>::iterator cHitCounter = fHitCountMap.find (cCbc);

                if (cHitCounter != fHitCountMap.end() )
                {
                    const std::vector<uint8_t>& cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

                    for ( auto& cChan : cTestGrpChannelVec )
                    {
                        if ( cEvent->DataBit ( cFe->getFeId(), cCbc->getCbcId(),  cChan ) )
                            cHitCounter->second++;
                    }
                }
                else LOG (INFO) << RED << "Error: could not find the HitCounter for CBC " << int ( cCbc->getCbcId() ) << RESET ;
            }
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
