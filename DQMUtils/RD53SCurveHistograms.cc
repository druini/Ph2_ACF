/*!
  \file                  RD53SCurveHistograms.cc
  \brief                 Implementation of SCurve calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SCurveHistograms.h"

using namespace Ph2_HwDescription;

void SCurveHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap)
{
  // #######################
  // # Retrieve parameters #
  // #######################
  nEvents    = this->findValue(pSettingsMap,"nEvents");
  nSteps     = this->findValue(pSettingsMap,"VCalHnsteps");
  startValue = this->findValue(pSettingsMap,"VCalHstart");
  stopValue  = this->findValue(pSettingsMap,"VCalHstop");
  offset     = this->findValue(pSettingsMap,"VCalMED");


  auto hOcc2D = CanvasContainer<TH2F>("SCurves", "SCurves", nSteps, startValue-offset, stopValue-offset, nEvents + 1, 0, 1 + 1. / nEvents);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc2D, Occupancy2D, "#DeltaVCal", "Efficiency");

  auto hErrReadOut2D = CanvasContainer<TH2F>("ReadoutErrors", "Readout Errors", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hErrReadOut2D, ErrorReadOut2D, "Columns", "Rows");

  auto hErrFit2D = CanvasContainer<TH2F>("FitErrors", "Fit Errors", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hErrFit2D, ErrorFit2D, "Columns", "Rows");

  auto hThreshold1D = CanvasContainer<TH1F>("Threshold1D", "Threshold Distribution", 1000, startValue-offset, stopValue-offset);
  bookImplementer(theOutputFile, theDetectorStructure, hThreshold1D, Threshold1D, "Threshold (#DeltaVCal)", "Entries");

  auto hNoise1D = CanvasContainer<TH1F>("Noise1D", "Noise Distribution", 100, 0, 50);
  bookImplementer(theOutputFile, theDetectorStructure, hNoise1D, Noise1D, "Noise (#DeltaVCal)", "Entries");

  auto hThreshold2D = CanvasContainer<TH2F>("Threshold2D", "Threshold Map", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hThreshold2D, Threshold2D, "Column", "Row");

  auto hNoise2D = CanvasContainer<TH2F>("Noise2D", "Noise Map", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hNoise2D, Noise2D, "Column", "Row");
}

void SCurveHistograms::fillOccupancy (const DetectorDataContainer& data, int DELTA_VCAL)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* hOcc2D             = Occupancy2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;
          auto* ErrorReadOut2DHist = ErrorReadOut2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              {
                if (cChip->getChannel<OccupancyAndPh>(row, col).isEnabled == true)
                  hOcc2D->Fill(DELTA_VCAL, cChip->getChannel<OccupancyAndPh>(row, col).fOccupancy + hOcc2D->GetYaxis()->GetBinWidth(0) / 2.);
                if (cChip->getChannel<OccupancyAndPh>(row, col).readoutError == true) ErrorReadOut2DHist->Fill(col + 1, row + 1);
              }
        }
}

void SCurveHistograms::fill (const DetectorDataContainer& data)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* Threshold1DHist = Threshold1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* Noise1DHist     = Noise1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* Threshold2DHist = Threshold2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* Noise2DHist     = Noise2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* ErrorFit2DHist  = ErrorFit2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              if (cChip->getChannel<ThresholdAndNoise>(row, col).fitError == true) ErrorFit2DHist->Fill(col + 1, row + 1);
              else if (cChip->getChannel<ThresholdAndNoise>(row, col).fNoise != 0)
                {
                  Threshold1DHist->Fill(cChip->getChannel<ThresholdAndNoise>(row, col).fThreshold);
                  Noise1DHist->Fill(cChip->getChannel<ThresholdAndNoise>(row, col).fNoise);
                  Threshold2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<ThresholdAndNoise>(row, col).fThreshold);
                  Noise2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<ThresholdAndNoise>(row, col).fNoise);
                }
        }
}

void SCurveHistograms::process ()
{
  draw<TH2F>(Occupancy2D, "gcolz", true, "Charge (electrons)");
  draw<TH2F>(ErrorReadOut2D, "gcolz");
  draw<TH2F>(ErrorFit2D, "gcolz");
  draw<TH1F>(Threshold1D, "", true, "Threshold (electrons)");
  draw<TH1F>(Noise1D, "", true, "Noise (electrons)");
  draw<TH2F>(Threshold2D, "gcolz");
  draw<TH2F>(Noise2D, "gcolz");
}
