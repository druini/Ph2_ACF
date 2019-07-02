#include "LatencyScan.h"
#include "../HWDescription/Cbc.h"

LatencyScan::LatencyScan() : Tool()
{}

LatencyScan::~LatencyScan() {}

void LatencyScan::Initialize (uint32_t pStartLatency, uint32_t pLatencyRange)
{
    for ( auto& cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        TH1F* cTriggerTDC = new TH1F ( Form ( "h_BeBoard_triggerTDC_Be%d", cBoardId ), Form ( "Trigger TDC BE%d; Trigger TDC; # of Hits", cBoardId ), fTDCBins, -0.5, fTDCBins-0.5);

        cTriggerTDC->SetFillColor ( 4 );
        cTriggerTDC->SetFillStyle ( 3001 );
        bookHistogram ( cBoard, "triggerTDC", cTriggerTDC );


        for ( auto& cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            TCanvas* ctmpCanvas = new TCanvas ( Form ( "c_online_canvas_fe%d", cFeId ), Form ( "FE%d  Online Canvas", cFeId ) );
            // ctmpCanvas->Divide( 2, 2 );
            fCanvasMap[cFe] = ctmpCanvas;

            fNCbc = cFe->getNChip();

            // 1D Hist forlatency scan
            TString cName =  Form ( "h_module_latency_Fe%d", cFeId );
            TObject* cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH1F* cLatHist = nullptr;
            
            cLatHist = new TH1F ( cName, Form ( "Latency FE%d; Latency; # of Hits", cFeId ), (pLatencyRange ) , pStartLatency - 0.5,  pStartLatency + (pLatencyRange ) - 0.5 );

            cLatHist->GetXaxis()->SetTitle ("Trigger Latency");
            cLatHist->SetFillColor ( 4 );
            cLatHist->SetFillStyle ( 3001 );
            bookHistogram ( cFe, "module_latency", cLatHist );

            cName =  Form ( "h_module_stub_latency_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH1F* cStubHist = new TH1F ( cName, Form ( "Stub Lateny FE%d; Stub Lateny; # of Stubs", cFeId ), pLatencyRange, pStartLatency, pStartLatency + pLatencyRange);
            cStubHist->SetMarkerStyle ( 2 );
            bookHistogram ( cFe, "module_stub_latency", cStubHist );

            cName =  Form ( "h_module_latency_2D_Fe%d", cFeId );
            TH2D* cLatencyScan2D =  new TH2D ( cName, Form ( "Latency FE%d; Stub Latency; L1 Latency; # of Events w/ no Hits and no Stubs", cFeId ), pLatencyRange  , pStartLatency - 0.5 ,  pStartLatency + (pLatencyRange ) - 0.5 , pLatencyRange  , pStartLatency - 0.5 ,  pStartLatency + (pLatencyRange ) - 0.5 );
            bookHistogram ( cFe, "module_latency_2D", cLatencyScan2D );
            
        }
    }

    parseSettings();
    LOG (INFO) << "Histograms and Settings initialised." ;
}

void LatencyScan::MeasureTriggerTDC()
{
    LOG (INFO) << "Measuring Trigger TDC ... " ;

    std::map<uint16_t, std::vector<uint16_t> > BeBoardTriggerTDCMap;
    // Take Data for all Modules
    for ( BeBoard* pBoard : fBoardVector )
    {
        // I need this to normalize the TDC values I get from the Strasbourg FW
        BeBoardTriggerTDCMap[pBoard->getBeId()] = std::vector<uint16_t>(fTDCBins,0);

        ReadNEvents ( pBoard, fNevents );

        const std::vector<Event*>& events = GetEvents ( pBoard );

        for (auto& cEvent : events)
        {

            uint8_t cTDCVal = cEvent->GetTDC();

            if (pBoard->getBoardType() == BoardType::D19C) {
                // Fix from Mykyta, ONLY the value of the cTDCShiftValue variable have to be changed, NEVER change the formula
                uint8_t cTDCShiftValue = 1;
                // don't touch next two lines (!!!)
                if ( cTDCVal < cTDCShiftValue ) cTDCVal += (fTDCBins-cTDCShiftValue);
                else cTDCVal -= cTDCShiftValue;
            }
            if (cTDCVal >= fTDCBins ) LOG (INFO) << "ERROR, TDC value not within expected range - normalized value is " << +cTDCVal << " - original Value was " << +cEvent->GetTDC() << "; not considering this Event!" <<  std::endl;
            else
            {
                ++BeBoardTriggerTDCMap[pBoard->getBeId()][cTDCVal];
            }
        }
    }

    for ( BeBoard* pBoard : fBoardVector ){
        TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( pBoard, "triggerTDC" ) );
        for(int tdcValue = 0; tdcValue<fTDCBins; ++tdcValue)
        {
            cTmpHist->SetBinContent(tdcValue+1, BeBoardTriggerTDCMap[pBoard->getBeId()][tdcValue]);
        }
    }

    return;
}


