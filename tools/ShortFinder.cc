#include "ShortFinder.h"

// fill the Histograms, count the hits and increment Vcth
struct HistogramFiller  : public HwDescriptionVisitor
{
    TH1F* fBotHist;
    TH1F* fTopHist;
    const Event* fEvent;

    HistogramFiller( TH1F* pBotHist, TH1F* pTopHist, const Event* pEvent ): fBotHist( pBotHist ), fTopHist( pTopHist ), fEvent( pEvent ) {}

    void visit( Cbc& pCbc ) {
        std::vector<bool> cDataBitVector = fEvent->DataBitVector( pCbc.getFeId(), pCbc.getCbcId() );
        for ( uint32_t cId = 0; cId < NCHANNELS; cId++ ) {
            if ( cDataBitVector.at( cId ) ) {
                uint32_t globalChannel = ( pCbc.getCbcId() * 254 ) + cId;
                //              LOG (INFO) << "Channel " << globalChannel << " VCth " << int(pCbc.getReg( "VCth" )) << std::endl;
                // find out why histograms are not filling!
                if ( globalChannel % 2 == 0 )
                    fBotHist->Fill( globalChannel / 2 );
                else
                    fTopHist->Fill( ( globalChannel - 1 ) / 2 );

            }
        }
    }
};

//Reload CBC registers from file found in results (fDirectoryName) directory . 
//If no directory is found use the default files for the different operational modes found in Ph2_ACF/settings
void ShortFinder::ReconfigureRegisters()
{
    bool cCheck;
    bool cHoleMode;
    auto cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )
    {
        cCheck = true;
        cHoleMode = ( cSetting->second == 1 ) ? true : false;
    }

    std::string cMode;

    if ( cCheck )
    {
        if ( cHoleMode ) cMode = "hole";
        else cMode = "electron";
    }

    

    for (auto& cBoard : fBoardVector)
    {
        fBeBoardInterface->CbcHardReset ( cBoard );
        for (auto& cFe : cBoard->fModuleVector)
        {
            for (auto& cCbc : cFe->fCbcVector)
            {
                 std::string pRegFile ;
                if( fDirectoryName.empty() )
                {
                    pRegFile = "settings/Cbc_default_" +  cMode + ".txt";
                }
                else
                {
                    char buffer[120];
                    sprintf(buffer, "%s/FE%dCBC%d.txt" , fDirectoryName.c_str() , cCbc->getFeId(), cCbc->getCbcId() );
                    pRegFile = buffer;
                }

                cCbc->loadfRegMap(pRegFile);
                fCbcInterface->ConfigureCbc ( cCbc );
                LOG (INFO) <<GREEN << "\t\t Successfully (re)configured CBC" << int ( cCbc->getCbcId() ) << "'s regsiters from " << pRegFile << " ." << RESET;
            }
        }

        //CbcFastReset as per recommendation of Mark Raymond
        fBeBoardInterface->CbcFastReset ( cBoard );
    }
}
void ShortFinder::ConfigureVcth(uint8_t pVcth)
{
    CbcRegWriter cWriter ( fCbcInterface, "VCth", pVcth );
    accept ( cWriter );
}

