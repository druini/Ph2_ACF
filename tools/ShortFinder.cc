#include "ShortFinder.h"

#ifdef __USE_ROOT__

// fill the Histograms, count the hits and increment Vcth
struct HistogramFillerShort  : public HwDescriptionVisitor
{
    TH1F* fBotHist;
    TH1F* fTopHist;
    const Event* fEvent;
    // real index of cbc, otherwise if one cbc is disabled, all indeces will break
    int fCbcIndex = 0;

    HistogramFillerShort ( TH1F* pBotHist, TH1F* pTopHist, const Event* pEvent ) : fBotHist ( pBotHist ), fTopHist ( pTopHist ), fEvent ( pEvent ) {}

    void visit ( Chip& pCbc )
    {
        std::vector<bool> cDataBitVector = fEvent->DataBitVector ( pCbc.getFeId(), pCbc.getChipId() );

        for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
        {
            if ( cDataBitVector.at ( cId ) )
            {
                uint32_t globalChannel = ( fCbcIndex * 254 ) + cId;

                // find out why histograms are not filling!
                if ( globalChannel % 2 == 0 )
                    fBotHist->Fill ( globalChannel / 2 );
                else
                    fTopHist->Fill ( ( globalChannel - 1 ) / 2 );

            }
        }

        fCbcIndex++;
    }
};

ShortFinder::ShortFinder() : Tool()
{
    fNShorts = 0;
    fNShortsTop = 0 ;
    fNShortsBottom = 0;
    fTotalEvents = 0;
    fNCbc = 2;
}

// D'tor
ShortFinder::~ShortFinder() {}

//Reload CBC registers from file found in results (fDirectoryName) directory .
//If no directory is found use the default files for the different operational modes found in Ph2_ACF/settings
void ShortFinder::ReconfigureRegisters()
{
    
    for (auto& cBoard : fBoardVector)
    {
        fBeBoardInterface->ChipReset ( cBoard );

        for (auto& cFe : cBoard->fModuleVector)
        {
            for (auto& cCbc : cFe->fReadoutChipVector)
            {
                std::string pRegFile ;

                if ( fDirectoryName.empty() )
                    pRegFile = "settings/Cbc_default_electron.txt";
                else
                {
                    char buffer[120];
                    sprintf (buffer, "%s/FE%dCBC%d.txt", fDirectoryName.c_str(), cCbc->getFeId(), cCbc->getChipId() );
                    pRegFile = buffer;
                }

                cCbc->loadfRegMap (pRegFile);
                fReadoutChipInterface->ConfigureChip ( cCbc );
                LOG (INFO) << GREEN << "\t\t Successfully (re)configured CBC" << int ( cCbc->getChipId() ) << "'s regsiters from " << pRegFile << " ." << RESET;
            }
        }

        fBeBoardInterface->ChipReSync ( cBoard );
    }
}
void ShortFinder::ConfigureVcth (uint16_t pVcth)
{
    ThresholdVisitor cWriter ( fReadoutChipInterface, pVcth );
    accept ( cWriter );
}

void ShortFinder::writeGraphs()
{
    fResultFile->cd();
    fShortsCanvas->Write ( fShortsCanvas->GetName(), TObject::kOverwrite );
    //fDataCanvas->Write ( fDataCanvas->GetName(), TObject::kOverwrite );
}
void ShortFinder::SaveResults()
{
    LOG (INFO) << BOLDBLUE << "Results of short finder for all CBCs written to " << fDirectoryName + "/Summary.root" << RESET ;
    writeGraphs();

}

