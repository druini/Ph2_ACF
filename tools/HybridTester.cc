#include "HybridTester.h"
#include <fstream>
#include <ctime>

// fill the Histograms, count the hits and increment Vcth

struct HistogramFiller  : public HwDescriptionVisitor
{
    TH1F* fBotHist;
    TH1F* fTopHist;
    const Event* fEvent;

    HistogramFiller ( TH1F* pBotHist, TH1F* pTopHist, const Event* pEvent ) : fBotHist ( pBotHist ), fTopHist ( pTopHist ), fEvent ( pEvent ) {}

    void visit ( Cbc& pCbc )
    {
        std::vector<bool> cDataBitVector = fEvent->DataBitVector ( pCbc.getFeId(), pCbc.getCbcId() );

        for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
        {
            if ( cDataBitVector.at ( cId ) )
            {
                uint32_t globalChannel = ( pCbc.getCbcId() * 254 ) + cId;

                //              std::cout << "Channel " << globalChannel << " VCth " << int(pCbc.getReg( "VCth" )) << std::endl;
                // find out why histograms are not filling!
                if ( globalChannel % 2 == 0 )
                    fBotHist->Fill ( globalChannel / 2 );
                else
                    fTopHist->Fill ( ( globalChannel - 1 ) / 2 );

            }
        }
    }
};