void ShortFinder::writeGraphs()
{
    fResultFile->cd();
    fShortsCanvas->Write( fShortsCanvas->GetName() , TObject::kOverwrite );
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
    auto cSetting = fSettingsMap.find( "HoleMode" );
    fHoleMode = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : true;
    
    // figure out how many CBCs you're working with
    Counter cCbcCounter;
    accept( cCbcCounter );
    fNCbc = cCbcCounter.getNCbc();

    // figure out what the number of events to take it 
    cSetting = fSettingsMap.find( "Nevents" );
    fTotalEvents = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 999; 

    // LOG (INFO) << "Read the following Settings: " ;
    // LOG (INFO) << "Hole Mode: " << fHoleMode << std::endl << "NEvents: " << fTotalEvents << std::endl << "NSigmas: " << fSigmas ;
}
void ShortFinder::Initialize()
{
    gStyle->SetOptStat( 000000 );
    //gStyle->SetTitleOffset( 1.3, "Y" );
    fDataCanvas = new TCanvas( "fDataCanvas", "SingleStripEfficiency", 1200, 800 );
    fDataCanvas->Divide( 2 );

    fShortsCanvas = new TCanvas("fShortsCanvas","Shorts", 1200 ,800 );
    fShortsCanvas->Divide(2 );

    InitialiseSettings();
    InitializeHists();
}
void ShortFinder::InitializeHists()
{
    TString cFrontName( "fHistTop" );
    fHistTop = ( TH1F* )( gROOT->FindObject( cFrontName ) );
    if ( fHistTop ) delete fHistTop;

    fHistTop = new TH1F( cFrontName, "Front Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ), -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
    fHistTop->SetFillColor( 4 );
    fHistTop->SetFillStyle( 3001 );

    TString cBackName( "fHistBottom" );
    fHistBottom = ( TH1F* )( gROOT->FindObject( cBackName ) );
    if ( fHistBottom ) delete fHistBottom;

    fHistBottom = new TH1F( cBackName, "Back Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ), -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
    fHistBottom->SetFillColor( 4 );
    fHistBottom->SetFillStyle( 3001 );
    
    TString cFrontNameMerged( "fHistTopMerged" );
    fHistTopMerged = ( TH1F* )( gROOT->FindObject( cFrontNameMerged ) );
    if ( fHistTopMerged ) delete fHistTopMerged;

    fHistTopMerged = new TH1F( cFrontNameMerged, "Front Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ) , -0.5, ( fNCbc / 2 * 254 ) -0.5 );
    fHistTopMerged->SetFillColor( 4 );
    fHistTopMerged->SetFillStyle( 3001 );

    TString cBackNameMerged( "fHistBottomMerged" );
    fHistBottomMerged = ( TH1F* )( gROOT->FindObject( cBackNameMerged ) );
    if ( fHistBottomMerged ) delete fHistBottomMerged;

    fHistBottomMerged = new TH1F( cBackNameMerged, "Back Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ) , -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
    fHistBottomMerged->SetFillColor( 4 );
    fHistBottomMerged->SetFillStyle( 3001 );


    TString cShortBackground( "fHistShortBackground" );
    fHistShortBackground = ( TH2F* )( gROOT->FindObject( cShortBackground ) );
    if ( fHistShortBackground ) delete fHistShortBackground;

    fHistShortBackground = new TH2F( cShortBackground, "", ( fNCbc / 2 * 254 ) , -0.5, ( fNCbc / 2 * 254 ) - 1.5 , 2  , -0.5 , 2.0 - 0.5 );
    fHistShortBackground->SetFillColor( 4 );
    fHistShortBackground->SetFillStyle( 3001 );
    //for( int i = 0 ; i < ( fNCbc / 2 * 254 ) ; i++ ) fHistShortsTop->Fill(i,0);
    fHistShortBackground->GetYaxis()->SetRangeUser(-0.5,1.5);
    fHistShortBackground->GetYaxis()->SetTitle("");
    fHistShortBackground->GetXaxis()->SetTitle("Strip Number");
    fHistShortBackground->GetYaxis()->SetTitleOffset(1.6);
    fHistShortBackground->GetYaxis()->SetBinLabel (1, Form ("Functional") );
    fHistShortBackground->GetYaxis()->SetBinLabel (2, Form ("Shorted") ); 




    TString cShortsTopName( "fHistShortsTop" );
    fHistShortsTop = ( TH1F* )( gROOT->FindObject( cShortsTopName ) );
    if ( fHistShortsTop ) delete fHistShortsTop;

    fHistShortsTop = new TH1F( cShortsTopName, "Shorts on Front Pad Channels", ( fNCbc / 2 * 254 ) , -0.5, ( fNCbc / 2 * 254 ) - 0.5  );
    fHistShortsTop->SetLineColor( 3 );
    fHistShortsTop->SetFillColor( 3 );
    fHistShortsTop->SetFillStyle( 3005 );
    for( int i = 0 ; i < ( fNCbc / 2 * 254 ) ; i++ ) fHistShortsTop->Fill(i,0); 
    
    // fill histogram randomly to check if the ploting/saving works
    // TRandom *eventGenerator = new TRandom();
    // for( int i = 0 ; i < 100 ; i++)
    // {
    //     int cChanelID = eventGenerator->Integer( fNCbc * 127 );
    //     int cShort = eventGenerator->Integer( 2 ) ;
    //     fHistShortsTop->Fill( cChanelID ,  cShort);
    // }
                


    TString cShortsBackName( "fHistShortsBottom" );
    fHistShortsBottom = ( TH1F* )( gROOT->FindObject( cShortsBackName ) );
    if ( fHistShortsBottom ) delete fHistShortsBottom;

    fHistShortsBottom = new TH1F( cShortsBackName, "Shorts on Back Pad Channels", ( fNCbc / 2 * 254 ) , -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
    fHistShortsBottom->SetLineColor( 4 );
    fHistShortsBottom->SetFillColor( 4 );
    fHistShortsBottom->SetFillStyle( 3005 );
    for( int i = 0 ; i < ( fNCbc / 2 * 254 ) ; i++ ) fHistShortsBottom->Fill(i,0); 
    
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

    fShortsCanvas->cd( 1 );
    fShortsCanvas->cd(1)->SetLeftMargin(0.15);
    fHistShortBackground->Draw(); fHistShortsTop->Draw("HistoSAME"); 
    fShortsCanvas->cd( 2 );
    fShortsCanvas->cd(2)->SetLeftMargin(0.15);
    fHistShortBackground->Draw(); fHistShortsBottom->Draw("HistoSAME"); 
    fShortsCanvas->Update();
    


}
void ShortFinder::UpdateHistsMerged() 
{
    fDataCanvas->cd( 1 );
    fHistTopMerged->Draw();
    fDataCanvas->cd( 2 );
    fHistBottomMerged->Draw();
    fDataCanvas->Update();


}

void ShortFinder::SetBeBoard(BeBoard* pBoard)
{

    fBeBoardInterface->WriteBoardReg (pBoard, "COMMISSIONNING_MODE_RQ", 1 );
    fBeBoardInterface->WriteBoardReg (pBoard, "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID", 1 );
    fBeBoardInterface->WriteBoardReg (pBoard, "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE", 2 );
    
    LOG (INFO) << "COMMISSIONNING_MODE_RQ: " << fBeBoardInterface->ReadBoardReg( pBoard, "COMMISSIONNING_MODE_RQ" );
    LOG (INFO) << "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID: " << fBeBoardInterface->ReadBoardReg( pBoard, "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID" ) ;
    LOG (INFO) << "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE: " << fBeBoardInterface->ReadBoardReg( pBoard, "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE" ) ;

    std::vector<std::pair<std::string, uint8_t>> cRegVec;    

    if ( fHoleMode )
        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0xE1 ) );
    else
        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0x61 ) );

    cRegVec.push_back ( std::make_pair ( "TestPulsePot", 0xF0 ) );

    cRegVec.push_back ( std::make_pair ( "VCth", 0x90 ) );

    cRegVec.push_back ( std::make_pair ( "TriggerLatency", 0x01 ) );

    CbcMultiRegWriter cMultiWriter ( fCbcInterface, cRegVec );
    this->accept ( cMultiWriter );
}
bool ShortFinder::CheckChannel(Short pShort , ShortsList pShortsList)
{
    for (auto cChannel : pShortsList)
    {
        if (cChannel[0] == pShort[0] && cChannel[1] == pShort[1]) return true;
    }
    return false;
}
//std::vector<std::array<int, 2>> HybridTester::MergeShorts(std::vector<std::array<int, 2>> pShortA, std::vector<std::array<int, 2>> pShortB)
void ShortFinder::MergeShorts(ShortsList pShortsListA)
{
    for (auto cChannel : pShortsListA)
    {
        if (!CheckChannel(cChannel, fShortsList))
        {
            fShortsList.push_back(cChannel);
        }
    }

    for (auto cMemberChannel: fShortsList)
    {
        for (auto i: cMemberChannel) LOG (INFO) << i << ' ';
    }
}

void ShortFinder::FindShorts(std::ostream& os )
{
    uint8_t cGroupAddress[8] = {0, 4, 2, 6, 1, 5, 3, 7};
    int cTestPulseGroupId = 0;
    
    ShortedChannel cShortedChannelInfo;     
    ShortedGroupsList cShortedGroupsArray;

    Short cGroundedChannel;
    ShortsList cGroundedChannelsList;
    
    CbcRegReader cReader( fCbcInterface, "VCth" );
    accept( cReader );
    fHistTop->GetYaxis()->SetRangeUser( 0, fTotalEvents );
    fHistBottom->GetYaxis()->SetRangeUser( 0, fTotalEvents );
    
    for ( BeBoard* pBoard : fBoardVector )
    {
        uint32_t cN = 1;
        uint32_t cNthAcq = 0;   

        SetBeBoard(pBoard);
                
        CbcRegWriter cWriter( fCbcInterface, "SelTestPulseDel&ChanGroup", cGroupAddress[cTestPulseGroupId]);
        os<<"\nShorted channels searching procedure\nSides: Top - 0\tBottom - 1 (Channel numbering starts from 0)\n"<<std::endl;
        os<<"      Side\t| Channel_ID\t| Group_ID\t| Shorted_With_Group_ID"<<std::endl;
        for (cTestPulseGroupId = 0; cTestPulseGroupId < 8; cTestPulseGroupId++)
        {
            cN = 1;
            cNthAcq = 0;

            cWriter.setRegister( "SelTestPulseDel&ChanGroup", cGroupAddress[cTestPulseGroupId]);
            accept( cWriter );

            fBeBoardInterface->Start( pBoard );
            while ( cN <=  fTotalEvents )
            {
                //Run( pBoard, cNthAcq );
                fBeBoardInterface->ReadData( pBoard, false );
                //fBeBoardInterface->ReadNEvents ( pBoard, cNthAcq );
                const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );
                
                // Loop over Events from this Acquisition
                for ( auto& cEvent : events )
                {
                    HistogramFiller cFiller( fHistBottom, fHistTop, cEvent );
                    pBoard->accept( cFiller );
                    
                    if ( cN % 100 == 0 )
                      UpdateHists();
                    
                    cN++;
                }
                cNthAcq++;
            }
            fBeBoardInterface->Stop( pBoard);

            ShortedGroup cShortedChannelsGroup;
            for ( uint16_t cChannelId = 1; cChannelId < fNCbc * 127 + 1; cChannelId++ )
            {   
                if ( fHistTop->GetBinContent( cChannelId ) > 0.5*fTotalEvents && ((cChannelId-1)%127)%8 != cTestPulseGroupId ) 
                {
                    cShortedChannelInfo = {0, (cChannelId-1), ((cChannelId-1)%127)%8, cTestPulseGroupId, 0};                
                    cShortedChannelsGroup.push_back(cShortedChannelInfo);
                    os<<"\t0\t|\t"<<cShortedChannelInfo[1]<<"\t|\t"<<cShortedChannelInfo[2]<<"\t|\t"<<cShortedChannelInfo[3]<<std::endl;
                }
                else if (fHistTop->GetBinContent( cChannelId ) < 0.5*fTotalEvents && ((cChannelId-1)%127)%8 == cTestPulseGroupId)
                {
                    cGroundedChannel = {0, (cChannelId-1)};
                    cGroundedChannelsList.push_back(cGroundedChannel);
                    os<<"\t0\t|\t"<<(cChannelId-1)<<"\t|\t"<<(cTestPulseGroupId)<<"\t|\tGND"<<std::endl;
                }
                
                if ( fHistBottom->GetBinContent( cChannelId ) > 0.5*fTotalEvents && ((cChannelId-1)%127)%8 != cTestPulseGroupId) 
                {
                    cShortedChannelInfo = {1, (cChannelId-1), ((cChannelId-1)%127)%8, cTestPulseGroupId, 0};
                    cShortedChannelsGroup.push_back(cShortedChannelInfo);
                    os<<"\t1\t|\t"<<cShortedChannelInfo[1]<<"\t|\t"<<cShortedChannelInfo[2]<<"\t|\t"<<cShortedChannelInfo[3]<<std::endl;
                }
                else if (fHistBottom->GetBinContent( cChannelId ) < 0.5*fTotalEvents && ((cChannelId-1)%127)%8 == cTestPulseGroupId)
                {
                    cGroundedChannel = {1, (cChannelId-1)};
                    cGroundedChannelsList.push_back(cGroundedChannel);
                    os<<"\t1\t|\t"<<(cChannelId-1)<<"\t|\t"<<(cTestPulseGroupId)<<"\t|\tGND"<<std::endl;
                }
                
            }
            cShortedGroupsArray[cTestPulseGroupId] = cShortedChannelsGroup;
            fHistBottom->Reset();
            fHistTop->Reset();
            os<<"------------------------------------------------------------------------"<<std::endl;


        }
    }
    ReconstructShorts(cShortedGroupsArray, os );

    UpdateHists();
}