void ShortFinder::InitialiseSettings()
{
    // figure out whether the hybrid was configured to run in hole/electron mode
    auto cSetting = fSettingsMap.find ( "HoleMode" );
    fHoleMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : false;

    // figure out how many CBCs you're working with
    Counter cCbcCounter;
    accept ( cCbcCounter );
    fNCbc = cCbcCounter.getNChip();

    // figure out what the number of events to take it
    cSetting = fSettingsMap.find ( "Nevents" );
    fTotalEvents = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 999;

    cSetting = fSettingsMap.find ( "TestPulsePotentiometer" );
    fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x7F;

    // LOG (INFO) << "Read the following Settings: " ;
    // LOG (INFO) << "Hole Mode: " << fHoleMode << std::endl << "NEvents: " << fTotalEvents << std::endl << "NSigmas: " << fSigmas ;
}
void ShortFinder::Initialize()
{
    gStyle->SetOptStat ( 000000 );
    //gStyle->SetTitleOffset( 1.3, "Y" );
    fDataCanvas = new TCanvas ( "fDataCanvas", "SingleStripEfficiency", 1200, 800 );
    fDataCanvas->Divide ( 2 );

    fShortsCanvas = new TCanvas ("fShortsCanvas", "Shorts", 1200, 800 );
    fShortsCanvas->Divide (2 );

    InitialiseSettings();
    InitializeHists();
}
void ShortFinder::InitializeHists()
{
    uint32_t cNBinsX = (fNCbc * 254) / 2;
    TString cFrontName ( "fHistTop" );
    fHistTop = ( TH1F* ) ( gROOT->FindObject ( cFrontName ) );

    if ( fHistTop ) delete fHistTop;

    fHistTop = new TH1F ( cFrontName, "Front Pad Channels; Pad Number; Occupancy [%]", ( cNBinsX ), -0.5, ( cNBinsX ) - 0.5 );
    fHistTop->SetFillColor ( 4 );
    fHistTop->SetFillStyle ( 3001 );

    TString cBackName ( "fHistBottom" );
    fHistBottom = ( TH1F* ) ( gROOT->FindObject ( cBackName ) );

    if ( fHistBottom ) delete fHistBottom;

    fHistBottom = new TH1F ( cBackName, "Back Pad Channels; Pad Number; Occupancy [%]", ( cNBinsX ), -0.5, ( cNBinsX ) - 0.5 );
    fHistBottom->SetFillColor ( 4 );
    fHistBottom->SetFillStyle ( 3001 );

    TString cFrontNameMerged ( "fHistTopMerged" );
    fHistTopMerged = ( TH1F* ) ( gROOT->FindObject ( cFrontNameMerged ) );

    if ( fHistTopMerged ) delete fHistTopMerged;

    fHistTopMerged = new TH1F ( cFrontNameMerged, "Front Pad Channels; Pad Number; Occupancy [%]", ( cNBinsX ) , -0.5, ( cNBinsX ) - 0.5 );
    fHistTopMerged->SetFillColor ( 4 );
    fHistTopMerged->SetFillStyle ( 3001 );

    TString cBackNameMerged ( "fHistBottomMerged" );
    fHistBottomMerged = ( TH1F* ) ( gROOT->FindObject ( cBackNameMerged ) );

    if ( fHistBottomMerged ) delete fHistBottomMerged;

    fHistBottomMerged = new TH1F ( cBackNameMerged, "Back Pad Channels; Pad Number; Occupancy [%]", ( cNBinsX ) , -0.5, ( cNBinsX ) - 0.5 );
    fHistBottomMerged->SetFillColor ( 4 );
    fHistBottomMerged->SetFillStyle ( 3001 );


    TString cShortBackground ( "fHistShortBackground" );
    fHistShortBackground = ( TH2F* ) ( gROOT->FindObject ( cShortBackground ) );

    if ( fHistShortBackground ) delete fHistShortBackground;

    fHistShortBackground = new TH2F ( cShortBackground, "", ( cNBinsX ) , -0.5, ( cNBinsX ) - 1.5 , 2  , -0.5 , 2.0 - 0.5 );
    fHistShortBackground->SetFillColor ( 4 );
    fHistShortBackground->SetFillStyle ( 3001 );
    //for( int i = 0 ; i < ( cNBinsX ) ; i++ ) fHistShortsTop->Fill(i,0);
    fHistShortBackground->GetYaxis()->SetRangeUser (-0.5, 1.5);
    fHistShortBackground->GetYaxis()->SetTitle ("");
    fHistShortBackground->GetXaxis()->SetTitle ("Strip Number");
    fHistShortBackground->GetYaxis()->SetTitleOffset (1.6);
    fHistShortBackground->GetYaxis()->SetBinLabel (1, Form ("Functional") );
    fHistShortBackground->GetYaxis()->SetBinLabel (2, Form ("Shorted") );




    TString cShortsTopName ( "fHistShortsTop" );
    fHistShortsTop = ( TH1F* ) ( gROOT->FindObject ( cShortsTopName ) );

    if ( fHistShortsTop ) delete fHistShortsTop;

    fHistShortsTop = new TH1F ( cShortsTopName, "Shorts on Front Pad Channels", ( cNBinsX ) , -0.5, ( cNBinsX ) - 0.5  );
    fHistShortsTop->SetLineColor ( 3 );
    fHistShortsTop->SetFillColor ( 3 );
    fHistShortsTop->SetFillStyle ( 3005 );

    for ( uint32_t i = 0 ; i < ( cNBinsX ) ; i++ ) fHistShortsTop->Fill (i, 0);

    // fill histogram randomly to check if the ploting/saving works
    // TRandom *eventGenerator = new TRandom();
    // for( int i = 0 ; i < 100 ; i++)
    // {
    //     int cChanelID = eventGenerator->Integer( fNCbc * 127 );
    //     int cShort = eventGenerator->Integer( 2 ) ;
    //     fHistShortsTop->Fill( cChanelID ,  cShort);
    // }



    TString cShortsBackName ( "fHistShortsBottom" );
    fHistShortsBottom = ( TH1F* ) ( gROOT->FindObject ( cShortsBackName ) );

    if ( fHistShortsBottom ) delete fHistShortsBottom;

    fHistShortsBottom = new TH1F ( cShortsBackName, "Shorts on Back Pad Channels", ( cNBinsX ) , -0.5, ( cNBinsX ) - 0.5 );
    fHistShortsBottom->SetLineColor ( 4 );
    fHistShortsBottom->SetFillColor ( 4 );
    fHistShortsBottom->SetFillStyle ( 3005 );

    for ( uint32_t i = 0 ; i < ( cNBinsX) ; i++ ) fHistShortsBottom->Fill (i, 0);

    // fill histogram randomly to check if the ploting/saving works
    // for( int i = 0 ; i < 100 ; i++)
    // {
    //     // fill histogram randomly to check if the ploting/saving works
    //     int cChanelID = eventGenerator->Integer( fNCbc * 127 );
    //     int cShort = eventGenerator->Integer( 2 ) ;
    //     fHistShortsBottom->Fill( cChanelID ,  cShort);
    // }


    UpdateHists();
}