std::map<Module*, uint8_t> LatencyScan::ScanLatency ( uint8_t pStartLatency, uint8_t pLatencyRange)
{    

    LOG (INFO) << "Scanning Latency ... " ;
    uint32_t cIterationCount = 0;

    // //Fabio - clean BEGIN
    // MakeTestGroups (FrontEndType::CBC3);
    // setFWTestPulse();
    // setSystemTestPulse ( 200, 0, true, false );
    // //Fabio - clean END

    LatencyVisitor cVisitor (fReadoutChipInterface, 0);
 
    for ( BeBoard* pBoard : fBoardVector )
    {
        for ( uint16_t cLat = pStartLatency; cLat < pStartLatency + pLatencyRange; cLat++ )
        {
            //  Set a Latency Value on all FEs
            cVisitor.setLatency (  cLat );
            this->accept ( cVisitor );

            ReadNEvents ( pBoard, fNevents );

            const std::vector<Event*>& events = GetEvents ( pBoard );
            countHitsLat ( pBoard, events, "module_latency", cLat, pStartLatency);
            // done counting hits for all FE's, now update the Histograms
            updateHists ( "module_latency", false );

        }

        // done counting hits for all FE's, now update the Histograms
        updateHists ( "module_latency", false );
        cIterationCount++;
    }


    // analyze the Histograms
    std::map<Module*, uint8_t> cLatencyMap;

    updateHists ( "module_latency", true );

    return cLatencyMap;
}