void HybridTester::InitializeHists()
{
    TString cFrontName ( "fHistTop" );
    fHistTop = ( TH1F* ) ( gROOT->FindObject ( cFrontName ) );

    if ( fHistTop ) delete fHistTop;

    fHistTop = new TH1F ( cFrontName, "Front Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 253 ) + 1, -0.5, ( fNCbc / 2 * 253 ) + .5 );
    fHistTop->SetFillColor ( 4 );
    fHistTop->SetFillStyle ( 3001 );

    TString cBackName ( "fHistBottom" );
    fHistBottom = ( TH1F* ) ( gROOT->FindObject ( cBackName ) );

    if ( fHistBottom ) delete fHistBottom;

    fHistBottom = new TH1F ( cBackName, "Back Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 253 ) + 1 , -0.5, ( fNCbc / 2 * 253 ) + .5 );
    fHistBottom->SetFillColor ( 4 );
    fHistBottom->SetFillStyle ( 3001 );

    TString cFrontNameMerged ( "fHistTopMerged" );
    fHistTopMerged = ( TH1F* ) ( gROOT->FindObject ( cFrontNameMerged ) );

    if ( fHistTopMerged ) delete fHistTopMerged;

    fHistTopMerged = new TH1F ( cFrontNameMerged, "Front Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 253 ) + 1, -0.5, ( fNCbc / 2 * 253 ) + .5 );
    fHistTopMerged->SetFillColor ( 4 );
    fHistTopMerged->SetFillStyle ( 3001 );

    TString cBackNameMerged ( "fHistBottomMerged" );
    fHistBottomMerged = ( TH1F* ) ( gROOT->FindObject ( cBackNameMerged ) );

    if ( fHistBottomMerged ) delete fHistBottomMerged;

    fHistBottomMerged = new TH1F ( cBackNameMerged, "Back Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 253 ) + 1 , -0.5, ( fNCbc / 2 * 253 ) + .5 );
    fHistBottomMerged->SetFillColor ( 4 );
    fHistBottomMerged->SetFillStyle ( 3001 );

    // Now the Histograms for SCurves
    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {

                uint32_t cCbcId = cCbc->getCbcId();

                TString cName = Form ( "SCurve_Fe%d_Cbc%d", cFeId, cCbcId );
                TObject* cObject = static_cast<TObject*> ( gROOT->FindObject ( cName ) );

                if ( cObject ) delete cObject;

                TH1F* cTmpScurve = new TH1F ( cName, Form ( "Noise Occupancy Cbc%d; VCth; Counts", cCbcId ), 255, 0, 255 );
                cTmpScurve->SetMarkerStyle ( 8 );
                fSCurveMap[cCbc] = cTmpScurve;

                cName = Form ( "SCurveFit_Fe%d_Cbc%d", cFeId, cCbcId );
                cObject = static_cast<TObject*> ( gROOT->FindObject ( cName ) );

                if ( cObject ) delete cObject;

                TF1* cTmpFit = new TF1 ( cName, MyErf, 0, 255, 2 );
                fFitMap[cCbc] = cTmpFit;
            }
        }
    }
}

void HybridTester::InitialiseSettings()
{
    auto cSetting = fSettingsMap.find ( "Threshold_NSigmas" );
    fSigmas = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 4;
    cSetting = fSettingsMap.find ( "Nevents" );
    fTotalEvents = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 999;
    cSetting = fSettingsMap.find ( "HoleMode" );
    fHoleMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : true;


    // std::cout << "Read the following Settings: " << std::endl;
    // std::cout << "Hole Mode: " << fHoleMode << std::endl << "NEvents: " << fTotalEvents << std::endl << "NSigmas: " << fSigmas << std::endl;
}

void HybridTester::Initialize ( bool pThresholdScan )
{
    fThresholdScan = pThresholdScan;
    gStyle->SetOptStat ( 000000 );
    //gStyle->SetTitleOffset( 1.3, "Y" );
    //  special Visito class to count objects
    Counter cCbcCounter;
    accept ( cCbcCounter );
    fNCbc = cCbcCounter.getNCbc();

    fDataCanvas = new TCanvas ( "fDataCanvas", "SingleStripEfficiency", 1200, 800 );
    fDataCanvas->Divide ( 2 );

    if ( fThresholdScan )
    {
        fSCurveCanvas = new TCanvas ( "fSCurveCanvas", "Noise Occupancy as function of VCth" );
        fSCurveCanvas->Divide ( fNCbc );
    }

    InitializeHists();
    InitialiseSettings();
}


void HybridTester::ScanThreshold()
{
    std::cout << "Scanning noise Occupancy to find threshold for test with external source ... " << std::endl;

    // Necessary variables
    uint32_t cEventsperVcth = 10;
    bool cNonZero = false;
    bool cAllOne = false;
    bool cSlopeZero = false;
    uint32_t cAllOneCounter = 0;
    uint32_t cSlopeZeroCounter = 0;
    uint32_t cOldHitCounter = 0;
    uint8_t  cDoubleVcth;
    uint8_t cVcth = ( fHoleMode ) ?  0xFF :  0x00;
    int cStep = ( fHoleMode ) ? -10 : 10;

    // Adaptive VCth loop
    while ( 0x00 <= cVcth && cVcth <= 0xFF )
    {
        if ( cAllOne ) break;

        if ( cVcth == cDoubleVcth )
        {
            cVcth +=  cStep;
            continue;
        }

        // Set current Vcth value on all Cbc's
        CbcRegWriter cWriter ( fCbcInterface, "VCth", cVcth );
        accept ( cWriter );
        uint32_t cN = 1;
        uint32_t cNthAcq = 0;
        uint32_t cHitCounter = 0;

        // maybe restrict to pBoard? instead of looping?
        if ( cAllOne ) break;

        for ( BeBoard* pBoard : fBoardVector )
        {
            fBeBoardInterface->ReadNEvents ( pBoard, cEventsperVcth );
            const std::vector<Event*>& events = fBeBoardInterface->GetEvents ( pBoard );

            // Loop over Events from this Acquisition
            for ( auto& cEvent : events )
            {
                // loop over Modules & Cbcs and count hits separately
                cHitCounter += fillSCurves ( pBoard,  cEvent, cVcth );
                cN++;
            }

            cNthAcq++;
            // std::cout << +cVcth << " " << cHitCounter << std::endl;
            // Draw the thing after each point
            updateSCurveCanvas ( pBoard );

            // check if the hitcounter is all ones

            if ( cNonZero == false && cHitCounter != 0 )
            {
                cDoubleVcth = cVcth;
                cNonZero = true;
                cVcth -= 2 * cStep;
                cStep /= 10;
                continue;
            }

            if ( cNonZero && cHitCounter != 0 )
            {
                // check if all Cbcs have reached full occupancy
                if ( cHitCounter > 0.95 * cEventsperVcth * fNCbc * NCHANNELS ) cAllOneCounter++;

                // add a second check if the global SCurve slope is 0 for 10 consecutive Vcth values
                // if ( fabs( cHitCounter - cOldHitCounter ) < 10 && cHitCounter != 0 ) cSlopeZeroCounter++;
            }

            if ( cAllOneCounter >= 10 ) cAllOne = true;

            // if ( cSlopeZeroCounter >= 10 ) cSlopeZero = true;

            if ( cAllOne )
            {
                std::cout << "All strips firing -- ending the scan at VCth " << +cVcth << std::endl;
                break;
            }

            // else if ( cSlopeZero )
            // {
            //   std::cout << "Slope of SCurve 0 -- ending the scan at VCth " << +cVcth << std::endl;
            //  break;
            // }

            cOldHitCounter = cHitCounter;
            cVcth += cStep;
        }
    }

    // Fit and save the SCurve & Fit - extract the right threshold
    // TODO
    processSCurves ( cEventsperVcth );
}

void HybridTester::processSCurves ( uint32_t pEventsperVcth )
{
    for ( auto cScurve : fSCurveMap )
    {
        fSCurveCanvas->cd ( cScurve.first->getCbcId() + 1 );

        cScurve.second->Scale ( 1 / double_t ( pEventsperVcth * NCHANNELS ) );
        cScurve.second->Draw ( "P" );
        // Write to file
        cScurve.second->Write ( cScurve.second->GetName(), TObject::kOverwrite );

        // Estimate parameters for the Fit
        double cFirstNon0 ( 0 );
        double cFirst1 ( 0 );

        // Not Hole Mode
        if ( !fHoleMode )
        {
            for ( Int_t cBin = 1; cBin <= cScurve.second->GetNbinsX(); cBin++ )
            {
                double cContent = cScurve.second->GetBinContent ( cBin );

                if ( !cFirstNon0 )
                {
                    if ( cContent ) cFirstNon0 = cScurve.second->GetBinCenter ( cBin );
                }
                else if ( cContent == 1 )
                {
                    cFirst1 = cScurve.second->GetBinCenter ( cBin );
                    break;
                }
            }
        }
        // Hole mode
        else
        {
            for ( Int_t cBin = cScurve.second->GetNbinsX(); cBin >= 1; cBin-- )
            {
                double cContent = cScurve.second->GetBinContent ( cBin );

                if ( !cFirstNon0 )
                {
                    if ( cContent ) cFirstNon0 = cScurve.second->GetBinCenter ( cBin );
                }
                else if ( cContent == 1 )
                {
                    cFirst1 = cScurve.second->GetBinCenter ( cBin );
                    break;
                }
            }
        }

        // Get rough midpoint & width
        double cMid = ( cFirst1 + cFirstNon0 ) * 0.5;
        double cWidth = ( cFirst1 - cFirstNon0 ) * 0.5;

        // find the corresponding fit
        auto cFit = fFitMap.find ( cScurve.first );

        if ( cFit == std::end ( fFitMap ) ) std::cout << "Error: could not find Fit for Cbc " << int ( cScurve.first->getCbcId() ) << std::endl;
        else
        {
            // Fit
            cFit->second->SetParameter ( 0, cMid );
            cFit->second->SetParameter ( 1, cWidth );

            cScurve.second->Fit ( cFit->second, "RNQ+" );
            cFit->second->Draw ( "same" );

            // Write to File
            cFit->second->Write ( cFit->second->GetName(), TObject::kOverwrite );

            // TODO
            // Set new VCth - for the moment each Cbc gets his own Vcth - I shold add a mechanism to take one that works for all!
            double_t pedestal = cFit->second->GetParameter ( 0 );
            double_t noise = cFit->second->GetParameter ( 1 );

            uint8_t cThreshold = ceil ( pedestal + fSigmas * fabs ( noise ) );

            std::cout << "Identified a noise Occupancy of 50% at VCth " << static_cast<int> ( pedestal ) << " -- increasing by " << fSigmas <<  " sigmas (=" << fabs ( noise ) << ") to " << +cThreshold << " for Cbc " << int ( cScurve.first->getCbcId() ) << std::endl;

            TLine* cLine = new TLine ( cThreshold, 0, cThreshold, 1 );
            cLine->SetLineWidth ( 3 );
            cLine->SetLineColor ( 2 );
            cLine->Draw ( "same" );

            fCbcInterface->WriteCbcReg ( cScurve.first, "VCth", cThreshold );
        }

    }

    fSCurveCanvas->Update();
#ifdef __HTTP__
    fHttpServer->ProcessRequests();
#endif

    // Write and Save the Canvas as PDF
    fSCurveCanvas->Write ( fSCurveCanvas->GetName(), TObject::kOverwrite );
    std::string cPdfName = fDirectoryName + "/NoiseOccupancy.pdf";
    fSCurveCanvas->SaveAs ( cPdfName.c_str() );
}

uint32_t HybridTester::fillSCurves ( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue )
{
    uint32_t cHitCounter = 0;

    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cCbc : cFe->fCbcVector )
        {
            auto cScurve = fSCurveMap.find ( cCbc );

            if ( cScurve == fSCurveMap.end() ) std::cout << "Error: could not find an Scurve object for Cbc " << int ( cCbc->getCbcId() ) << std::endl;
            else
            {
                for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
                {
                    if ( pEvent->DataBit ( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
                    {
                        cScurve->second->Fill ( pValue );
                        cHitCounter++;
                    }
                }
            }
        }
    }

    return cHitCounter;
}

void HybridTester::updateSCurveCanvas ( BeBoard* pBoard )
{

    // Here iterate over the fScurveMap and update
    // fSCurveCanvas->cd();
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cCbc : cFe->fCbcVector )
        {
            uint32_t cCbcId = cCbc->getCbcId();
            auto cScurve = fSCurveMap.find ( cCbc );

            if ( cScurve == fSCurveMap.end() ) std::cout << "Error: could not find an Scurve object for Cbc " << int ( cCbc->getCbcId() ) << std::endl;
            else
            {
                fSCurveCanvas->cd ( cCbcId + 1 );
                cScurve->second->Draw ( "P" );
            }
        }
    }

    fSCurveCanvas->Update();
#ifdef __HTTP__
    fHttpServer->ProcessRequests();
#endif
}