void ShortFinder::ReconstructShorts(ShortedGroupsList pShortedGroupsArray, std::ostream& os )
{

    LOG (INFO) << std::endl<<"---------Creating shorted pairs-----------------";
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

            cShort.push_back(temp_shorted_channel);
            cShort.push_back(temp_shorted_channel);
            
            index = 0;
            matching_channel_found = false;

            for ( auto cCandidate : pShortedGroupsArray[cShortedChannelInfo[2]] )
            {   
                if(cShortedChannelInfo[3] == cCandidate[2])
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

                pShortedGroupsArray[cShortedChannelInfo[2]].erase(pShortedGroupsArray[cShortedChannelInfo[2]].begin() + best_candidate_index);

                for (auto cMemberChannel: cShort)
                {
                    for (auto i: cMemberChannel) LOG (INFO) << i << ' ';
                } 
                os <<"smallest distance: "<<smallest_distance<<std::endl;
                //DisplayGroupsContent(pShortedGroupsArray);

                cShortsVector.push_back(cShort);
            }
            else os <<"ERROR: No matching channel found for detected short (watch the level of noise)!"<<std::endl;

            smallest_distance = 10000;
            cShort.clear();
        }
    }

    os <<"---------Merging shorts-------------------------"<<std::endl;
    index = cShortsVector.size();
    fNShorts = cShortsVector.size();
    os <<"Number of shorted connections found: "<< fNShorts <<std::endl;

    while ( index > 1 ) 
    {
        index--;
        sub_index = index;
        while (sub_index > 0)
        {
            sub_index--;
            if (CheckShortsConnection( cShortsVector[index], cShortsVector[sub_index]))
            {
                cShortsVector[sub_index] = MergeShorts( cShortsVector[index], cShortsVector[sub_index]);
                cShortsVector.erase(cShortsVector.begin() + index);
                break;
            }
        }  
    }

    os <<"---------Outcome--------------------------------"<< std::endl;
    fNShortsTop = 0 ;
    fNShortsBottom = 0; 
    for (auto someShort: cShortsVector)
    {
        for (auto cMemberChannel: someShort)
        {
            if( cMemberChannel[0] == 0 )
            {
                fHistShortsTop->Fill( cMemberChannel[1] , 1 );
                fNShortsTop++;
            }
            else if ( cMemberChannel[0] == 1 )
            {
                fHistShortsBottom->Fill( cMemberChannel[1] , 1 );
                fNShortsBottom++;
            }
            for (auto i: cMemberChannel)
            { 
                os  << i << ' ';
            }
        }
        os <<std::endl;
    }
}
bool ShortFinder::CheckShortsConnection(ShortsList pShortA, ShortsList pShortB)
{
    for (auto cChannel : pShortA)
    {
        if (CheckChannelInShortPresence(cChannel, pShortB)) return true;
    } 
    return false;
}

ShortsList ShortFinder::MergeShorts(ShortsList pShortA, ShortsList pShortB)
{
    for (auto cChannel : pShortA)
    {
        if (!CheckChannelInShortPresence(cChannel, pShortB))
        {
            pShortB.push_back(cChannel);
        }
    }

    for (auto cMemberChannel: pShortB)
    {
        for (auto i: cMemberChannel) LOG (INFO) << i << ' ';
    }
    LOG (INFO) <<std::endl;
    
    return pShortB;
}
bool ShortFinder::CheckChannelInShortPresence( Short pShortedChannel, ShortsList pShort)
{
    for (auto cChannel : pShort)
    {
        if (cChannel[0] == pShortedChannel[0] && cChannel[1] == pShortedChannel[1]) return true;
    }
    return false;
}