std::map<Module*, uint8_t> LatencyScan::ScanStubLatency ( uint8_t pStartLatency, uint8_t pLatencyRange )
{
    // This is not super clean but should work
    // Take the default VCth which should correspond to the pedestal and add 8 depending on the mode to exclude noise
    // ThresholdVisitor in read mode
    //ThresholdVisitor cThresholdVisitor (fReadoutChipInterface);
    //this->accept (cThresholdVisitor);
    //uint16_t cVcth = cThresholdVisitor.getThreshold();

    //int cVcthStep = ( fHoleMode == 1 ) ? +10 : -10;
    //LOG (INFO) << "VCth value from config file is: " << +cVcth << " ;  changing by " << cVcthStep << "  to " << + ( cVcth + cVcthStep ) << " supress noise hits for crude latency scan!" ;
    //cVcth += cVcthStep;

    ////  Set that VCth Value on all FEs
    //cThresholdVisitor.setOption ('w');
    //cThresholdVisitor.setThreshold (cVcth);
    //this->accept (cThresholdVisitor);

    // set test pulse (if needed)
    //setFWTestPulse();
    //setSystemTestPulse ( fTestPulseAmplitude, 0, true, fHoleMode );

    // Now the actual scan
    LOG (INFO) << "Scanning Stub Latency ... " ;

    for ( BeBoard* pBoard : fBoardVector )
    {
        for ( uint8_t cLat = pStartLatency; cLat < pStartLatency + pLatencyRange; cLat++ )
        {
            uint32_t cN = 0;
            int cNStubs = 0;
            uint32_t cNevents = 0 ;
        // Take Data for all Modules
            //here set the stub latency
            for (auto cReg : getStubLatencyName (pBoard->getBoardType() ) )
                fBeBoardInterface->WriteBoardReg (pBoard, cReg, cLat);

            fBeBoardInterface->Start(pBoard);
            do
            {
                uint32_t cNeventsReadBack = ReadData( pBoard );
                if( cNeventsReadBack == 0 )
                {
                    LOG (INFO) << BOLDRED << "..... Read back " << +cNeventsReadBack << " events!! Why?!" << RESET ;
                    continue;
                }

                const std::vector<Event*>& events = GetEvents ( pBoard );
                cNevents += events.size(); 

                // Loop over Events from this Acquisition
                for ( auto& cEvent : events )
                {
                    for ( auto cFe : pBoard->fModuleVector )
                        cNStubs += countStubs ( cFe, cEvent, "module_stub_latency", cLat );
                }

            }while( cNevents < fNevents );
            fBeBoardInterface->Stop(pBoard);
            LOG (INFO) << "Stub Latency " << +cLat << " Stubs " << cNStubs  << " Events " << cN ;

            //ReadNEvents ( pBoard, fNevents );
            //const std::vector<Event*>& events = GetEvents ( pBoard );

            // if(cN <3 ) LOG(INFO) << *cEvent ;

            // Loop over Events from this Acquisition
            //for ( auto& cEvent : events )
            //{
            //    for ( auto cFe : pBoard->fModuleVector )
            //        cNStubs += countStubs ( cFe, cEvent, "module_stub_latency", cLat );
            //
            //    cN++;
            //}
            //LOG (INFO) << "Stub Latency " << +cLat << " Stubs " << cNStubs  << " Events " << cN ;

        }

        // done counting hits for all FE's, now update the Histograms
        updateHists ( "module_stub_latency", false );
    }

    // analyze the Histograms
    std::map<Module*, uint8_t> cStubLatencyMap;

    LOG (INFO) << "Identified the Latency with the maximum number of Stubs at: " ;

    for ( auto cFe : fModuleHistMap )
    {
        TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( cFe.first, "module_stub_latency" ) );
        uint8_t cStubLatency =  static_cast<uint8_t> ( cTmpHist->GetMaximumBin() - 1 );
        cStubLatencyMap[cFe.first] = cStubLatency;

        //BeBoardRegWriter cLatWriter ( fBeBoardInterface, "", 0 );

        //if ( cFe.first->getFeId() == 0 ) cLatWriter.setRegister ( "cbc_stubdata_latency_adjust_fe1", cStubLatency );
        //else if ( cFe.first->getFeId() == 1 ) cLatWriter.setRegister ( "cbc_stubdata_latency_adjust_fe2", cStubLatency );

        //this->accept ( cLatWriter );

        LOG (INFO) << "Stub Latency FE " << +cFe.first->getModuleId()  << ": " << +cStubLatency << " clock cycles!" ;
    }

    return cStubLatencyMap;
}