void ShortFinder::UpdateHists()
{
    // fDataCanvas->cd( 1 );
    // fHistTop->Draw();
    // fDataCanvas->cd( 2 );
    // fHistBottom->Draw();
    // fDataCanvas->Update();

    fShortsCanvas->cd ( 1 );
    fShortsCanvas->cd (1)->SetLeftMargin (0.15);
    fHistShortBackground->Draw();
    fHistShortsTop->Draw ("HistoSAME");
    fShortsCanvas->cd ( 2 );
    fShortsCanvas->cd (2)->SetLeftMargin (0.15);
    fHistShortBackground->Draw();
    fHistShortsBottom->Draw ("HistoSAME");
    fShortsCanvas->Update();

    this->HttpServerProcess();
}
void ShortFinder::UpdateHistsMerged()
{
    fDataCanvas->cd ( 1 );
    fHistTopMerged->Draw();
    fDataCanvas->cd ( 2 );
    fHistBottomMerged->Draw();
    fDataCanvas->Update();

    this->HttpServerProcess();
}

void ShortFinder::SetBeBoard (BeBoard* pBoard)
{
    if(pBoard->getBoardType() == BoardType::D19C) fBeBoardInterface->WriteBoardReg(pBoard, "fc7_daq_cnfg.fast_command_block.test_pulse.delay_after_test_pulse", 1);
    setFWTestPulse();

    // (potential, group, enable test pulse, hole mode)
    setSystemTestPulse(fTestPulseAmplitude,0x00,true,fHoleMode);

    //edit G.A: in order to be compatible with CBC3 (9 bit trigger latency) the recommended method is this:
    LatencyVisitor cLatencyVisitor (fReadoutChipInterface, 0x01);
    this->accept (cLatencyVisitor);

    // Take the default VCth which should correspond to the pedestal and add 8 depending on the mode to exclude noise
    // ThresholdVisitor in read mode
    ThresholdVisitor cThresholdVisitor (fReadoutChipInterface);
    this->accept (cThresholdVisitor);
    uint16_t cVcth = cThresholdVisitor.getThreshold();

    int cVcthStep = ( fHoleMode == 1 ) ? +10 : -10;
    LOG (INFO) << "VCth value from config file is: " << +cVcth << " ;  changing by " << cVcthStep << "  to " << + ( cVcth + cVcthStep ) << " supress noise hits for crude latency scan!" ;
    cVcth += cVcthStep;

    //  Set that VCth Value on all FEs
    cThresholdVisitor.setOption ('w');
    cThresholdVisitor.setThreshold (cVcth);
    this->accept (cThresholdVisitor);
}
bool ShortFinder::CheckChannel (Short pShort, ShortsList pShortsList)
{
    for (auto cChannel : pShortsList)
    {
        if (cChannel[0] == pShort[0] && cChannel[1] == pShort[1]) return true;
    }

    return false;
}

