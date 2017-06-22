
#include "SignalScan.h"

void SignalScan::Initialize ()
{
    for ( auto& cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto& cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();
            fNCbc = cFe->getNCbc();
            TCanvas* ctmpCanvas = new TCanvas ( Form ( "c_online_canvas_fe%d", cFeId ), Form ( "FE%d  Online Canvas", cFeId ) );
            // ctmpCanvas->Divide( 2, 2 );
            fCanvasMap[cFe] = ctmpCanvas;

            // 1D Hist forlatency scan
            TString cName =  Form ( "h_module_thresholdScan_Fe%d", cFeId );
            TObject* cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH2F* cSignalHist = new TH2F ( cName, Form ( "Signal threshold vs channel FE%d; Channel # ; Threshold; # of Hits", cFeId ), fNCbc * NCHANNELS, -0.5, fNCbc * NCHANNELS - 0.5, 255, -.5,  255 - .5 );
            bookHistogram ( cFe, "module_signal", cSignalHist );
        }
    }

    // To read the blah-specific stuff
    parseSettings();
    LOG (INFO) << "Histograms & Settings initialised." ;
}


void SignalScan::ScanSignal (int pSignalScanLength)
{
    //add an std::ofstream here to hold the values of TDC, #hits, VCth
    std::ofstream output;
    std::string cFilename = fDirectoryName + "/SignalScanData.txt";
    output.open (cFilename, std::ios::out | std::ios::app);
    output << "TDC/I:nHits/I:nClusters/I:thresh/I:dataBitString/C:clusterString/C" ;

    // The step scan is +1 for hole mode
    int cVcthDirection = ( fHoleMode == 1 ) ? +1 : -1;

    // I need to read the current threshold here, save it in a variable, step back by fStepback, update the variable and then increment n times by fSignalScanStep
    // CBC VCth reader and writer

    // This is a bit ugly but since I program the same global value to both chips I guess it is ok...
    ThresholdVisitor cVisitor (fCbcInterface);
    this->accept (cVisitor);
    uint16_t cVCth = cVisitor.getThreshold();

    LOG (INFO) << "Programmed VCth value = " << +cVCth << " - falling back by " << fStepback << " to " << uint32_t (cVCth - cVcthDirection * fStepback) ;

    cVCth = uint16_t (cVCth - cVcthDirection * fStepback);
    cVisitor.setOption ('w');
    cVisitor.setThreshold (cVCth);
    this->accept (cVisitor);

    for (int i = 0; i < pSignalScanLength; i += fSignalScanStep )
    {
        LOG (INFO) << "Threshold: " << +cVCth << " - Iteration " << i << " - Taking " << fNevents ;

        // Take Data for all Modules
        for ( BeBoard* pBoard : fBoardVector )
        {
            // I need this to normalize the TDC values I get from the Strasbourg FW
            bool pStrasbourgFW = false;

            //if (pBoard->getBoardType() == BoardType::GLIB || pBoard->getBoardType() == BoardType::CTA) pStrasbourgFW = true;
            uint32_t cTotalEvents = 0;

            fBeBoardInterface->Start (pBoard);

            while (cTotalEvents < fNevents)
            {
                ReadData ( pBoard );

                const std::vector<Event*>& events = GetEvents ( pBoard );
                cTotalEvents += events.size();

                // Loop over Events from this Acquisition
                for ( auto& cEvent : events )
                {
                    for ( auto cFe : pBoard->fModuleVector )
                    {
                        TH2F* cSignalHist = static_cast<TH2F*> (getHist ( cFe, "module_signal") );
                        int cEventHits = 0;
                        int cEventClusters = 0;

                        std::string cDataString;
                        std::string cClusterDataString;

                        for ( auto cCbc : cFe->fCbcVector )
                        {
                            //now loop the channels for this particular event and increment a counter
                            for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
                            {
                                if ( cEvent->DataBit ( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
                                {
                                    cSignalHist->Fill (cCbc->getCbcId() *NCHANNELS + cId, cVCth );
                                    cEventHits++;
                                }
                            }

                            //append HexDataString to cDataString
                            cDataString += cEvent->DataHexString (cCbc->getFeId(), cCbc->getCbcId() );
                            cDataString += "-";

                            std::vector<Cluster> cClusters = cEvent->getClusters (cCbc->getFeId(), cCbc->getCbcId() );
                            cEventClusters += cClusters.size();

                            cClusterDataString += "-";

                            for (int i = 0; i < cClusters.size(); i++)
                            {
                                cClusterDataString += std::to_string (cClusters[i].fFirstStrip) + "."
                                                      + std::to_string (cClusters[i].fClusterWidth) + "^"
                                                      + std::to_string (cClusters[i].fSensor) + "-";
                            }

                        }

                        // This becomes an ofstream
                        output << +cEvent->GetTDC() << " "
                               << cEventHits << " "
                               << cEventClusters << " "
                               << +cVCth << " "
                               << cDataString << " "
                               << cClusterDataString ;
                    }
                }

                LOG (INFO) << "Recorded " << cTotalEvents << " Events" ;
                updateHists ( "module_signal", false );
            }

            fBeBoardInterface->Stop (pBoard);

        }

        // done counting hits for all FE's, now update the Histograms
        updateHists ( "module_signal", false );
        // now I need to increment the threshold by cVCth+fVcthDirecton*fSignalScanStep
        cVCth += cVcthDirection * fSignalScanStep;
        cVisitor.setOption ('w');
        cVisitor.setThreshold (cVCth);
        this->accept (cVisitor);

    }

    output.close();
}


//////////////////////////////////////          PRIVATE METHODS             //////////////////////////////////////



void SignalScan::updateHists ( std::string pHistName, bool pFinal )
{
    for ( auto& cCanvas : fCanvasMap )
    {
        // maybe need to declare temporary pointers outside the if condition?
        if ( pHistName == "module_signal" )
        {
            cCanvas.second->cd();
            TH2F* cTmpHist = dynamic_cast<TH2F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->DrawCopy ( "colz" );
            cCanvas.second->Update();
        }

    }
}


void SignalScan::parseSettings()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;

    cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )  fHoleMode = cSetting->second;
    else fHoleMode = 1;

    cSetting = fSettingsMap.find ( "PedestalStepBack" );

    if ( cSetting != std::end ( fSettingsMap ) )  fStepback = cSetting->second;
    else fStepback = 1;

    cSetting = fSettingsMap.find ( "SignalScanStep" );

    if ( cSetting != std::end ( fSettingsMap ) )  fSignalScanStep = cSetting->second;
    else fSignalScanStep = 1;

    LOG (INFO) << "Parsed the following settings:" ;
    LOG (INFO) << "	Nevents = " << fNevents ;
    LOG (INFO) << "	HoleMode = " << int ( fHoleMode ) ;
    LOG (INFO) << "	Step back from Pedestal = " << fStepback ;
    LOG (INFO) << "	SignalScanStep = " << fSignalScanStep ;
}