void LatencyScan::ScanLatency2D(uint8_t pStartLatency, uint8_t pLatencyRange)
{
    
    LatencyVisitor cVisitor (fReadoutChipInterface, 0);
    int cNSteps = 0 ; 
    for ( uint16_t cLatency = pStartLatency; cLatency < pStartLatency + pLatencyRange; cLatency++ )
    {
        //  Set a Latency Value on all FEs
        cVisitor.setLatency (  cLatency );
        this->accept ( cVisitor );

        // maximum stub latency can only be L1 latency ... 
        for ( uint8_t cStubLatency = 0; cStubLatency < cLatency ; cStubLatency++ )
        {
            // Take Data for all Modules
            for ( BeBoard* pBoard : fBoardVector )
            {
                // set a stub latency value on all FEs 
                for (auto cReg : getStubLatencyName (pBoard->getBoardType() ) )
                    fBeBoardInterface->WriteBoardReg (pBoard, cReg, cStubLatency);

                // I need this to normalize the TDC values I get from the Strasbourg FW
                int cNevents = 0 ;  
                int cNEvents_wHit = 0 ; 
                int cNEvents_wStub = 0 ; 
                int cNEvents_wBoth = 0; 
                fBeBoardInterface->Start(pBoard);
                do
                {
                    uint32_t cNeventsReadBack = ReadData( pBoard );
                    if( cNeventsReadBack == 0 )
                    {
                        LOG (INFO) << BOLDRED << "..... Read back " << +cNeventsReadBack << " events!! Why?!" << RESET ;
                        continue;
                    }

                    const std::vector<Event*>& events = GetEvents ( pBoard );
                    cNevents += events.size(); 

                    for ( auto cFe : pBoard->fModuleVector )
                    {
                        for( auto cEvent : events )
                        {
                            bool cHitFound = false; 
                            bool cStubFound = false; 
                            //now loop the channels for this particular event and increment a counter
                            for ( auto cCbc : cFe->fReadoutChipVector )
                            {
                                int cHitCounter = cEvent->GetNHits (cCbc->getFeId(), cCbc->getChipId() );
                                std::vector<Stub> cStubs = cEvent->StubVector (cCbc->getFeId(), cCbc->getChipId());
                                int cStubCounter = cStubs.size(); 

                                if( cHitCounter == 0 )
                                {
                                
                                }

                                if( cHitCounter > 0 )
                                    cHitFound = true;

                                if( cStubCounter > 0 )
                                    cStubFound = true;
                            }
                            cNEvents_wHit += cHitFound ? 1 : 0 ; 
                            cNEvents_wStub += cStubFound ? 1 : 0 ; 
                            cNEvents_wBoth += ( cHitFound && cStubFound ) ? 1 : 0 ; 
                        }
                        TH2D* cTmpHist2D = dynamic_cast<TH2D*> ( getHist ( cFe, "module_latency_2D" ) );
                        cTmpHist2D->Fill( cStubLatency , cLatency , cNEvents_wBoth ); 
                        cTmpHist2D->GetZaxis()->SetRangeUser(1,cNevents);
                        updateHists("module_latency_2D", false );
                    }
                    
                }while( cNevents < fNevents );
                fBeBoardInterface->Stop(pBoard);

                if( cNSteps % 10 == 0 )
                {
                    LOG (INFO) << BOLDBLUE << "For an L1 latency of " << +cLatency << " and a stub latency of " << +cStubLatency << " - found : " << RESET ;
                    LOG (INFO) << BOLDBLUE << "\t\t " << cNEvents_wHit << "/" << cNevents << " events with a hit. " << RESET ;
                    LOG (INFO) << BOLDBLUE << "\t\t " << cNEvents_wStub << "/" << cNevents << " events with a stub. " << RESET ;
                    LOG (INFO) << BOLDBLUE << "\t\t " << cNEvents_wBoth << "/" << cNevents << " events with both a hit and a stub. " << RESET ;
                }
                
            }
            cNSteps++; 
        }
    }

    // now display a message to the user to let them know what the optimal latencies are for each FE
    for ( BeBoard* pBoard : fBoardVector )
    {
        for ( auto cFe : pBoard->fModuleVector )
        {
            std::pair<uint8_t, uint16_t> cOptimalLatencies;
            cOptimalLatencies.first = 0 ;
            cOptimalLatencies.second = 0; 
            int cMaxNEvents_wBoth = 0 ; 
     
            TH2D* cTmpHist2D = dynamic_cast<TH2D*> ( getHist ( cFe, "module_latency_2D" ) );

            for( int cBinX = 1 ; cBinX < cTmpHist2D->GetXaxis()->GetNbins() ; cBinX ++ )
            {
                for( int cBinY = 1 ; cBinY < cTmpHist2D->GetYaxis()->GetNbins() ; cBinY ++ )
                {
                    int cBinContent = cTmpHist2D->GetBinContent( cBinX , cBinY ); 
                    if( cBinContent >= cMaxNEvents_wBoth )
                    {
                        cOptimalLatencies.first = (uint8_t)cTmpHist2D->GetXaxis()->GetBinCenter(cBinX);
                        cOptimalLatencies.second = (uint16_t)cTmpHist2D->GetYaxis()->GetBinCenter(cBinY);
                        cMaxNEvents_wBoth = cBinContent;
                    }
                }
            }
            
            // if interested in the tdc phase then do the latency scan (trigger only) with TDC on
            // scan 4 clock cycles around optimal latency
            // something fishy here ... needs to be fixed (binning on TDC histogram is weird)
            // if( !pNoTdc )
            // {
            //     // I think I need to re-configure the histogram w/ TDC
            //     TString cName =  Form ( "h_module_latency_Fe%d", cFe->getFeId() );
            //     TObject* cObj = gROOT->FindObject ( cName );
            //     if ( cObj ) delete cObj;

            //     int cLatencyStart = std::max( 0, (int)cOptimalLatencies.second - 2 );
            //     int cLatencyRange = 4 ; 
            //     TH1F* cLatHist = new TH1F ( cName, Form ( "Latency FE%d; Latency; # of Hits", cFe->getFeId() ), (cLatencyRange) * fTDCBins, cLatencyStart,  cLatencyStart + (cLatencyRange )  * fTDCBins );
            //     //modify the axis labels
            //     uint32_t pLabel = cLatencyStart ;

            //     for (uint32_t cLatency = cLatencyStart; cLatency < cLatencyStart + cLatencyRange; ++cLatency)
            //     {
            //         for (uint32_t cPhase = 0; cPhase < fTDCBins; ++cPhase)
            //         {
            //             int cBin = 1 + cLatHist->GetXaxis()->FindBin( convertLatencyPhase (cLatencyStart, cLatency, cPhase) );
            //             if( cBin == 0 )
            //                 LOG(INFO) << BOLDRED << "!!!! " << +cLatency << " [latency], " << cLatencyStart << " [start latency] " << +cPhase << " [TDC phase] : converted latency " << convertLatencyPhase (cLatencyStart, cLatency, cPhase) <<  RESET ; 
            //             cLatHist->GetXaxis()->SetBinLabel (cBin, Form ("%d+%d", cLatency, cPhase) );
            //         }
            //     }
                
            //     ScanLatency ( cLatencyStart , cLatencyRange , false );
            // }

            LOG (INFO) << BOLDRED << "************************************************************************************" << RESET ;
            LOG (INFO) << BOLDRED << "For FE" << +cFe->getFeId() << " found optimal latencies to be : " << RESET ; 
            LOG (INFO) << BOLDRED << "........ Stub Latency of " << +cOptimalLatencies.first << " and a Trigger Latency of " << +cOptimalLatencies.second << RESET; 
            LOG (INFO) << BOLDRED << "************************************************************************************" << RESET ;
        }
    }

}