void HybridTester::TestRegisters()
{
    // This method has to be followed by a configure call, otherwise the CBCs will be in an undefined state
    struct RegTester : public HwDescriptionVisitor
    {
        CbcInterface* fInterface;
        std::map<uint32_t, std::set<std::string>> fBadRegisters;
        RegTester ( CbcInterface* pInterface ) : fInterface ( pInterface )
        {
            std::set<std::string> tempset;
            fBadRegisters[0] = tempset;
            fBadRegisters[1] = tempset;
        }

        void visit ( Cbc& pCbc )
        {
            uint8_t cFirstBitPattern = 0xAA;
            uint8_t cSecondBitPattern = 0x55;

            CbcRegMap cMap = pCbc.getRegMap();

            for ( const auto& cReg : cMap )
            {
                if ( !fInterface->WriteCbcReg ( &pCbc, cReg.first, cFirstBitPattern, true ) ) fBadRegisters[pCbc.getCbcId()] .insert ( cReg.first );

                if ( !fInterface->WriteCbcReg ( &pCbc, cReg.first, cSecondBitPattern, true ) ) fBadRegisters[pCbc.getCbcId()] .insert ( cReg.first );
            }
        }

        void dumpResult ( std::string fDirectoryName )
        {
            std::ofstream report ( fDirectoryName + "/registers_test.txt" ); // Creates a file in the current directory
            report << "Testing Cbc Registers one-by-one with complimentary bit-patterns (0xAA, 0x55)" << std::endl;

            for ( const auto& cCbc : fBadRegisters )
            {
                report << "Malfunctioning Registers on Cbc " << cCbc.first << " : " << std::endl;

                for ( const auto& cReg : cCbc.second ) report << cReg << std::endl;

            }

            report.close();
            std::cout << "Channels diagnosis report written to: " + fDirectoryName + "/registers_test.txt" << std::endl;
        }
    };

    // This should probably be done in the top level application but there I do not have access to the settings map
    time_t start_time = time (0);
    char* start = ctime (&start_time);
    std::cout << "start: " << start << std::endl;
    std::cout << std::endl << "Running registers testing tool ... " << std::endl;
    RegTester cRegTester ( fCbcInterface );
    accept ( cRegTester );
    cRegTester.dumpResult ( fDirectoryName );
    std::cout << "Done testing registers, re-configuring to calibrated state!" << std::endl;
    start_time = time (0);
    char* stop = ctime (&start_time);
    std::cout << "stop: " << stop << std::endl;
    ConfigureHw();
}

