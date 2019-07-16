/*!
  \file                  RD53ScurveHistograms.cc
  \brief                 Implementation of SCurve calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ScurveHistograms.h"

using namespace Ph2_HwDescription;

void RD53ScurveHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure)
{
    size_t ToTsize   = RD53::SetBits(RD53EvtEncoder::NBIT_TOT / NPIX_REGION) + 1;
    size_t BCIDsize  = RD53::SetBits(RD53EvtEncoder::NBIT_BCID) + 1;
    size_t TrgIDsize = RD53::SetBits(RD53EvtEncoder::NBIT_TRIGID) + 1;

    auto hOcc2D = HistContainer<TH2F>("Scurves", "Scurves", nSteps, startValue, stopValue, nEvents / 2 + 1, 0, 1 + 2. / nEvents);
    bookImplementer(theOutputFile, theDetectorStructure, hOcc2D, Occupancy2D, "#DeltaVCal", "Efficiency");

    auto hThreshold1D = HistContainer<TH1F>("Threshold1D", "Threshold Distribution", 1000,0,1000);
    bookImplementer(theOutputFile, theDetectorStructure, hThreshold1D, Threshold1D, "#DeltaVCal", "Entries");

    auto hNoise1D = HistContainer<TH1F>("Noise1D", "Noise Distribution", 100, 0, 30);
    bookImplementer(theOutputFile, theDetectorStructure, hNoise1D, Noise1D, "#DeltaVCal", "Entries");

    auto hThreshold2D = HistContainer<TH2F>("Threshold2D", "Threshold Map", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
    bookImplementer(theOutputFile, theDetectorStructure, hThreshold2D, Threshold2D, "Column", "Row");

    auto hNoise2D = HistContainer<TH2F>("Noise2D", "Noise Map", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
    bookImplementer(theOutputFile, theDetectorStructure, hNoise2D, Noise2D, "Column", "Row");
}

void RD53ScurveHistograms::fillOccupancy(const DetectorDataContainer& data, int VCAL_HIGH)
{
    for (const auto board : data) {
        for (const auto module : *board) {
            for (const auto chip : *module) {
                auto* hOcc2D = Occupancy2D.at(board->getIndex())
                                            ->at(module->getIndex())
                                            ->at(chip->getIndex())
                                            ->getSummary<HistContainer<TH2F> >()
                                            .fTheHistogram;
                for (auto row = 0; row < RD53::nRows; row++) {
                    for (auto col = 0; col < RD53::nCols; col++) {
                        hOcc2D->Fill(VCAL_HIGH, chip->getChannel<Occupancy>(row, col).fOccupancy);
                    }
                }
            }
        }
    }
}

void RD53ScurveHistograms::fillThresholdNoise(const DetectorDataContainer& data)
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

void RD53ScurveHistograms::process ()
{
  draw<TH2F>(Occupancy2D, "gcolz", true, "Charge (electrons)");
  draw<TH1F>(Threshold1D, "", true, "Charge (electrons)");
  draw<TH1F>(Noise1D, "", true, "Charge (electrons)");
  draw<TH2F>(Threshold2D, "gcolz");
  draw<TH2F>(Noise2D, "gcolz");
}