void ShortFinder::SetTestGroup(BeBoard* pBoard, uint8_t pTestGroup)
{
    for (auto cFe : pBoard->fModuleVector)
    {
        for (auto cCbc : cFe->fReadoutChipVector)
        {
            std::vector<std::pair<std::string, uint16_t>> cRegVec;
            uint8_t cRegValue = this->to_reg ( 0, pTestGroup );

            if (cCbc->getFrontEndType() == FrontEndType::CBC3)
            {
                //CBC3
                cRegVec.push_back ( std::make_pair ( "TestPulseDel&ChanGroup",  cRegValue ) );
            }
            else
            {
                //CBC2
                cRegVec.push_back ( std::make_pair ( "SelTestPulseDel&ChanGroup",  cRegValue ) );
            }

            this->fReadoutChipInterface->WriteChipMultReg (cCbc, cRegVec);
        }
    }
}

//std::vector<std::array<int, 2>> HybridTester::MergeShorts(std::vector<std::array<int, 2>> pShortA, std::vector<std::array<int, 2>> pShortB)
void ShortFinder::MergeShorts (ShortsList pShortsListA)
{
    for (auto cChannel : pShortsListA)
    {
        if (!CheckChannel (cChannel, fShortsList) )
            fShortsList.push_back (cChannel);
    }

    std::stringstream outp;
    for (auto cMemberChannel : fShortsList)
    {
        for (auto i : cMemberChannel) outp << i << ' ';
    }
    LOG(INFO) << outp.str();
}