void HybridTester::Measure()
{
    std::cout << "Mesuring Efficiency per Strip ... " << std::endl;
    std::cout << "Taking data with " << fTotalEvents << " Events!" << std::endl;

    CbcRegReader cReader ( fCbcInterface, "VCth" );
    accept ( cReader );
    fHistTop->GetYaxis()->SetRangeUser ( 0, fTotalEvents );
    fHistBottom->GetYaxis()->SetRangeUser ( 0, fTotalEvents );

    for ( BeBoard* pBoard : fBoardVector )
    {
        uint32_t cN = 1;
        uint32_t cNthAcq = 0;

        fBeBoardInterface->Start ( pBoard );

        fBeBoardInterface->ReadNEvents ( pBoard, fTotalEvents );
        const std::vector<Event*>& events = fBeBoardInterface->GetEvents ( pBoard );

        // Loop over Events from this Acquisition
        for ( auto& cEvent : events )
        {
            HistogramFiller cFiller ( fHistBottom, fHistTop, cEvent );
            pBoard->accept ( cFiller );

            if ( cN % 100 == 0 )
                UpdateHists();

            cN++;
        }

        cNthAcq++;

    }

    fHistTop->Scale ( 100 / double_t ( fTotalEvents ) );
    fHistTop->GetYaxis()->SetRangeUser ( 0, 100 );
    fHistBottom->Scale ( 100 / double_t ( fTotalEvents ) );
    fHistBottom->GetYaxis()->SetRangeUser ( 0, 100 );
    UpdateHists();

    std::cout << "Mean occupancy at the Top side: " << fHistTop->Integral() / (double) (fNCbc * 127) << std::endl;
    std::cout << "Mean occupancy at the Bottom side: " << fHistBottom->Integral() / (double) (fNCbc * 127) << std::endl;
}

