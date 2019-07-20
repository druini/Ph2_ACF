/*!
        \file                SLinkDQMHistogrammer.h
        \brief               base class to create and fill monitoring histograms
        \author              Suchandra Dutta and Subir Sarkar
        \version             1.0
        \date                13/10/15
        Support :            mail to : Suchandra.Dutta@cern.ch, Subir.Sarkar@cern.ch
*/

#include "SLinkDQMHistogrammer.h"
#include "DQMEvent.h"

#include "TROOT.h"
#include "TH1.h"
#include "TH2I.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TFile.h"
#include "TString.h"
#include "TDirectory.h"
#include "TPaveStats.h"
#include "TStyle.h"
#include "TIterator.h"



#ifdef __OTSDAQ__
using namespace ots;
#endif



SLinkDQMHistogrammer::SLinkDQMHistogrammer (int evtType) :
    eventType_ (evtType)
{
}

SLinkDQMHistogrammer::~SLinkDQMHistogrammer()
{
    TIter next (gROOT->GetList() );
    TObject* obj;

    while ( (obj = next() ) )
    {
        if (obj->InheritsFrom ("TH1") || obj->InheritsFrom ("TTree") ) delete obj;
    }
}

void SLinkDQMHistogrammer::bookHistograms (const std::vector<std::pair<uint8_t, std::vector<uint8_t> > >& fe_mapping)
{
    totalNumberHitsH_ = new TH1I ("tot_hits", "Total Number of Hits", 101, -0.5, 100.5);
    totalNumberStubsH_ = new TH1I ("tot_stubs", "Total Number of Stubs", 101, -0.5, 100.5);

    size_t ntot = 0;

    for ( auto const& it : fe_mapping )
    {
        uint8_t feId = it.first;
        std::vector<uint8_t> rous = it.second;
        uint8_t nrou = rous.size();
        //book FEU histograms
        bookFEUHistograms (feId, nrou);

        for (size_t irou = 0; irou < nrou; irou++)
        {
            uint8_t rouId = rous[irou];
            LOG (INFO) << " fedId " << +feId << " rouId " << +rouId  << " ROUs " << +nrou  << std::endl;
            ntot++;
            // Book ROU label histograms
            bookROUHistograms (feId, rouId);
        }
    }
}
void SLinkDQMHistogrammer::fillHistograms (const std::vector<DQMEvent*>& event_list)
{
    unsigned long ival = 0; // as the files is read in chunks

    std::vector<uint8_t> rouErrs;
    std::vector<uint16_t> rouPLAdds;
    std::vector<uint16_t> rouL1Counters;
    std::vector<uint8_t> evenChannels;
    std::vector<uint8_t> oddChannels;

    for ( const auto& ev : event_list )
    {
        ival++;
        //std::cout << " Event # " << ival << std::endl;

        // Read Condition Data
        readConditionData (ev);

        totalHits_ = 0;
        totalStubs_ = 0;

        FEUHistos feu_h;
        uint8_t nRouFe = 0;
        uint8_t nColFe = 0;
        uint16_t xchn;

        uint8_t nrou = 0;

        for (auto& it : rouHMap_ )
        {
            uint16_t cKey = it.first;
            ROUHistos rou_h = it.second;

            uint8_t feId = (cKey >> 8) & 0xFF;
            uint8_t rouId = cKey & 0xFF;

            nrou++;

            if (!rou_h.bookedHistos) continue;

            //      std::cout << " Filling ROU histos for RouId " << +rouId << " in FeId " << +feId << std::endl;

            if (nrou == 1)
            {
                std::map<uint8_t,  FEUHistos >::iterator fPos = feuHMap_.find (feId);

                if (fPos != feuHMap_.end() )
                {
                    feu_h = fPos->second;
                    nRouFe = feu_h.nROUs_;
                    nColFe = feu_h.nColumns_;
                }
                else nRouFe = 0;
            }

            if (nRouFe == 0) continue;

            // get Readout Data
            const std::pair<ReadoutStatus, std::vector<bool>>& readoutData = ev->trkPayload().readoutData (cKey);

            // Fill Readout Status
            const ReadoutStatus& rs = readoutData.first;

            uint8_t error_rou = 0;

            if (rs.error2() ) error_rou = 1;

            if (rs.error1() ) error_rou |= 1 << 1;

            uint16_t pladdress_rou = rs.PA();
            rouErrs.push_back (error_rou);
            rouPLAdds.push_back (pladdress_rou);
            rouL1Counters.push_back (rs.l1ACounter() );

            // Fill Readout channel data
            const std::vector<bool>& channelData = readoutData.second;
            uint32_t nhits_rou = 0;

            for ( size_t i = 0; i != channelData.size(); i++)
            {
                if ( !channelData[i] ) continue;

                nhits_rou++;

                if ( nColFe == 1 || (rouId <= (nRouFe / 2 - 1) ) ) xchn = 254 * rouId + i;
                else  xchn = (nrou - 1) * 254 - ( (254 * rouId) + i);

                fillROUHistograms (rou_h, i);

                uint8_t ichan = xchn / 2 + 1;

                //std::cout << " i " << i  << " xchn " << xchn << " ichan " << +ichan << std::endl;
                if ( (xchn % 2) == 0) evenChannels.push_back (ichan);
                else oddChannels.push_back (ichan);
            }

            rou_h.plAddH_->Fill (pladdress_rou);
            rou_h.errBitH_->Fill (error_rou);
            rou_h.hitVsVCTHH_->Fill (rou_h.vcthSetting_, nhits_rou);
            totalHits_ += nhits_rou;

            if (nrou == nRouFe)
            {
                //std::cout << " filling FEU histos for FeId " << +feId << " " << evenChannels.size() << "  " << oddChannels.size() << std::endl;
                fillFEUHistograms (feu_h, evenChannels, oddChannels, rouErrs, rouPLAdds, rouL1Counters);
                // stub data
                const std::vector<StubInfo>&  stubInfos = ev->stubData().stubs (feId);
                fillStubInformation (feu_h, stubInfos);
                totalStubs_ += stubInfos.size();
                evenChannels.clear();
                oddChannels.clear();
                rouErrs.clear();
                rouPLAdds.clear();
                rouL1Counters.clear();
                nrou = 0;
            }
        }

        if (totalNumberHitsH_) totalNumberHitsH_->Fill (totalHits_);

        if (totalNumberStubsH_) totalNumberStubsH_->Fill (totalStubs_);
    }
}
void SLinkDQMHistogrammer::fillHistograms (const DQMEvent* event)
{

    std::vector<uint8_t> rouErrs;
    std::vector<uint16_t> rouPLAdds;
    std::vector<uint16_t> rouL1Counters;
    std::vector<uint8_t> evenChannels;
    std::vector<uint8_t> oddChannels;

    //std::cout << " Event # " << ival << std::endl;

    // Read Condition Data
    readConditionData (event);

    totalHits_ = 0;
    totalStubs_ = 0;

    FEUHistos feu_h;
    uint8_t nRouFe = 0;
    uint8_t nColFe = 0;
    uint16_t xchn;

    uint8_t nrou = 0;

    for (auto& it : rouHMap_ )
    {
        uint16_t cKey = it.first;
        ROUHistos rou_h = it.second;

        uint8_t feId = (cKey >> 8) & 0xFF;
        uint8_t rouId = cKey & 0xFF;

        nrou++;

        if (!rou_h.bookedHistos) continue;

        //      std::cout << " Filling ROU histos for RouId " << +rouId << " in FeId " << +feId << std::endl;

        if (nrou == 1)
        {
            std::map<uint8_t,  FEUHistos >::iterator fPos = feuHMap_.find (feId);

            if (fPos != feuHMap_.end() )
            {
                feu_h = fPos->second;
                nRouFe = feu_h.nROUs_;
                nColFe = feu_h.nColumns_;
            }
            else nRouFe = 0;
        }

        if (nRouFe == 0) continue;

        // get Readout Data
        const std::pair<ReadoutStatus, std::vector<bool>>& readoutData = event->trkPayload().readoutData (cKey);

        // Fill Readout Status
        const ReadoutStatus& rs = readoutData.first;

        uint8_t error_rou = 0;

        if (rs.error2() ) error_rou = 1;

        if (rs.error1() ) error_rou |= 1 << 1;

        uint16_t pladdress_rou = rs.PA();
        rouErrs.push_back (error_rou);
        rouPLAdds.push_back (pladdress_rou);
        rouL1Counters.push_back (rs.l1ACounter() );

        // Fill Readout channel data
        const std::vector<bool>& channelData = readoutData.second;
        uint32_t nhits_rou = 0;

        for ( size_t i = 0; i != channelData.size(); i++)
        {
            if ( !channelData[i] ) continue;

            nhits_rou++;

            if ( nColFe == 1 || (rouId <= (nRouFe / 2 - 1) ) ) xchn = 254 * rouId + i;
            else  xchn = (nrou - 1) * 254 - ( (254 * rouId) + i);

            fillROUHistograms (rou_h, i);

            uint8_t ichan = xchn / 2 + 1;

            //std::cout << " i " << i  << " xchn " << xchn << " ichan " << +ichan << std::endl;
            if ( (xchn % 2) == 0) evenChannels.push_back (ichan);
            else oddChannels.push_back (ichan);
        }

        rou_h.plAddH_->Fill (pladdress_rou);
        rou_h.errBitH_->Fill (error_rou);
        rou_h.hitVsVCTHH_->Fill (rou_h.vcthSetting_, nhits_rou);
        totalHits_ += nhits_rou;

        if (nrou == nRouFe)
        {
            //std::cout << " filling FEU histos for FeId " << +feId << " " << evenChannels.size() << "  " << oddChannels.size() << std::endl;
            fillFEUHistograms (feu_h, evenChannels, oddChannels, rouErrs, rouPLAdds, rouL1Counters);
            // stub data
            const std::vector<StubInfo>&  stubInfos = event->stubData().stubs (feId);
            fillStubInformation (feu_h, stubInfos);
            totalStubs_ += stubInfos.size();
            evenChannels.clear();
            oddChannels.clear();
            rouErrs.clear();
            rouPLAdds.clear();
            rouL1Counters.clear();
            nrou = 0;
        }

        if (totalNumberHitsH_) totalNumberHitsH_->Fill (totalHits_);

        if (totalNumberStubsH_) totalNumberStubsH_->Fill (totalStubs_);
    }
}
void SLinkDQMHistogrammer::fillROUHistograms (ROUHistos& rou_h, uint8_t ch)
{
    uint8_t ichan = ch / 2 + 1;

    if ( (ch % 2) == 0) rou_h.evenChanOccuH_->Fill (ichan);
    else rou_h.oddChanOccuH_->Fill (ichan );
}
void SLinkDQMHistogrammer::fillFEUHistograms (FEUHistos& feu_h,
        std::vector<uint8_t>& even_list,
        std::vector<uint8_t>& odd_list,
        std::vector<uint8_t>& errs,
        std::vector<uint16_t>& adds,
        std::vector<uint16_t>& counts)
{
    if (!feu_h.bookedHistos) return;

    uint8_t nrou = feu_h.nROUs_;
    uint8_t col = feu_h.nColumns_;

    for (uint8_t i = 0; i != even_list.size(); i++)
    {
        feu_h.sen0HitProfH_->Fill (even_list[i], col - 1);
        feu_h.sen0C0HitProfH_->Fill (even_list[i]);

        if (col == 2) feu_h.sen0C1HitProfH_->Fill (even_list[i]);

        feu_h.sen0HitProfUnfoldedH_->Fill ( (col - 1) * 127 * nrou / 2 + even_list[i]);

        if (even_list.size() == odd_list.size() )
        {
            feu_h.hitCorrC0H_->Fill (even_list[i], odd_list[i]);
            feu_h.deltaHitC0H->Fill (even_list[i] - odd_list[i]);

            if (col == 2)
            {
                feu_h.hitCorrC1H_->Fill (even_list[i], odd_list[i]);
                feu_h.deltaHitC1H->Fill (even_list[i] - odd_list[i]);
            }
        }
    }

    for (uint8_t i = 0; i <  odd_list.size(); i++)
    {
        feu_h.sen1HitProfH_->Fill (odd_list[i], col - 1);
        feu_h.sen1C0HitProfH_->Fill (odd_list[i]);

        if (col == 2) feu_h.sen1C1HitProfH_->Fill (odd_list[i]);

        feu_h.sen1HitProfUnfoldedH_->Fill ( (col - 1) * 127 * 8 + odd_list[i]);
    }

    if (!even_list.size() && !odd_list.size() )      feu_h.senCorrH_->Fill (1);
    else if (!even_list.size() && !odd_list.size() ) feu_h.senCorrH_->Fill (2);
    else if (even_list.size() && !odd_list.size() )  feu_h.senCorrH_->Fill (3);
    else if (even_list.size() && odd_list.size() )   feu_h.senCorrH_->Fill (4);

    for (size_t ir = 0; ir !=  errs.size(); ir++) feu_h.rouErrorH_->Fill (ir * 4 + errs[ir]);

    for (size_t ip1 = 0; ip1 !=  adds.size(); ip1++)
    {
        for (size_t ip2 = 0; ip2 != adds.size(); ip2++)
        {
            if (ip1 == ip2) continue;

            uint32_t phase = (adds[ip1] - adds[ip2] + 511) % 511;
            feu_h.PLAddPhaseDiffH_->Fill (phase);
            feu_h.PLAddPhaseCorrH_->Fill (adds[ip1], adds[ip2]);
        }
    }

    for (size_t ic1 = 0; ic1 !=  counts.size(); ic1++)
    {
        for (size_t ic2 = 0; ic2 != counts.size(); ic2++)
        {
            if (ic1 == ic2) continue;

            feu_h.L1CounterDiffH_->Fill (counts[ic1] - counts[ic2]);
        }
    }
}
void SLinkDQMHistogrammer::saveHistograms (const std::string& out_file)
{
    TFile* fout = TFile::Open ( out_file.c_str(), "RECREATE" );

    // sensor plots
    for ( auto& imap : feuHMap_ )
    {
        uint8_t feId = imap.first;
        TString name = " FeId_";
        name += feId;

        //std::cout << " Saving Histograms for  FeId  " << +feId <<  " in => " << name.Data() << std::endl;

        fout->mkdir (name);
        fout->cd (name);
        FEUHistos feu_h = imap.second;

        if (feu_h.bookedHistos)
        {
            feu_h.hitCorrC0H_->Write();
            feu_h.deltaHitC0H->Write();

            feu_h.sen0HitProfH_->Write();
            feu_h.sen1HitProfH_->Write();

            feu_h.sen0HitProfUnfoldedH_->Write();
            feu_h.sen1HitProfUnfoldedH_->Write();

            feu_h.sen0C0HitProfH_->Write();
            feu_h.sen1C0HitProfH_->Write();

            if (feu_h.nColumns_ == 2)
            {
                feu_h.hitCorrC1H_->Write();
                feu_h.deltaHitC1H->Write();
                feu_h.sen0C1HitProfH_->Write();
                feu_h.sen1C1HitProfH_->Write();
            }

            feu_h.L1CounterDiffH_->Write();
            feu_h.PLAddPhaseDiffH_->Write();
            feu_h.PLAddPhaseCorrH_->Write();
            feu_h.rouErrorH_->Write();

            feu_h.senCorrH_->Write();
            feu_h.stubCountH_->Write();
            feu_h.stubPositionH_->Write();
            feu_h.stubBendH_->Write();

            feu_h.stubVsHVH_->Write();
        }
    }

    // ROU sections
    fout->cd();

    for ( auto& imap : rouHMap_ )
    {
        uint8_t key = imap.first;

        uint8_t feId = (key >> 8) & 0xFF;
        uint8_t rouId = key & 0xFF;
        //std::cout << " Save  for FeId  RouId " << +feId << " " << +rouId << std::endl;

        TString name = " FeId_";
        name += feId;
        name += "/";
        name += "RouId_";
        name +=  rouId;
        //std::cout << " Saving Histograms for  FeId  " << +feId << " RouId " << +rouId<< " in => " << name.Data() << std::endl;

        fout->mkdir (name);
        fout->cd (name);
        ROUHistos rou_h = imap.second;

        if (rou_h.bookedHistos)
        {
            rou_h.errBitH_->Write();
            rou_h.plAddH_->Write();
            rou_h.evenChanOccuH_->Write();
            rou_h.oddChanOccuH_->Write();

            rou_h.hitVsVCTHH_->Write();
        }
    }

    // common ones
    fout->cd();

    if (totalNumberHitsH_) totalNumberHitsH_->Write();

    if (totalNumberStubsH_) totalNumberStubsH_->Write();

    fout->Close();
}
void SLinkDQMHistogrammer::resetHistograms()
{
    TIter next (gROOT->GetList() );
    TObject* obj;

    while ( (obj = next() ) ) if (obj->InheritsFrom ("TH1") ) dynamic_cast<TH1*> (obj)->Reset();
}
void SLinkDQMHistogrammer::bookFEUHistograms (uint8_t fe_id, uint8_t n_rou)
{
    TString ss_feu = "FEU_";
    ss_feu += fe_id;
    FEUHistos feu_h;

    uint16_t nbin;
    uint8_t ncol;

    if (n_rou == 2)
    {
        ncol = 1;
        nbin = n_rou * 127;
    }
    else
    {
        ncol = 2;
        nbin = n_rou / 2 * 127;
    }

    feu_h.senCorrH_ = new TH1I ("SensorHitCount_" + ss_feu, "SensorHitCount_" + ss_feu, 4, 0.5, 4.5 );
    feu_h.senCorrH_->GetXaxis()->SetBinLabel (1, "No hits");
    feu_h.senCorrH_->GetXaxis()->SetBinLabel (2, "Even & !Odd");
    feu_h.senCorrH_->GetXaxis()->SetBinLabel (3, "Odd & !Even");
    feu_h.senCorrH_->GetXaxis()->SetBinLabel (4, "Even & Odd");

    feu_h.sen0HitProfH_ = new TH2I ("Sensor0HitProfile_" + ss_feu, "Sensor0HitProfile_" + ss_feu, nbin, -0.5, nbin - 0.5, ncol, -0.5, ncol - 0.5);
    feu_h.sen0HitProfH_->SetStats (false);
    feu_h.sen1HitProfH_ = new TH2I ("Sensor1HitProfile_" + ss_feu, "Sensor1HitProfile_" + ss_feu, nbin, -0.5, nbin - 0.5, ncol, -0.5, ncol - 0.5);
    feu_h.sen1HitProfH_->SetStats (false);
    feu_h.hitCorrC0H_ = new TH2I ("HitCorr_col0_" + ss_feu, "HitCorr_col0_" + ss_feu, nbin, -0.5, nbin - 0.5, nbin, -0.5, nbin - 0.5);
    feu_h.hitCorrC0H_->SetStats (false);

    feu_h.deltaHitC0H = new TH1D ("HitDifference_col0_" + ss_feu, "HitDifference_col0_" + ss_feu, 2 * nbin,  -nbin, nbin);

    feu_h.sen0HitProfUnfoldedH_ = new TH1I ("Sensor0_HitProf_Unfold_" + ss_feu, "Sensor0_HitProf_Unfold_" + ss_feu, nbin * ncol, -0.5, nbin * ncol - 0.5);
    feu_h.sen1HitProfUnfoldedH_ = new TH1I ("Sensor1_HitProf_Unfold_" + ss_feu, "Sensor1_HitProf_Unfold_" + ss_feu, nbin * ncol, -0.5, nbin * ncol - 0.5);

    feu_h.sen0C0HitProfH_ = new TH1I ("Sensor0_HitProf_col0_" + ss_feu, "Sensor0_HitProf_col0_" + ss_feu, nbin, -0.5, nbin - 0.5);
    feu_h.sen1C0HitProfH_ = new TH1I ("Sensor1_HitProf_col0_" + ss_feu, "Sensor1_HitProf_col0_" + ss_feu, nbin, -0.5, nbin - 0.5);

    feu_h.L1CounterDiffH_  = new TH1I ("L1CounterDiff_" + ss_feu, "L1CounterDiff_" + ss_feu, 1023, -511.5, 511.5);
    feu_h.PLAddPhaseDiffH_ = new TH1I ("PipeLineAddPhasedDiff_" + ss_feu, "PipeLineAddPhasedDiff_" + ss_feu, 1023, -511.5, 511.5);
    feu_h.PLAddPhaseCorrH_ = new TH2I ("PipeLineAddPhaseCorr_" + ss_feu, "PipeLineAddPhaseCorr_" + ss_feu, 511, -0.5, 510.5, 511, -0.5, 510.5);
    feu_h.PLAddPhaseCorrH_->SetStats (false);
    feu_h.rouErrorH_  = new TH1I ("ROUError_" + ss_feu, "ROUError_" + ss_feu, n_rou * 4, 0.0, n_rou * 4);
    feu_h.rouErrorH_->GetXaxis()->SetNdivisions (4 * 100);

    for (size_t i = 0; i < n_rou; i++)
    {
        TString tag = "Chip";
        tag += i;
        tag += "(";

        feu_h.rouErrorH_->GetXaxis()->SetBinLabel ( (i * 4 + 1), tag + "0)");
        feu_h.rouErrorH_->GetXaxis()->SetBinLabel ( (i * 4 + 2), tag + "1)");
        feu_h.rouErrorH_->GetXaxis()->SetBinLabel ( (i * 4 + 3), tag + "2)");
        feu_h.rouErrorH_->GetXaxis()->SetBinLabel ( (i * 4 + 4), tag + "3)");
    }

    if (ncol == 2)
    {

        feu_h.hitCorrC1H_ = new TH2I ("HitCorr_col1_" + ss_feu, "HitCorr_col1_" + ss_feu, nbin, -0.5, nbin - 0.5, nbin, -0.5, nbin - 0.5);
        feu_h.hitCorrC1H_->SetStats (false);

        feu_h.deltaHitC1H = new TH1D ("HitDifference_col1_" + ss_feu, "HitDifference_col1_" + ss_feu, 2 * nbin, -nbin, nbin);

        feu_h.sen0C1HitProfH_ = new TH1I ("Sensor0_HitProf_col1_" + ss_feu, "Sensor0_HitProf_col0_" + ss_feu, nbin, -0.5, nbin - 0.5);
        feu_h.sen1C1HitProfH_ = new TH1I ("Sensor1_HitProf_col1_" + ss_feu, "Sensor1_HitProf_col0_" + ss_feu, nbin, -0.5, nbin - 0.5);
    }
    else
    {
        feu_h.hitCorrC1H_ = nullptr;
        feu_h.deltaHitC1H = nullptr;
        feu_h.sen0C1HitProfH_ = nullptr;
        feu_h.sen1C1HitProfH_ = nullptr;
    }

    feu_h.stubCountH_ = new TH1I ("StubCount_" + ss_feu, "StubCount_" + ss_feu, 4 * n_rou, 0.0, n_rou * 4);
    feu_h.stubCountH_->GetXaxis()->SetNdivisions (4 * 100);

    for (size_t i = 0; i < n_rou; i++)
    {
        TString tag = "chip";
        tag += i;
        tag += "(";
        feu_h.stubCountH_->GetXaxis()->SetBinLabel ( (i * 4 + 1), tag + "0)");
        feu_h.stubCountH_->GetXaxis()->SetBinLabel ( (i * 4 + 2), tag + "1)");
        feu_h.stubCountH_->GetXaxis()->SetBinLabel ( (i * 4 + 3), tag + "2)");
        feu_h.stubCountH_->GetXaxis()->SetBinLabel ( (i * 4 + 4), tag + "3)");
    }

    feu_h.stubPositionH_ = new TH1F ("StubPosition" + ss_feu, "StubPosition" + ss_feu, 2 * nbin * ncol, -0.25, nbin * ncol - 0.25);
    feu_h.stubBendH_ = new TH1I ("StubBend_" + ss_feu, "StubBend_" + ss_feu, 31, -15.5, 15.5);
    feu_h.stubVsHVH_ = new TProfile ("StubCountVsHV_" + ss_feu, "StubCountVsHV_" + ss_feu, 100, 0.0, 500.0, 0.0, 100.0);

    feu_h.nROUs_ = n_rou;
    feu_h.nColumns_ = ncol;
    feu_h.hvSetting_ = 0;
    feu_h.bookedHistos = true;
    feuHMap_.insert ({fe_id, feu_h});
    //std::cout << " Booked Histograms for  FeId  " << +fe_id <<  " and inserted in FEUMap with key " << fe_id << " Tag " << ss_feu.Data() << std::endl;
}
void SLinkDQMHistogrammer::bookROUHistograms (uint8_t fe_id, uint8_t i_rou)
{
    TString ss_rou = "FEU_";
    ss_rou += fe_id;
    ss_rou += "_ROU_";
    ss_rou += i_rou;

    ROUHistos rou_h;

    rou_h.errBitH_           = new TH1I ("ErrorBit_" + ss_rou, "ErrotBit_" + ss_rou, 6, -0.5, 5.5 );
    rou_h.plAddH_            = new TH1I ("PipeLineAdd_" + ss_rou, "PipeLineAdd_" + ss_rou, 256, -0.5, 255.5 );
    rou_h.evenChanOccuH_     = new TH1I ("EvenChannelOccupancy_" + ss_rou, "EvenChannelOccupancy_" + ss_rou, 127, 0.5, 127.5 );
    rou_h.oddChanOccuH_      = new TH1I ("OddChannelOccupancy_" + ss_rou, "OddChannelOccupancy_" + ss_rou, 127, 0.5, 127.5 );
    rou_h.hitVsVCTHH_        = new TProfile ("HitCountVsVCTH_" + ss_rou, "HitCountVsVCTH_" + ss_rou, 500, 0.0, 500.0, 0.0, 300.);
    rou_h.vcthSetting_       = 0;
    rou_h.bookedHistos = true;

    uint16_t key = fe_id << 8 | i_rou;

    rouHMap_.insert ({key, rou_h});
    //std::cout << " Booked Histograms for  FeId  " << +fe_id <<  " RouId " << +i_rou << " and inserted in ROUMap with key " << key << " Tag " << ss_rou.Data() <<std::endl;
}
void SLinkDQMHistogrammer::fillStubInformation (FEUHistos& feu_h, const std::vector<StubInfo>& stubs)
{
    if (!feu_h.bookedHistos) return;

    uint8_t nrou = feu_h.nROUs_;
    std::vector<uint8_t> scount;

    for (size_t i = 0; i < nrou; i++) scount.push_back (0);

    uint8_t ntot_stub = 0;

    for ( const auto& istub : stubs)
    {
        if (istub.chipId() < scount.size() ) scount[istub.chipId()]++;

        ntot_stub++;
        feu_h.stubPositionH_->Fill (istub.chipId() * 127 + istub.address() / 2.);
        feu_h.stubBendH_->Fill (istub.bend() );
    }

    for (size_t i = 0; i < nrou; i++) feu_h.stubCountH_->Fill (i * 4 + scount[i]);

    feu_h.stubVsHVH_->Fill (feu_h.hvSetting_, ntot_stub);
}
void SLinkDQMHistogrammer::readConditionData (const DQMEvent* evt)
{
    for (size_t i = 0; i < evt->condData().size(); i++)
    {
        const std::tuple<uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, size_t>& data = evt->condData().data (i);
        uint8_t i2cPage = std::get<2> (data);
        uint8_t i2cReg = std::get<3> (data);
        uint8_t uid = std::get<4> (data);

        // Get HV status
        if (uid == 5)
        {
            uint8_t feId = std::get<0> (data);
            std::map<uint8_t,  FEUHistos >::iterator fPos = feuHMap_.find (feId);

            if (fPos != feuHMap_.end() )
            {
                fPos->second.hvSetting_ = std::get<5> (data);;
            }
        }

        // Get VCTH status
        if (uid == 1 && i2cPage == 0 && i2cReg == 0X4F)
        {
            uint8_t feId = std::get<0> (data);
            uint8_t rouId = std::get<1> (data);
            uint16_t key = feId << 8 | rouId;
            std::map<uint16_t,  ROUHistos >::iterator fPos = rouHMap_.find (key);

            if (fPos != rouHMap_.end() )
            {
                fPos->second.vcthSetting_ = std::get<5> (data);;
                //std::cout << " FeId " << feId << " ROU Id " << rouId << " VCTH Status " << std::get<5>(data) << std::endl;
            }
        }
    }
}
