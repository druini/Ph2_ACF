
#include "CMTester.h"

// This has no bad-strip masking and does not take a reduced number of active strips into account yet!

// PUBLIC METHODS

void CMTester::Initialize()
{
    // gStyle->SetOptStat( 000000 );
    // gStyle->SetTitleOffset( 1.3, "Y" );
    for ( auto& cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto& cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto& cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // Fill Canvas Map
                TCanvas* ctmpCanvas = new TCanvas ( Form ( "c_online_canvas_fe%d_cbc%d", cFeId, cCbcId ), Form ( "FE%d CBC%d Online Canvas", cFeId, cCbcId ), 800, 800 );
                ctmpCanvas->Divide ( 2, 2 );
                fCanvasMap[cCbc] = ctmpCanvas;

                // here create an empty std::set<int> for noisy strips
                std::set<int> cTmpSet;
                fNoiseStripMap[cCbc] = cTmpSet;

                // here create the CBC-wise histos

                // histogram for the number of hits
                TString cName = Form ( "h_nhits_Fe%dCbc%d", cFeId, cCbcId ) ;
                TObject* cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TH1F* cHist = new TH1F ( cName, Form ( "Number of Hits FE%d CBC%d; Hits; Count", cFeId, cCbcId ), NCHANNELS + 1, -.5, NCHANNELS + 0.5 );
                cHist->SetLineColor ( 9 );
                cHist->SetLineWidth ( 2 );
                bookHistogram ( cCbc, "nhits", cHist );

                // 2D profile for the combined odccupancy
                cName = Form ( "p_combinedoccupancy_Fe%dCbc%d", cFeId, cCbcId );
                cObj = ( TProfile2D* ) gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                //  no clue why i can not call it cName but when I do, it produces a segfault!!
                TProfile2D* c2DOccProfile = new TProfile2D ( cName, Form ( "Combined Occupancy FE%d CBC%d; Strip; Strip; Occupancy", cFeId, cCbcId ),  NCHANNELS + 1, -0.5, NCHANNELS + 0.5, NCHANNELS + 1, -0.5, NCHANNELS + 0.5 );

                c2DOccProfile->SetMarkerColor ( 1 );
                bookHistogram ( cCbc, "combinedoccupancy", c2DOccProfile );

                // 2D Profile for correlation coefficient
                cName =  Form ( "p_correlation_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TH2F* c2DHist = new TH2F ( cName, Form ( "Correlation FE%d CBC%d; Strip; Strip; Correlation coefficient", cFeId, cCbcId ), NCHANNELS + 1, -.5, NCHANNELS + 0.5, NCHANNELS + 1, -.5, NCHANNELS + 0.5 );
                bookHistogram ( cCbc, "correlation", c2DHist );

                // 1D projection of the combined odccupancy
                cName =  Form ( "p_occupancyprojection_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TProfile* cProfile = new TProfile ( cName, Form ( "Projection of combined Occupancy FE%d CBC%d;  NNeighbors; Probability", cFeId, cCbcId ), NCHANNELS + 1, -.5, NCHANNELS + 0.5 );
                cProfile->SetLineColor ( 9 );
                cProfile->SetLineWidth ( 2 );
                bookHistogram ( cCbc, "occupancyprojection", cProfile );

                // 1D projection of the uncorrelated odccupancy
                cName = Form ( "p_uncorr_occupancyprojection_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                cProfile = new TProfile ( cName, Form ( "Projection of uncorrelated Occupancy FE%d CBC%d;  NNeighbors; Probability", cFeId, cCbcId ), NCHANNELS + 1, -.5, NCHANNELS + 0.5 );
                cProfile->SetLineColor ( 2 );
                cProfile->SetLineWidth ( 2 );
                bookHistogram ( cCbc, "uncorr_occupancyprojection", cProfile );

                // 1D projection of the correlation
                cName =  Form ( "p_correlationprojection_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                cProfile = new TProfile ( cName, Form ( "Projection of Correlation FE%d CBC%d;  NNeighbors; Correlation", cFeId, cCbcId ), NCHANNELS + 1, -.5, NCHANNELS + 0.5 );
                cProfile->SetLineColor ( 9 );
                cProfile->SetLineWidth ( 2 );
                bookHistogram ( cCbc, "correlationprojection", cProfile );

                // 1D hit probability profile
                cName = Form ( "p_hitprob_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                cProfile = new TProfile ( cName, Form ( "Hit Probability FE%d CBC%d;  Strip; Probability", cFeId, cCbcId ), NCHANNELS + 1, -.5, NCHANNELS + 0.5 );
                cProfile->SetLineColor ( 9 );
                cProfile->SetLineWidth ( 2 );
                bookHistogram ( cCbc, "hitprob", cProfile );

                // dummy TF1* for fit & dummy TH1F* for 0CM
                cName = Form ( "f_nhitsfit_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TF1* cCmFit = new TF1 ( cName, hitProbFunction, 0, 255, 4 );
                bookHistogram ( cCbc, "nhitsfit", cCmFit );

                cName = Form ( "h_nocm_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TH1F* cNoCM = new TH1F ( cName, "Noise hit distributtion", NCHANNELS + 1, -0.5, NCHANNELS + 0.5 );
                cNoCM->SetLineColor ( 16 );
                bookHistogram ( cCbc, "nocm", cNoCM );
            }

            // PER MODULE PLOTS
            uint32_t cNCbc = cFe->getNCbc();

            // 2D profile for the combined odccupancy
            TString cName =  Form ( "p_module_combinedoccupancy_Fe%d", cFeId ) ;
            TObject* cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TProfile2D* c2DProfile = new TProfile2D ( cName, Form ( "Combined Occupancy FE%d; Strip; Strip; Occupancy", cFeId ), cNCbc * NCHANNELS + 1, -.5, cNCbc * NCHANNELS + 0.5, cNCbc * NCHANNELS + 1, -.5, cNCbc * NCHANNELS + 0.5 );
            bookHistogram ( cFe, "module_combinedoccupancy", c2DProfile );

            // 2D Hist for correlation coefficient
            cName =  Form ( "p_module_correlation_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH2F* c2DHist = new TH2F ( cName, Form ( "Correlation FE%d CBC%d; Strip; Strip; Correlation coefficient", cFeId ), cNCbc * NCHANNELS + 1, -.5, cNCbc * NCHANNELS + 0.5, cNCbc * NCHANNELS + 1, -.5, cNCbc * NCHANNELS + 0.5 );
            bookHistogram ( cFe, "module_correlation", c2DHist );

            // 1D projection of the combined odccupancy
            cName =  Form ( "p_module_occupancyprojection_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TProfile* cProfile = new TProfile ( cName, Form ( "Projection of combined Occupancy FE%d;  NNeighbors; Probability", cFeId ), cNCbc * NCHANNELS + 1, -.5, cNCbc * NCHANNELS + 0.5 );
            cProfile->SetLineColor ( 9 );
            cProfile->SetLineWidth ( 2 );
            bookHistogram ( cFe, "module_occupancyprojection", cProfile );

            // 1D projection of the uncorrelated occupancy
            cName = Form ( "p_module_uncorr_occupancyprojection_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            cProfile = new TProfile ( cName, Form ( "Projection of uncorrelated Occupancy FE%d;  NNeighbors; Probability", cFeId ),  cNCbc * NCHANNELS + 1, -.5, cNCbc * NCHANNELS + 0.5 );
            cProfile->SetLineColor ( 2 );
            cProfile->SetLineWidth ( 2 );
            bookHistogram ( cFe, "module_uncorr_occupancyprojection", cProfile );

            // 1D projection of the correlation
            cName =  Form ( "p_module_correlationprojection_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            cProfile = new TProfile ( cName, Form ( "Projection of Correlation FE%d;  NNeighbors; Correlation", cFeId ),  cNCbc * NCHANNELS + 1, -.5, cNCbc * NCHANNELS + 0.5 );
            cProfile->SetLineColor ( 9 );
            cProfile->SetLineWidth ( 2 );
            bookHistogram ( cFe, "module_correlationprojection", cProfile );
        }
    }

    // initializeHists();

    LOG (INFO) << "Histograms and Settings initialised." ;
}



void CMTester::ScanNoiseChannels()
{
    LOG (INFO) << "Scanning for noisy channels! " ;
    uint32_t cTotalEvents = 500;

    for ( BeBoard* pBoard : fBoardVector )
    {
        uint32_t cN = 1;
        uint32_t cNthAcq = 0;

        //fBeBoardInterface->Start ( pBoard );

        //while ( cN <=  cTotalEvents )
        //{
        ReadNEvents ( pBoard, cTotalEvents );
        const std::vector<Event*>& events = GetEvents ( pBoard );

        // Loop over Events from this Acquisition
        for ( auto& cEvent : events )
        {
            for ( auto& cFe : pBoard->fModuleVector )
            {
                for ( auto& cCbc : cFe->fCbcVector )
                {
                    // just re-use the hitprobability histogram here?
                    // this has to go into a dedicated method
                    TProfile* cNoiseStrips = dynamic_cast<TProfile*> ( getHist ( cCbc, "hitprob" ) );

                    const std::vector<bool>& list = cEvent->DataBitVector ( cFe->getFeId(), cCbc->getCbcId() );
                    int cChan = 0;

                    for ( const auto& b : list )
                    {
                        int fillvalue = ( b ) ? 1 : 0;
                        cNoiseStrips->Fill ( cChan++, fillvalue );
                    }
                }
            }

            if ( cN % 100 == 0 )
                // updateHists();
                LOG (INFO) << "Acquired " << cN << " Events for Noise Strip Scan!" ;

            cN++;
        }

        cNthAcq++;
        //} // End of Analyze Events of last Acquistion loop

        //fBeBoardInterface->Stop ( pBoard );
    }

    // done taking data, now iterate over p_noisestrips and find out the bad strips, push them into the fNoiseStripMap, then clear the histogram
    for ( const auto& cCbc : fCbcHistMap )
    {

        TProfile* cNoiseStrips = dynamic_cast<TProfile*> ( getHist ( cCbc.first,  "hitprob" ) );

        auto cNoiseSet  =  fNoiseStripMap.find ( cCbc.first );

        if ( cNoiseSet == std::end ( fNoiseStripMap ) ) LOG (ERROR) << " Error: Could not find noisy strip container for CBC " << int ( cCbc.first->getCbcId() ) ;
        else
        {
            double cMean = cNoiseStrips->GetMean ( 2 );
            double cNoise = cNoiseStrips->GetRMS ( 2 );

            LOG (INFO) << "Found average Occupancy of " << cMean ;

            for ( int  cBin = 0; cBin < cNoiseStrips->GetNbinsX(); cBin++ )
            {
                double cStripOccupancy = cNoiseStrips->GetBinContent ( cBin );

                if ( fabs ( cStripOccupancy - cMean ) > cMean / 2 )
                {
                    cNoiseSet->second.insert ( cNoiseStrips->GetBinCenter ( cBin ) );
                    LOG (INFO) << "Found noisy Strip on CBC " << int ( cCbc.first->getCbcId() ) << " : " << cNoiseStrips->GetBinCenter ( cBin ) ;
                }
            }
        }

        cNoiseStrips->Reset();
    }
}

void CMTester::TakeData()
{
    parseSettings();

    //CbcRegReader cReader ( fCbcInterface, "VCth" );
    // accept( cReader );

    for ( BeBoard* pBoard : fBoardVector )
    {
        uint32_t cN = 0;
        uint32_t cNthAcq = 0;

        fBeBoardInterface->Start ( pBoard );

        //while ( cN <=  fNevents )
        //{
        // Run( pBoard, cNthAcq );
        ReadNEvents ( pBoard, fNevents );
        const std::vector<Event*>& events = GetEvents ( pBoard );

        // Loop over Events from this Acquisition

        for ( auto& cEvent : events )
        {
            analyze ( pBoard, cEvent );

            if ( cN % 100 == 0 )
            {
                LOG (INFO) << cN << " Events recorded!" ;
                updateHists();
            }

            cN++;
        }

        cNthAcq++;
        //} // End of Analyze Events of last Acquistion loop

        //fBeBoardInterface->Stop ( pBoard );
    }

    updateHists();
}



void CMTester::FinishRun()
{
    //  Iterate through maps, pick histogram that I need and the other one
    LOG (INFO) << "Fitting and computing aditional histograms ... " ;
    // first CBCs
    LOG (INFO) << "per CBC ..";

    for ( auto cCbc : fCbcHistMap )
    {

        TH1F* cTmpNHits = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "nhits" ) );
        TH1F* cNoCM = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "nocm" ) );
        TF1* cNHitsFit = dynamic_cast<TF1*> ( getHist ( cCbc.first, "nhitsfit" ) );

        // here I need the number of active channels which i can get from the noise strip set
        auto cNoiseStrips = fNoiseStripMap.find ( cCbc.first );
        uint32_t cNactiveChan = ( cNoiseStrips != std::end ( fNoiseStripMap ) ) ? ( NCHANNELS - cNoiseStrips->second.size() ) :  NCHANNELS;

        // Fit NHits and create 0 CM
        fitDistribution ( cTmpNHits, cNHitsFit, cNactiveChan );
        createNoiseDistribution ( cNoCM, cNHitsFit->GetParameter ( 0 ), 0, cNHitsFit->GetParameter ( 2 ), cNHitsFit->GetParameter ( 3 ) );

        // now compute the correlation coefficient and the uncorrelated probability
        TProfile2D* cTmpOccProfile = dynamic_cast<TProfile2D*> ( getHist ( cCbc.first, "combinedoccupancy" ) );
        TProfile* cUncorrHitProb = dynamic_cast<TProfile*> ( getHist ( cCbc.first, "uncorr_occupancyprojection" ) );
        TH2F* cCorrelation2D = dynamic_cast<TH2F*> ( getHist ( cCbc.first, "correlation" ) );
        TProfile* cCorrProjection = dynamic_cast<TProfile*> ( getHist ( cCbc.first,  "correlationprojection" ) );


        for ( int cIdx = 0; cIdx < cTmpOccProfile->GetNbinsX(); cIdx++ )
        {
            for ( int cIdy = 0; cIdy < cTmpOccProfile->GetNbinsY(); cIdy++ )
            {
                double xx = cTmpOccProfile->GetBinContent ( cIdx, cIdx );
                double yy = cTmpOccProfile->GetBinContent ( cIdy, cIdy );
                double xy = cTmpOccProfile->GetBinContent ( cIdx, cIdy );

                // Fill the correlation & the uncorrelated probability
                // frac(Oxy-OxOy)(sqrt(Ox-Ox^2)*sqrt(Oy-Oy^2))
                cCorrelation2D->SetBinContent ( cIdx, cIdy, ( xy - xx * yy ) / ( sqrt ( xx - pow ( xx, 2 ) ) * sqrt ( yy - pow ( yy, 2 ) ) ) );

                if ( xx != 0 && yy != 0 ) cUncorrHitProb->Fill ( cIdx - cIdy, xx * yy );

                // and finally project the correlation
                xy = cCorrelation2D->GetBinContent ( cIdx, cIdy );

                if ( xy == xy ) cCorrProjection->Fill ( cIdx - cIdy, xy );
            }
        }
    }

    LOG (INFO) << " done!" ;
    LOG (INFO) << "per module ... ";

    // now module wise
    for ( auto& cFe : fModuleHistMap )
    {
        TString cName = Form ( "FE%d", cFe.first->getFeId() );

        // get histograms
        TProfile2D* cTmpOccProfile = dynamic_cast<TProfile2D*> ( getHist ( cFe.first, "module_combinedoccupancy" ) );
        TProfile* cUncorrHitProb = dynamic_cast<TProfile*> ( getHist ( cFe.first, "module_uncorr_occupancyprojection" ) );
        TH2F* cCorrelation2D = dynamic_cast<TH2F*> ( getHist ( cFe.first, "module_correlation" ) );
        TProfile* cCorrProjection = dynamic_cast<TProfile*> ( getHist ( cFe.first,  "module_correlationprojection" ) );

        for ( int cIdx = 0; cIdx < cTmpOccProfile->GetNbinsX(); cIdx++ )
        {
            for ( int cIdy = 0; cIdy < cTmpOccProfile->GetNbinsY(); cIdy++ )
            {
                double xx = cTmpOccProfile->GetBinContent ( cIdx, cIdx );
                double yy = cTmpOccProfile->GetBinContent ( cIdy, cIdy );
                double xy = cTmpOccProfile->GetBinContent ( cIdx, cIdy );

                // Fill the correlation & the uncorrelated probability
                // frac(Oxy-OxOy)(sqrt(Ox-Ox^2)*sqrt(Oy-Oy^2))
                cCorrelation2D->SetBinContent ( cIdx, cIdy, ( xy - xx * yy ) / ( sqrt ( xx - pow ( xx, 2 ) ) * sqrt ( yy - pow ( yy, 2 ) ) ) );

                if ( xx != 0 && yy != 0 ) cUncorrHitProb->Fill ( cIdx - cIdy, xx * yy );

                // and finally project the correlation
                xy = cCorrelation2D->GetBinContent ( cIdx, cIdy );

                if ( xy == xy ) cCorrProjection->Fill ( cIdx - cIdy, xy );
            }
        }
    }

    LOG (INFO) << " done!" ;
    // Not drawing anything yet
    updateHists ( true );
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS

void CMTester::analyze ( BeBoard* pBoard, const Event* pEvent )
{
    uint32_t cHitCounter = 0;

    for ( auto& cFe : pBoard->fModuleVector )
    {

        std::vector<bool> cModuleData; // use this to store data for all CBCs....

        for ( auto& cCbc : cFe->fCbcVector )
        {

            // here loop over the channels and fill the histograms
            // dont forget to get them first
            TH1F* cTmpNHits = dynamic_cast<TH1F*> ( getHist ( cCbc, "nhits" ) );
            TProfile* cTmpHitProb = dynamic_cast<TProfile*> ( getHist ( cCbc, "hitprob" ) );
            TProfile2D* cTmpOccProfile = dynamic_cast<TProfile2D*> ( getHist ( cCbc, "combinedoccupancy" ) );
            TProfile* cTmpCombinedOcc = dynamic_cast<TProfile*> ( getHist ( cCbc, "occupancyprojection" ) );

            int cNHits = 0;

            // here add a check if the strip is masked and if I am simulating or not!
            std::vector<bool> cSimResult;

            if ( fDoSimulate )
            {
                for ( int cChan = 0; cChan < 254; cChan++ )
                {
                    bool cResult = randHit ( fSimOccupancy / float ( 100 ) );
                    cSimResult.push_back ( cResult );
                }
            }

            for ( int cChan = 0; cChan < NCHANNELS; cChan++ )
            {
                bool chit;

                if ( fDoSimulate ) chit = cSimResult.at ( cChan );
                else chit = pEvent->DataBit ( cFe->getFeId(), cCbc->getCbcId(), cChan );

                // move the CBC data in a vector that has data for the whole module
                cModuleData.push_back ( chit );

                //  count hits/event
                if ( chit  && !isMasked ( cCbc, cChan ) )
                    cNHits++;

                // Fill Single Strip Efficiency
                if ( !isMasked ( cCbc, cChan ) ) cTmpHitProb->Fill ( cChan, int ( chit ) );

                // For combined occupancy 1D projection & 2D profile
                for ( int cChan2 = 0; cChan2 < 254; cChan2++ )
                {
                    bool chit2;

                    if ( fDoSimulate ) chit2 = cSimResult.at ( cChan2 );
                    else chit2 = pEvent->DataBit ( cFe->getFeId(), cCbc->getCbcId(), cChan2 );

                    int cfillValue = 0;

                    if ( chit && chit2 ) cfillValue = 1;

                    if ( !isMasked ( cCbc, cChan ) && !isMasked ( cCbc, cChan2 ) )
                    {
                        // Fill 2D occupancy
                        cTmpOccProfile->Fill ( cChan, cChan2, cfillValue );

                        // Fill projection: this could be done in FinishRun() but then no live updates
                        if ( cChan - cChan2 >= 0 ) cTmpCombinedOcc->Fill ( cChan - cChan2, cfillValue );
                    }
                }
            }

            // Fill NHits Histogram
            cTmpNHits->Fill ( cNHits );

        }

        // Here deal with per-module Histograms
        TProfile2D* cTmpOccProfile = dynamic_cast<TProfile2D*> ( getHist ( cFe,  "module_combinedoccupancy" ) );
        TProfile* cTmpCombinedOcc = dynamic_cast<TProfile*> ( getHist ( cFe, "module_occupancyprojection" ) );

        uint32_t cChanCt1 = 0;

        // since I use the module bool vector i constructed myself this already includes simulation results if simulation flag is set!
        for ( auto cChan1 : cModuleData )
        {
            uint32_t cChanCt2 = 0;

            for ( auto cChan2 : cModuleData )
            {
                int fillvalue = 0;

                if ( cChan1 && cChan2 )  fillvalue = 1;

                if ( !isMasked ( cChanCt1 ) && !isMasked ( cChanCt2 ) )
                {
                    cTmpOccProfile->Fill ( cChanCt1, cChanCt2, fillvalue );
                    cTmpCombinedOcc->Fill ( cChanCt1 - cChanCt2, fillvalue );
                }

                cChanCt2++;
            }

            cChanCt1++;
        }
    }
}

void CMTester::updateHists ( bool pFinal )
{
    // method to iterate over the histograms that I want to draw and update the canvases
    for ( auto& cCbc : fCbcHistMap )
    {
        auto cCanvas = fCanvasMap.find ( cCbc.first );

        if ( cCanvas == fCanvasMap.end() ) LOG (INFO) << "Error: could not find the canvas for Cbc " << int ( cCbc.first->getCbcId() ) ;
        else
        {
            TH1F* cTmpNHits = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "nhits" ) );
            TProfile2D* cTmpOccProfile = dynamic_cast<TProfile2D*> ( getHist ( cCbc.first, "combinedoccupancy" ) );
            TProfile* cTmpCombinedOcc = dynamic_cast<TProfile*> ( getHist ( cCbc.first, "occupancyprojection" ) );
            TProfile* cUncorrHitProb;
            TH1F* cNoCM;
            TF1* cCMFit;
            TProfile* cCorrProjection;


            if ( pFinal )
            {
                cUncorrHitProb = dynamic_cast<TProfile*> ( getHist ( cCbc.first, "uncorr_occupancyprojection" ) );
                cNoCM = dynamic_cast<TH1F*> ( getHist ( cCbc.first, "nocm" ) );
                cCMFit = dynamic_cast<TF1*> ( getHist ( cCbc.first, "nhitsfit" ) );
                cCorrProjection = dynamic_cast<TProfile*> ( getHist ( cCbc.first,  "correlationprojection" ) );
            }

            // Get the 4 things I want to draw and draw it!
            // 1. NHits
            cCanvas->second->cd ( 1 );

            if ( pFinal )
            {
                cNoCM->Draw( );
                cTmpNHits->Draw ( "same" );
                cCMFit->Draw ( "same" );
                TLegend* cLegend = new TLegend ( 0.13, 0.66, 0.38, 0.88, "" );
                cLegend->SetBorderSize ( 0 );
                cLegend->SetFillColor ( kWhite );
                cLegend->AddEntry ( cTmpNHits, "Data", "f" );
                cLegend->AddEntry ( cCMFit, Form ( "Fit (CM %4.2f+-%4.2f, THR %4.2f)", fabs ( cCMFit->GetParameter ( 1 ) ), cCMFit->GetParError ( 1 ), cCMFit->GetParameter ( 0 ) ), "l" );
                cLegend->AddEntry ( cNoCM, "CM = 0", "l" );
                cLegend->SetTextSize ( 0.05 );
                cLegend->Draw ( "same" );
            }
            else  cTmpNHits->Draw();

            // 2. 2D occupancy
            cCanvas->second->cd ( 2 );
            cTmpOccProfile->Draw ( "colz" );
            // 3. 1D combined occupancy
            cCanvas->second->cd ( 3 );
            cTmpCombinedOcc->Draw();

            if ( pFinal )
            {
                cUncorrHitProb->Draw ( "hist same" );
                TLegend* cLegend = new TLegend ( 0.13, 0.66, 0.38, 0.88, "" );
                cLegend->SetBorderSize ( 0 );
                cLegend->SetFillColor ( kWhite );
                cLegend->AddEntry ( cTmpCombinedOcc, "measured hit probability", "l" );
                cLegend->AddEntry ( cUncorrHitProb, "uncorrelated hit probability", "l" );
                cLegend->SetTextSize ( 0.05 );
                cLegend->Draw ( "same" );

                // 4. Correlation projection
                cCanvas->second->cd ( 4 );
                cCorrProjection->Draw();
            }

            cCanvas->second->Update();

        }
    }
}



bool CMTester::randHit ( float pProbability )
{
    float val = float ( rand() ) / RAND_MAX;

    if ( val < pProbability ) return true;
    else return false;
}

bool CMTester::isMasked ( Cbc* pCbc, int pChannel )
{
    auto cNoiseStripSet = fNoiseStripMap.find ( pCbc );

    if ( cNoiseStripSet == std::end ( fNoiseStripMap ) )
    {
        LOG (ERROR) << "Error: could not find the set of noisy strips for CBC " << int ( cNoiseStripSet->first->getCbcId() ) ;
        return false;
    }
    else
    {
        auto cNoiseStrip = cNoiseStripSet->second.find ( pChannel );

        if ( cNoiseStrip == std::end ( cNoiseStripSet->second ) ) return false;
        else return true;
    }
}

bool CMTester::isMasked ( int pGlobalChannel )
{
    uint32_t cCbcId;

    if ( pGlobalChannel < 254 ) cCbcId = 0;
    else if ( pGlobalChannel > 253 && pGlobalChannel < 508 ) cCbcId = 1;
    else if ( pGlobalChannel > 507 && pGlobalChannel < 762 ) cCbcId = 2;
    else if ( pGlobalChannel > 761 && pGlobalChannel < 1016 ) cCbcId = 3;
    else if ( pGlobalChannel > 1015 && pGlobalChannel < 1270 ) cCbcId = 4;
    else if ( pGlobalChannel > 1269 && pGlobalChannel < 1524 ) cCbcId = 5;
    else if ( pGlobalChannel > 1523 && pGlobalChannel < 1778 ) cCbcId = 6;
    else if ( pGlobalChannel > 1777 && pGlobalChannel < 2032 ) cCbcId = 5;
    else return true;

    for ( const auto& cNoiseStripSet : fNoiseStripMap )
    {
        if ( int ( cNoiseStripSet.first->getCbcId() ) == cCbcId )
        {
            auto cNoiseStrip = cNoiseStripSet.second.find ( pGlobalChannel - cCbcId * 254 );

            if ( cNoiseStrip == std::end ( cNoiseStripSet.second ) ) return false;
            else return true;
        }
        else return false;
    }
}




void CMTester::parseSettings()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;

    cSetting = fSettingsMap.find ( "doSimulate" );

    if ( cSetting != std::end ( fSettingsMap ) ) fDoSimulate = cSetting->second;
    else fDoSimulate = false;

    cSetting = fSettingsMap.find ( "SimOccupancy" );

    if ( cSetting != std::end ( fSettingsMap ) )  fSimOccupancy = cSetting->second;
    else fSimOccupancy = 50;

    LOG (INFO) << "Parsed the following settings:" ;
    LOG (INFO) << "	Nevents = " << fNevents ;
    LOG (INFO) << "	simulate = " << int ( fDoSimulate ) ;
    LOG (INFO) << "	sim. Occupancy (%) = " << int ( fSimOccupancy ) ;

}
