#pragma once

#include "../RootUtils/HistContainer.h"
#include "../RootUtils/RootContainerFactory.h"
#include "RD53HistogramsBase.h"
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>

class RD53ScurveHistograms : RD53HistogramsBase {
    DetectorDataContainer Scurves2D;
    DetectorDataContainer Threshold1D;
    DetectorDataContainer Noise1D;
    DetectorDataContainer Threshold2D;
    DetectorDataContainer Noise2D;

    int nEvents;
    int nSteps;
    int startValue;
    int stopValue;

public:
    RD53ScurveHistograms(int nEvents, int nSteps, int startValue, int stopValue)
        : nEvents(nEvents)
        , nSteps(nSteps)
        , startValue(startValue)
        , stopValue(stopValue)
    {}

    void book(TFile* theOutputFile, const DetectorContainer& theDetectorStructure)
    {

        book_impl(theOutputFile, theDetectorStructure,
                  HistContainer<TH2F>("Scurves", "Scurves", nSteps, startValue, stopValue,
                                      nEvents / 2 + 1, 0, 1 + 2. / nEvents),
                  Scurves2D, "#DeltaVCal", "Efficiency");

        book_impl(theOutputFile, theDetectorStructure, HistContainer<TH1F>("Threshold1D", "Threshold1D", 1000, 0, 1000),
                  Threshold1D, "Threshold (#DeltaVCal)", "Entries");

        book_impl(theOutputFile, theDetectorStructure, HistContainer<TH1F>("Noise1D", "Noise1D", 150, 0, 30), Noise1D,
                  "Noise (#DeltaVCal)", "Entries");

        book_impl(theOutputFile, theDetectorStructure,
                  HistContainer<TH2F>("Threshold2D", "Threshold2D", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows),
                  Threshold2D, "Columns", "Rows");

        book_impl(theOutputFile, theDetectorStructure,
                  HistContainer<TH2F>("Noise2D", "Noise2D", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows), Noise2D,
                  "Columns", "Rows");
    }

    void fill_occupancy(const DetectorDataContainer& data, int VCAL_HIGH)
    {
        for (const auto board : data) {
            for (const auto module : *board) {
                for (const auto chip : *module) {
                    auto* Scurves2DHist = Scurves2D.at(board->getIndex())
                                              ->at(module->getIndex())
                                              ->at(chip->getIndex())
                                              ->getSummary<HistContainer<TH2F> >()
                                              .fTheHistogram;
                    for (auto row = 0; row < RD53::nRows; row++) {
                        for (auto col = 0; col < RD53::nCols; col++) {
                            Scurves2DHist->Fill(VCAL_HIGH, chip->getChannel<Occupancy>(row, col).fOccupancy);
                        }
                    }
                }
            }
        }
    }

    void fill_threshold_noise(const DetectorDataContainer& data)
    {
        for (const auto board : data) {
            for (const auto module : *board) {
                for (const auto chip : *module) {
                    auto* Threshold1DHist = Threshold1D.at(board->getIndex())
                                                ->at(module->getIndex())
                                                ->at(chip->getIndex())
                                                ->getSummary<HistContainer<TH1F> >()
                                                .fTheHistogram;
                    auto* Noise1DHist = Noise1D.at(board->getIndex())
                                            ->at(module->getIndex())
                                            ->at(chip->getIndex())
                                            ->getSummary<HistContainer<TH1F> >()
                                            .fTheHistogram;

                    auto* Threshold2DHist = Threshold2D.at(board->getIndex())
                                                ->at(module->getIndex())
                                                ->at(chip->getIndex())
                                                ->getSummary<HistContainer<TH1F> >()
                                                .fTheHistogram;

                    auto* Noise2DHist = Noise2D.at(board->getIndex())
                                            ->at(module->getIndex())
                                            ->at(chip->getIndex())
                                            ->getSummary<HistContainer<TH1F> >()
                                            .fTheHistogram;


                    for (auto row = 0; row < RD53::nRows; row++) {
                        for (auto col = 0; col < RD53::nCols; col++) {
                            if (chip->getChannel<ThresholdAndNoise>(row, col).fNoise) {
                                Threshold1DHist->Fill(chip->getChannel<ThresholdAndNoise>(row, col).fThreshold);
                                Noise1DHist->Fill(chip->getChannel<ThresholdAndNoise>(row, col).fNoise);
                                Threshold2DHist->SetBinContent(row + 1, col + 1, chip->getChannel<ThresholdAndNoise>(row, col).fThreshold);
                                Noise2DHist->SetBinContent(row + 1, col + 1, chip->getChannel<ThresholdAndNoise>(row, col).fNoise);
                            }
                        }
                    }
                }
            }
        }
    }

    // should be called "draw"
    void process()
    {
        draw<TH2F>(Scurves2D);
        draw<TH1F>(Threshold1D);
        draw<TH1F>(Noise1D);
        draw<TH2F>(Threshold2D);
        draw<TH2F>(Noise2D);
    }
};