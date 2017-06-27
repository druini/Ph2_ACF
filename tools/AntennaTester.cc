#include "AntennaTester.h"

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

                // find out why histograms are not filling!
                if ( globalChannel % 2 == 0 )
                    fBotHist->Fill ( globalChannel / 2 );
                else
                    fTopHist->Fill ( ( globalChannel - 1 ) / 2 );

            }
        }
    }
};

void AntennaTester::Initialize()
{
    gStyle->SetOptStat ( 000000 );
    fDataCanvas = new TCanvas ( "fAntennaDataCanvas", "SingleStripEfficiency (w/ Antenna)", 1200, 800 );
    fDataCanvas->Divide ( 2 );


    InitialiseSettings();
    InitializeHists();
}
void AntennaTester::InitialiseSettings()
{
    fDecisionThreshold = 10.0;
    // figure out whether the hybrid was configured to run in hole/electron mode
    auto cSetting = fSettingsMap.find ( "HoleMode" );
    fHoleMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : true;

    // figure out how many CBCs you're working with
    Counter cCbcCounter;
    accept ( cCbcCounter );
    fNCbc = cCbcCounter.getNCbc();

    // figure out what the number of events to take is
    cSetting = fSettingsMap.find ( "Nevents" );
    fTotalEvents = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 999;
}
void AntennaTester::InitializeHists()
{
    TString cFrontName ( "fAntennaHistTop" );
    fHistTop = ( TH1F* ) ( gROOT->FindObject ( cFrontName ) );

    if ( fHistTop ) delete fHistTop;

    fHistTop = new TH1F ( cFrontName, "Front Pad Channels; Pad Number; Occupancy measured w/ Antenna [%]", ( fNCbc / 2 * 254 ), -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
    fHistTop->SetFillColor ( 4 );
    fHistTop->SetFillStyle ( 3001 );

    TString cBackName ( "fAntennaHistBottom" );
    fHistBottom = ( TH1F* ) ( gROOT->FindObject ( cBackName ) );

    if ( fHistBottom ) delete fHistBottom;

    fHistBottom = new TH1F ( cBackName, "Back Pad Channels; Pad Number; Occupancy measured w/ Antenna [%]", ( fNCbc / 2 * 254 ), -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
    fHistBottom->SetFillColor ( 4 );
    fHistBottom->SetFillStyle ( 3001 );

    TString cFrontNameMerged ( "fAntennaHistTopMerged" );
    fHistTopMerged = ( TH1F* ) ( gROOT->FindObject ( cFrontNameMerged ) );

    if ( fHistTopMerged ) delete fHistTopMerged;

    fHistTopMerged = new TH1F ( cFrontNameMerged, "Front Pad Channels; Pad Number; Occupancy measured w/ Antenna [%]", ( fNCbc / 2 * 254 ), -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
    fHistTopMerged->SetFillColor ( 4 );
    fHistTopMerged->SetFillStyle ( 3001 );

    TString cBackNameMerged ( "fAntennaHistBottomMerged" );
    fHistBottomMerged = ( TH1F* ) ( gROOT->FindObject ( cBackNameMerged ) );

    if ( fHistBottomMerged ) delete fHistBottomMerged;

    fHistBottomMerged = new TH1F ( cBackNameMerged, "Back Pad Channels; Pad Number; Occupancy measured w/ Antenna [%]", ( fNCbc / 2 * 254 ), -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
    fHistBottomMerged->SetFillColor ( 4 );
    fHistBottomMerged->SetFillStyle ( 3001 );
    UpdateHists();
}


void AntennaTester::UpdateHists()
{
    fDataCanvas->cd ( 1 );
    fHistTop->Draw();
    fDataCanvas->cd ( 2 );
    fHistBottom->Draw();
    fDataCanvas->Update();
}
void AntennaTester::UpdateHistsMerged()
{
    fDataCanvas->cd ( 1 );
    fHistTopMerged->Draw();
    fDataCanvas->cd ( 2 );
    fHistBottomMerged->Draw();
    fDataCanvas->Update();
}
void AntennaTester::ReconfigureCBCRegisters (std::string pDirectoryName )
{
    for (auto& cBoard : fBoardVector)
    {
        fBeBoardInterface->CbcHardReset ( cBoard );

        for (auto& cFe : cBoard->fModuleVector)
        {
            for (auto& cCbc : cFe->fCbcVector)
            {
                std::string pRegFile ;
                char buffer[120];

                if ( pDirectoryName.empty() )
                    sprintf (buffer, "%s/FE%dCBC%d.txt", fDirectoryName.c_str(), cCbc->getFeId(), cCbc->getCbcId() );
                else
                    sprintf (buffer, "%s/FE%dCBC%d.txt", pDirectoryName.c_str(), cCbc->getFeId(), cCbc->getCbcId() );

                pRegFile = buffer;
                cCbc->loadfRegMap (pRegFile);
                fCbcInterface->ConfigureCbc ( cCbc );
                LOG (INFO)  << GREEN << "\t\t Successfully reconfigured CBC" << int ( cCbc->getCbcId() ) << "'s regsiters from " << pRegFile << " ." << RESET;
            }
        }

        //CbcFastReset as per recommendation of Mark Raymond
        fBeBoardInterface->CbcFastReset ( cBoard );
    }
}

void AntennaTester::writeObjects()
{
    //writeGraphs();
    fResultFile->cd();
    UpdateHistsMerged();

    fHistTop->Write ( fHistTop->GetName(), TObject::kOverwrite );
    fHistBottom->Write ( fHistBottom->GetName(), TObject::kOverwrite );
    fDataCanvas->Write ( fDataCanvas->GetName(), TObject::kOverwrite );
    LOG (INFO)  << BOLDBLUE << "Results of Antenna scan written to " << fDirectoryName + "/Summary.root" << RESET;

    this->SaveResults();
    fResultFile->Flush();
}


void AntennaTester::Measure()
{

    LOG (INFO)  << "Measuring Efficiency per Strip with the Antenna ... " ;
    LOG (INFO)  << "Taking data with " << fTotalEvents << " Events!";

    //in read mode like this!
    ThresholdVisitor cReader ( fCbcInterface );
    accept ( cReader );

    InitializeHists();
#ifdef __ANTENNA__
    Antenna cAntenna;
    cAntenna.initializeAntenna();

    for (int channel = 0; channel < fNCbc; channel++) cAntenna.ConfigureSpiSlave ( channel );

    fHistTop->GetYaxis()->SetRangeUser ( 0, fTotalEvents );
    fHistBottom->GetYaxis()->SetRangeUser ( 0, fTotalEvents );

    for ( uint8_t analog_switch_cs = 0; analog_switch_cs < fNCbc; analog_switch_cs++ )
    {
        LOG (INFO)  << "Chip Select ID " << +analog_switch_cs ;
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

                    while ( cN <=  fTotalEvents )
                    {
                        // Run( pBoard, cNthAcq );
                        ReadData ( pBoard );
                        const std::vector<Event*>& events = GetEvents ( pBoard );

                        // Loop over Events from this Acquisition
                        for ( auto& cEvent : events )
                        {
                            HistogramFiller cFiller ( fHistBottom, fHistTop, cEvent );
                            pBoard->accept ( cFiller );

                            if ( cN % 100 == 0 ) UpdateHists();

                            cN++;
                        }

                        cNthAcq++;
                    }

                    fBeBoardInterface->Stop ( pBoard, cNthAcq );

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

    cAntenna.close();
#endif
    fHistTopMerged->Scale ( 100 / double_t ( fTotalEvents ) );
    fHistTopMerged->GetYaxis()->SetRangeUser ( 0, 100 );
    fHistBottomMerged->Scale ( 100 / double_t ( fTotalEvents ) );
    fHistBottomMerged->GetYaxis()->SetRangeUser ( 0, 100 );

    UpdateHistsMerged();
    LOG (INFO) << BOLDGREEN << "Mean occupancy (measured w/ antenna) for the Top side: " << GetMeanOccupancyTop() <<  "%." << RESET  ;
    LOG (INFO) << BOLDGREEN << "Mean occupancy (measured w/ antenna) for the Botton side: " << GetMeanOccupancyBottom() << "%." << RESET ;


    //TestChannels( fDecisionThreshold );

}