//////////////////////////////////////          PRIVATE METHODS             //////////////////////////////////////

// int LatencyScan::countHits ( BeBoard* pBoard,  const std::vector<Event*> pEventVec, std::map<uint8_t, std::map<uint8_t,uint32_t> > numberOfHits, std::map<uint8_t,uint32_t> > numberOfStubs)
// {

// }

int LatencyScan::countHitsLat ( BeBoard* pBoard,  const std::vector<Event*> pEventVec, std::string pHistName, uint16_t pParameter, uint32_t pStartLatency)
{
    
    uint32_t cTotalHits = 0;

    for ( auto cFe : pBoard->fModuleVector )
    {
        uint32_t cHitSum = 0;
        //  get histogram to fill
        TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( cFe, pHistName ) );
        for (auto& cEvent : pEventVec)
        {
            //first, reset the hit counter - I need separate counters for each event
            int cHitCounter = 0;
            
            for ( auto cCbc : cFe->fReadoutChipVector )
            {
                //now loop the channels for this particular event and increment a counter
                cHitCounter += cEvent->GetNHits (cCbc->getFeId(), cCbc->getChipId() );
            }

            //now I have the number of hits in this particular event for all CBCs and the TDC value
            
            cTmpHist->Fill (pParameter, cHitCounter);
            
            //if (cHitCounter != 0 ) std::cout << "Found " << cHitCounter << " Hits in this event!" << std::endl;

            //GA: old, potentially buggy code
            //cTmpHist->Fill (cBin, cHitCounter);

            cHitSum += cHitCounter;
        }

        LOG (INFO) << "FE: " << +cFe->getFeId() << "; Latency " << +pParameter << " clock cycles; Hits " << cHitSum  << "; Events " << fNevents ;
        cTotalHits += cHitSum;
    }

    return cTotalHits;
}