void ShortFinder::FindShorts (std::ostream& os )
{
    int cTestPulseGroupId = 0;

    ShortedChannel cShortedChannelInfo;
    ShortedGroupsList cShortedGroupsArray;

    Short cGroundedChannel;
    ShortsList cGroundedChannelsList;

    //in read mode
    ThresholdVisitor cVisitor (fReadoutChipInterface);
    accept (cVisitor);
    fHistTop->GetYaxis()->SetRangeUser ( 0, fTotalEvents );
    fHistBottom->GetYaxis()->SetRangeUser ( 0, fTotalEvents );

    for ( BeBoard* pBoard : fBoardVector )
    {
        uint32_t cN = 1;

        SetBeBoard (pBoard);

        os << "\nShorted channels searching procedure\nSides: Top - 0\tBottom - 1 (Channel numbering starts from 0)\n" << std::endl;
        os << "      CBC\t| Side\t| Channel_ID\t| Group_ID\t| Shorted_With_Group_ID" << std::endl;

        for (cTestPulseGroupId = 0; cTestPulseGroupId < 8; cTestPulseGroupId++)
        {
            cN = 1;

            this->SetTestGroup(pBoard,(uint8_t)cTestPulseGroupId);

            ReadNEvents ( pBoard, fTotalEvents );
            const std::vector<Event*>& events = GetEvents ( pBoard );

            // Loop over Events from this Acquisition
            for ( auto& cEvent : events )
            {
                HistogramFillerShort cFiller ( fHistBottom, fHistTop, cEvent );
                pBoard->accept ( cFiller );

                if ( cN % 50 == 0 )
                    //LOG(INFO) << *cEvent;
                    UpdateHists();

                cN++;
            }

            ShortedGroup cShortedChannelsGroup;

            for ( uint16_t cChannelId = 1; cChannelId < fNCbc * 127 + 1; cChannelId++ )
            {
                if ( fHistTop->GetBinContent ( cChannelId ) > 0.5 * fTotalEvents && ( (cChannelId - 1) % 127) % 8 != cTestPulseGroupId )
                {
                    cShortedChannelInfo = {0, (cChannelId - 1), ( (cChannelId - 1) % 127) % 8, cTestPulseGroupId, 0};
                    cShortedChannelsGroup.push_back (cShortedChannelInfo);
                    os << "\t0\t|\t" << cShortedChannelInfo[1] << "\t|\t" << cShortedChannelInfo[2] << "\t|\t" << cShortedChannelInfo[3] << std::endl;
                }
                else if (fHistTop->GetBinContent ( cChannelId ) < 0.5 * fTotalEvents && ( (cChannelId - 1) % 127) % 8 == cTestPulseGroupId)
                {
                    cGroundedChannel = {0, (cChannelId - 1) };
                    cGroundedChannelsList.push_back (cGroundedChannel);
                    os << "\t0\t|\t" << (cChannelId - 1) << "\t|\t" << (cTestPulseGroupId) << "\t|\tGND" << std::endl;
                }

                if ( fHistBottom->GetBinContent ( cChannelId ) > 0.5 * fTotalEvents && ( (cChannelId - 1) % 127) % 8 != cTestPulseGroupId)
                {
                    cShortedChannelInfo = {1, (cChannelId - 1), ( (cChannelId - 1) % 127) % 8, cTestPulseGroupId, 0};
                    cShortedChannelsGroup.push_back (cShortedChannelInfo);
                    os << "\t1\t|\t" << cShortedChannelInfo[1] << "\t|\t" << cShortedChannelInfo[2] << "\t|\t" << cShortedChannelInfo[3] << std::endl;
                }
                else if (fHistBottom->GetBinContent ( cChannelId ) < 0.5 * fTotalEvents && ( (cChannelId - 1) % 127) % 8 == cTestPulseGroupId)
                {
                    cGroundedChannel = {1, (cChannelId - 1) };
                    cGroundedChannelsList.push_back (cGroundedChannel);
                    os << "\t1\t|\t" << (cChannelId - 1) << "\t|\t" << (cTestPulseGroupId) << "\t|\tGND" << std::endl;
                }

            }

            cShortedGroupsArray[cTestPulseGroupId] = cShortedChannelsGroup;
            fHistBottom->Reset();
            fHistTop->Reset();
            os << "------------------------------------------------------------------------" << std::endl;


        }
    }

    ReconstructShorts (cShortedGroupsArray, os );

    UpdateHists();
}

