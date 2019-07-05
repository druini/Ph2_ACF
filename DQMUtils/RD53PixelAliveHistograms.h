#pragma once

#include "../RootUtils/HistContainer.h"
#include "../RootUtils/RootContainerFactory.h"
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>

class RD53PixelAliveHistograms {
    DetectorDataContainer Occupancy2D;
    DetectorDataContainer ToT;
    DetectorDataContainer Occupancy1D;
    DetectorDataContainer BCID;
    DetectorDataContainer TriggerID;
    DetectorDataContainer Error;

    int canvas_id = 0;
    std::vector<std::unique_ptr<TCanvas> > canvases;

    int nEvents;

public:
    RD53PixelAliveHistograms(int nEvents)
        : nEvents(nEvents)
    {
    }

    void book(TFile* theOutputFile, const DetectorContainer& theDetectorStructure)
    {

        book_impl(theOutputFile, theDetectorStructure,
                  HistContainer<TH2F>("PixelAlive", "Pixel alive", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows),
                  Occupancy2D, "Columns", "Rows");

        book_impl(theOutputFile, theDetectorStructure, HistContainer<TH1F>("ToT", "ToT distribution", 16, 0, 16), ToT, "ToT",
                  "Entries");

        book_impl(theOutputFile, theDetectorStructure, HistContainer<TH1F>("Occ1D", "Occ1D", nEvents + 1, 0, nEvents + 1),
                  Occupancy1D, "Occupancy", "Entries");

        book_impl(
            theOutputFile, theDetectorStructure,
            HistContainer<TH1F>("BCID", "BCID", 1 << (RD53EvtEncoder::NBIT_BCID + 1), 0, 1 << (RD53EvtEncoder::NBIT_BCID + 1)),
            BCID, "#DeltaBCID", "Entries");

        book_impl(theOutputFile, theDetectorStructure,
                  HistContainer<TH1F>("TriggerID", "TriggerID", 1 << (RD53EvtEncoder::NBIT_TRIGID + 1), 0,
                                      1 << (RD53EvtEncoder::NBIT_TRIGID + 1)),
                  TriggerID, "#DeltaTrigger-ID", "Entries");

        book_impl(theOutputFile, theDetectorStructure,
                  HistContainer<TH2F>("Errors", "Errors", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows), Error,
                  "Columns", "Rows");
    }

    void fill(const DetectorDataContainer& data)
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

    // should be called "draw"
    void process()
    {
        draw<TH2F>(Occupancy2D);
        draw<TH1F>(ToT);
        draw<TH1F>(Occupancy1D);
    }

private:
    template <class Hist>
    void book_impl(TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const HistContainer<Hist>& hist_container,
                   DetectorDataContainer& data_container, const char* XTitle = nullptr, const char* YTitle = nullptr)
    {
        if (XTitle)
            hist_container.fTheHistogram->SetXTitle(XTitle);
        if (YTitle)
            hist_container.fTheHistogram->SetYTitle(YTitle);

        RootContainerFactory theRootFactory;
        theRootFactory.bookChipHistrograms(theOutputFile, theDetectorStructure, data_container, hist_container);
    }

    template <class Hist> void draw(DetectorDataContainer& HistDataContainer)
    {
        for (auto board : HistDataContainer) {
            for (auto module : *board) {
                for (auto chip : *module) {
                    canvases.emplace_back(
                        new TCanvas(("hist_canvas_" + std::to_string(canvas_id++)).c_str(), "canvas")); //, 0, 0, 1000, 700));
                    canvases.back()->cd();
                    chip->getSummary<HistContainer<Hist> >().fTheHistogram->Draw();
                }
            }
        }
    }
};