int LatencyScan::countStubs ( Module* pFe,  const Event* pEvent, std::string pHistName, uint8_t pParameter )
{
    // loop over Modules & Cbcs and count hits separately
    int cStubCounter = 0;

    //  get histogram to fill
    TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( pFe, pHistName ) );

    for ( auto cCbc : pFe->fReadoutChipVector )
    {
        if ( pEvent->StubBit ( cCbc->getFeId(), cCbc->getChipId() ) )
            cStubCounter += pEvent->StubVector ( cCbc->getFeId(), cCbc->getChipId() ).size();
    }

    int cBin = cTmpHist->FindBin (pParameter);
    float cBinContent = cTmpHist->GetBinContent (cBin);
    cBinContent += cStubCounter;
    cTmpHist->SetBinContent (cBin, cBinContent);

    //if (cStubCounter != 0) std::cout << "Found " << cStubCounter << " Stubs in this event" << std::endl;

    //GA, old, potentially buggy code)
    //cTmpHist->Fill ( pParameter );

    return cStubCounter;
}

void LatencyScan::updateHists ( std::string pHistName, bool pFinal )
{
    for ( auto& cCanvas : fCanvasMap )
    {
        // maybe need to declare temporary pointers outside the if condition?
        if ( pHistName == "module_latency" )
        {
            cCanvas.second->cd();
            TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->DrawCopy ( );
            cCanvas.second->Update();

        }
        else if ( pHistName == "module_stub_latency" )
        {
            cCanvas.second->cd();
            TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->DrawCopy ( );
            cCanvas.second->Update();
        }
        else if (pHistName == "module_latency_2D" )
        {
            cCanvas.second->cd();
            TH2D* cTmpHist = dynamic_cast<TH2D*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->DrawCopy ( "colz" );
            cTmpHist->SetStats(0);
            cCanvas.second->Update();
        }

    }

    this->HttpServerProcess();
}


void LatencyScan::parseSettings()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;

    cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )  fHoleMode = cSetting->second;
    else fHoleMode = 1;

         cSetting = fSettingsMap.find ( "TriggerSource" );
         if ( cSetting != std::end ( fSettingsMap ) ) trigSource = cSetting->second;
         LOG (INFO)  <<int (trigSource);

    //cSetting = fSettingsMap.find ( "TestPulsePotentiometer" );
    //fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x7F;

    LOG (INFO) << "Parsed the following settings:" ;
    LOG (INFO) << "	Nevents = " << fNevents ;
    LOG (INFO) << "	HoleMode = " << int ( fHoleMode ) ;
    //LOG (INFO) << "   TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
}
void LatencyScan::writeObjects()
{
    this->SaveResults();
    // just use auto iterators to write everything to disk
    // this is the old method before Tool class was cool
    fResultFile->cd();

    // Save canvasses too
    //fNoiseCanvas->Write ( fNoiseCanvas->GetName(), TObject::kOverwrite );
    //fPedestalCanvas->Write ( fPedestalCanvas->GetName(), TObject::kOverwrite );
    //fFeSummaryCanvas->Write ( fFeSummaryCanvas->GetName(), TObject::kOverwrite );
    fResultFile->Flush();
}