void ShortFinder::ReconstructShorts (ShortedGroupsList pShortedGroupsArray, std::ostream& os )
{

    LOG (INFO) << std::endl << "---------Creating shorted pairs-----------------";
    std::vector<ShortsList> cShortsVector;
    ShortsList cShort;
    Short temp_shorted_channel;

    Short best_candidate;
    int best_candidate_index = 0;
    int index = 0;
    int sub_index = 0;
    int cross_side_punishment = 2;
    int smallest_distance = 10000;
    bool matching_channel_found;

    for ( auto cShortedChannelsGroup : pShortedGroupsArray)
    {
        for (auto cShortedChannelInfo : cShortedChannelsGroup)
        {
            temp_shorted_channel[0] = cShortedChannelInfo[0];
            temp_shorted_channel[1] = cShortedChannelInfo[1];

            cShort.push_back (temp_shorted_channel);
            cShort.push_back (temp_shorted_channel);

            index = 0;
            matching_channel_found = false;

            for ( auto cCandidate : pShortedGroupsArray[cShortedChannelInfo[2]] )
            {
                if (cShortedChannelInfo[3] == cCandidate[2])
                {
                    if ( smallest_distance > (cross_side_punishment * (cShort[0][0] - cCandidate[0]) * (cShort[0][0] - cCandidate[0]) + (cShort[0][1] - cCandidate[1]) *  (cShort[0][1] - cCandidate[1]) ) )
                    {
                        smallest_distance = (cross_side_punishment * (cShort[0][0] - cCandidate[0]) * (cShort[0][0] - cCandidate[0]) + (cShort[0][1] - cCandidate[1]) * (cShort[0][1] - cCandidate[1]) );
                        best_candidate[0] = cCandidate[0];
                        best_candidate[1] = cCandidate[1];
                        best_candidate_index = index;
                        matching_channel_found = true;
                    }
                }

                index++;
            }

            if (matching_channel_found)
            {
                if (best_candidate[0] <= cShort[0][0] && best_candidate[1] < cShort[0][1] )
                {
                    cShort[0][0] = best_candidate[0];
                    cShort[0][1] = best_candidate[1];
                }

                else
                {
                    cShort[1][0] = best_candidate[0];
                    cShort[1][1] = best_candidate[1];
                }

                pShortedGroupsArray[cShortedChannelInfo[2]].erase (pShortedGroupsArray[cShortedChannelInfo[2]].begin() + best_candidate_index);

                std::stringstream outp;
                for (auto cMemberChannel : cShort)
                {
                    for (auto i : cMemberChannel) outp << i << ' ';
                }
                LOG(INFO) << outp.str();

                os << "smallest distance: " << smallest_distance << std::endl;
                //DisplayGroupsContent(pShortedGroupsArray);

                cShortsVector.push_back (cShort);
            }
            else os << "ERROR: No matching channel found for detected short (watch the level of noise)!" << std::endl;

            smallest_distance = 10000;
            cShort.clear();
        }
    }

    os << "---------Merging shorts-------------------------" << std::endl;
    index = cShortsVector.size();
    fNShorts = cShortsVector.size();
    os << "Number of shorted connections found: " << fNShorts << std::endl;

    while ( index > 1 )
    {
        index--;
        sub_index = index;

        while (sub_index > 0)
        {
            sub_index--;

            if (CheckShortsConnection ( cShortsVector[index], cShortsVector[sub_index]) )
            {
                cShortsVector[sub_index] = MergeShorts ( cShortsVector[index], cShortsVector[sub_index]);
                cShortsVector.erase (cShortsVector.begin() + index);
                break;
            }
        }
    }

    os << "---------Outcome--------------------------------" << std::endl;
    fNShortsTop = 0 ;
    fNShortsBottom = 0;

    for (auto someShort : cShortsVector)
    {
        for (auto cMemberChannel : someShort)
        {
            if ( cMemberChannel[0] == 0 )
            {
                fHistShortsTop->Fill ( cMemberChannel[1], 1 );
                fNShortsTop++;
            }
            else if ( cMemberChannel[0] == 1 )
            {
                fHistShortsBottom->Fill ( cMemberChannel[1], 1 );
                fNShortsBottom++;
            }

            for (auto i : cMemberChannel)
                os  << i << ' ';
        }

        os << std::endl;
    }
}
bool ShortFinder::CheckShortsConnection (ShortsList pShortA, ShortsList pShortB)
{
    for (auto cChannel : pShortA)
    {
        if (CheckChannelInShortPresence (cChannel, pShortB) ) return true;
    }

    return false;
}

ShortsList ShortFinder::MergeShorts (ShortsList pShortA, ShortsList pShortB)
{
    for (auto cChannel : pShortA)
    {
        if (!CheckChannelInShortPresence (cChannel, pShortB) )
            pShortB.push_back (cChannel);
    }

    std::stringstream outp;
    for (auto cMemberChannel : pShortB)
    {
        for (auto i : cMemberChannel) outp << i << ' ';
    }

    LOG (INFO) << outp.str();

    return pShortB;
}
bool ShortFinder::CheckChannelInShortPresence ( Short pShortedChannel, ShortsList pShort)
{
    for (auto cChannel : pShort)
    {
        if (cChannel[0] == pShortedChannel[0] && cChannel[1] == pShortedChannel[1]) return true;
    }

    return false;
}

#endif