void HybridTester::AntennaScan()
{
#ifdef __ANTENNA__
    std::cout << "Mesuring Efficiency per Strip ... " << std::endl;
    std::cout << "Taking data with " << fTotalEvents << " Events!" << std::endl;

    CbcRegReader cReader ( fCbcInterface, "VCth" );
    accept ( cReader );

    Antenna cAntenna;
    cAntenna.initializeAntenna();

    for (int channel = 0; channel < fNCbc; channel++) cAntenna.ConfigureSpiSlave ( channel );

    fHistTop->GetYaxis()->SetRangeUser ( 0, fTotalEvents );
    fHistBottom->GetYaxis()->SetRangeUser ( 0, fTotalEvents );

    for ( uint8_t analog_switch_cs = 0; analog_switch_cs < fNCbc; analog_switch_cs++ )
    {
        std::cout << "Chip Select ID " << +analog_switch_cs << std::endl;
        cAntenna.ConfigureSpiSlave ( analog_switch_cs );

        for ( uint8_t channel_position = 1; channel_position < 10; channel_position++ )
        {
            cAntenna.TurnOnAnalogSwitchChannel ( channel_position );

            if (channel_position == 9) break;

            for ( auto& cShelve : fShelveVector )
            {
                for ( BeBoard* pBoard : cShelve->fBoardVector )
                {
                    uint32_t cN = 1;
                    uint32_t cNthAcq = 0;

                    fBeBoardInterface->Start ( pBoard );

                    //while ( cN <=  fTotalEvents )
                    //{
                    //// Run( pBoard, cNthAcq );
                    fBeBoardInterface->ReadNEvents ( pBoard, fTotalEvents );
                    const std::vector<Event*>& events = fBeBoardInterface->GetEvents ( pBoard );

                    // Loop over Events from this Acquisition
                    for ( auto& cEvent : events )
                    {
                        HistogramFiller cFiller ( fHistBottom, fHistTop, cEvent );
                        pBoard->accept ( cFiller );

                        if ( cN % 100 == 0 ) UpdateHists();

                        cN++;
                    }

                    cNthAcq++;

                    /*Here the reconstruction of histograms happens*/
                    for ( uint16_t channel_id = 1; channel_id < fNCbc * 127 + 1; channel_id++ )
                    {
                        if ( fHistTopMerged->GetBinContent ( channel_id ) < fHistTop->GetBinContent ( channel_id ) ) fHistTopMerged->SetBinContent ( channel_id, fHistTop->GetBinContent ( channel_id ) );

                        if ( fHistBottomMerged->GetBinContent ( channel_id ) < fHistBottom->GetBinContent ( channel_id ) ) fHistBottomMerged->SetBinContent ( channel_id, fHistBottom->GetBinContent ( channel_id ) );
                    }

                    /*Here clearing histograms after each event*/
                    fHistBottom->Reset();
                    fHistTop->Reset();
                }

            }

        }
    }

    fHistTopMerged->Scale ( 100 / double_t ( fTotalEvents ) );
    fHistTopMerged->GetYaxis()->SetRangeUser ( 0, 100 );
    fHistBottomMerged->Scale ( 100 / double_t ( fTotalEvents ) );
    fHistBottomMerged->GetYaxis()->SetRangeUser ( 0, 100 );

    UpdateHistsMerged();

    cAntenna.close();

    TestChannels ( fDecisionThreshold );
#endif
}

