#pragma once

#include "RD53PixelAliveHistograms.h"

using namespace Ph2_HwDescription;

void RD53PixelAliveHistograms::book(TFile* theOutputFile, const DetectorContainer& theDetectorStructure)
{
    size_t ToTsize = RD53::SetBits(RD53EvtEncoder::NBIT_TOT / NPIX_REGION) + 1;
    size_t BCIDsize = RD53::SetBits(RD53EvtEncoder::NBIT_BCID) + 1;
    size_t TrgIDsize = RD53::SetBits(RD53EvtEncoder::NBIT_TRIGID) + 1;

    book_impl(theOutputFile, theDetectorStructure,
              HistContainer<TH2F>("PixelAlive", "Pixel alive", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows),
              Occupancy2D, "Columns", "Rows");

    book_impl(theOutputFile, theDetectorStructure, HistContainer<TH1F>("ToT", "ToT distribution", ToTsize, 0, ToTsize), ToT,
              "ToT", "Entries");

    book_impl(theOutputFile, theDetectorStructure, HistContainer<TH1F>("Occ1D", "Occ1D", nEvents + 1, 0, nEvents + 1),
              Occupancy1D, "Occupancy", "Entries");

    book_impl(theOutputFile, theDetectorStructure, HistContainer<TH1F>("BCID", "BCID", BCIDsize, 0, BCIDsize), BCID, "#DeltaBCID",
              "Entries");

    book_impl(theOutputFile, theDetectorStructure, HistContainer<TH1F>("TriggerID", "TriggerID", TrgIDsize, 0, TrgIDsize),
              TriggerID, "#DeltaTrigger-ID", "Entries");

    book_impl(theOutputFile, theDetectorStructure,
              HistContainer<TH2F>("Errors", "Errors", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows), Error, "Columns",
              "Rows");
}

void RD53PixelAliveHistograms::fill(const DetectorDataContainer& data)
{
    for (const auto board : data) {
        for (const auto module : *board) {
            for (const auto chip : *module) {
                auto* Occupancy2DHist = Occupancy2D.at(board->getIndex())
                                            ->at(module->getIndex())
                                            ->at(chip->getIndex())
                                            ->getSummary<HistContainer<TH2F> >()
                                            .fTheHistogram;
                auto* ToTHist = ToT.at(board->getIndex())
                                    ->at(module->getIndex())
                                    ->at(chip->getIndex())
                                    ->getSummary<HistContainer<TH1F> >()
                                    .fTheHistogram;
                auto* Occupancy1DHist = Occupancy1D.at(board->getIndex())
                                            ->at(module->getIndex())
                                            ->at(chip->getIndex())
                                            ->getSummary<HistContainer<TH1F> >()
                                            .fTheHistogram;

                auto* BCIDHist = BCID.at(board->getIndex())
                                     ->at(module->getIndex())
                                     ->at(chip->getIndex())
                                     ->getSummary<HistContainer<TH1F> >()
                                     .fTheHistogram;

                auto* TriggerIDHist = TriggerID.at(board->getIndex())
                                          ->at(module->getIndex())
                                          ->at(chip->getIndex())
                                          ->getSummary<HistContainer<TH1F> >()
                                          .fTheHistogram;

                auto* ErrHist = Error.at(board->getIndex())
                                    ->at(module->getIndex())
                                    ->at(chip->getIndex())
                                    ->getSummary<HistContainer<TH1F> >()
                                    .fTheHistogram;

                for (auto row = 0; row < RD53::nRows; row++) {
                    for (auto col = 0; col < RD53::nCols; col++) {
                        if (chip->getChannel<OccupancyAndPh>(row, col).fOccupancy != 0) {
                            Occupancy2DHist->SetBinContent(col + 1, row + 1,
                                                           chip->getChannel<OccupancyAndPh>(row, col).fOccupancy);
                            ToTHist->Fill(chip->getChannel<OccupancyAndPh>(row, col).fPh);
                            Occupancy1DHist->Fill(chip->getChannel<OccupancyAndPh>(row, col).fOccupancy * nEvents);
                        }
                        if (chip->getChannel<OccupancyAndPh>(row, col).fErrors != 0)
                            ErrHist->SetBinContent(col + 1, row + 1, chip->getChannel<OccupancyAndPh>(row, col).fErrors);
                    }
                }
                for (auto i = 1; i < chip->getSummary<GenericDataVector, OccupancyAndPh>().data1.size(); i++) {
                    int deltaBCID = chip->getSummary<GenericDataVector, OccupancyAndPh>().data1[i]
                                    - chip->getSummary<GenericDataVector, OccupancyAndPh>().data1[i - 1];

                    BCIDHist->Fill((deltaBCID > 0 ? 0 : RD53::SetBits(RD53EvtEncoder::NBIT_BCID) + 1) + deltaBCID);
                }

                for (auto i = 1; i < chip->getSummary<GenericDataVector, OccupancyAndPh>().data2.size(); i++) {
                    int deltaTrgID = chip->getSummary<GenericDataVector, OccupancyAndPh>().data2[i]
                                     - chip->getSummary<GenericDataVector, OccupancyAndPh>().data2[i - 1];

                    TriggerIDHist->Fill((deltaTrgID > 0 ? 0 : RD53::SetBits(RD53EvtEncoder::NBIT_TRIGID) + 1) + deltaTrgID);
                }
            }
        }
    }
}

void RD53PixelAliveHistograms::process()
{
    draw<TH2F>(Occupancy2D, "gcolz");
    draw<TH1F>(ToT);
    draw<TH1F>(Occupancy1D);
    draw<TH1F>(BCID);
    draw<TH1F>(TriggerID);
    draw<TH1F>(Error);
}