void HybridTester::SaveTestingResults (std::string pHybridId)
{

    std::ifstream infile;
    std::string line_buffer;
    std::string content_buffer;
    std::string date_string = currentDateTime();
    std::string filename = "Results/HybridTestingDatabase/Hybrid_ID" + pHybridId + "_on" + date_string + ".txt";
    std::ofstream myfile;
    myfile.open ( filename.c_str() );
    myfile << "Hybrid ID: " << pHybridId << std::endl;
    myfile << "Created on: " << date_string << std::endl << std::endl;
    myfile << " Hybrid Testing Report" << std::endl;
    myfile << "-----------------------" << std::endl << std::endl;
    myfile << " Write/Read Registers Test" << std::endl;
    myfile << "---------------------------" << std::endl;

    infile.open ( fDirectoryName + "/registers_test.txt" );

    while ( getline ( infile, line_buffer ) ) content_buffer += line_buffer + "\r\n"; // To get all the lines.

    if ( content_buffer == "" ) myfile << "Test not performed!" << std::endl;

    infile.close();
    myfile << content_buffer << std::endl;
    content_buffer = "";
    myfile << " Channels Functioning Test" << std::endl;
    myfile << "---------------------------" << std::endl;
    infile.open ( fDirectoryName + "/channels_test2.txt" );

    while ( getline ( infile, line_buffer ) ) content_buffer += line_buffer + "\r\n"; // To get all the lines.

    if ( content_buffer == "" ) myfile << "Test not performed!" << std::endl;

    infile.close();
    myfile << content_buffer << std::endl;
    myfile.close();
    std::cout << std::endl << "Summary testing report written to: " << std::endl << filename << std::endl;
}

void HybridTester::SaveResults()
{
    fHistTop->Write ( fHistTop->GetName(), TObject::kOverwrite );
    fHistBottom->Write ( fHistBottom->GetName(), TObject::kOverwrite );
    fDataCanvas->Write ( fDataCanvas->GetName(), TObject::kOverwrite );

    fResultFile->Write();
    fResultFile->Close();


    std::cout << std::endl << "Resultfile written correctly!" << std::endl;

    std::string cPdfName = fDirectoryName + "/HybridTestResults.pdf";
    fDataCanvas->SaveAs ( cPdfName.c_str() );

    if ( fThresholdScan )
    {
        cPdfName = fDirectoryName + "/ThresholdScanResults.pdf";
        fSCurveCanvas->SaveAs ( cPdfName.c_str() );
    }
}
