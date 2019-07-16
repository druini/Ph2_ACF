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

void RD53SCurveHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure)
{
  auto hOcc2D = HistContainer<TH2F>("SCurves", "SCurves", nSteps, startValue, stopValue, nEvents / 2 + 1, 0, 1 + 2. / nEvents);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc2D, Occupancy2D, "#DeltaVCal", "Efficiency");

  auto hThreshold1D = HistContainer<TH1F>("Threshold1D", "Threshold Distribution", 1000, 0, 1000);
  bookImplementer(theOutputFile, theDetectorStructure, hThreshold1D, Threshold1D, "#DeltaVCal", "Entries");

  auto hNoise1D = HistContainer<TH1F>("Noise1D", "Noise Distribution", 100, 0, 30);
  bookImplementer(theOutputFile, theDetectorStructure, hNoise1D, Noise1D, "#DeltaVCal", "Entries");

  auto hThreshold2D = HistContainer<TH2F>("Threshold2D", "Threshold Map", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hThreshold2D, Threshold2D, "Column", "Row");

  auto hNoise2D = HistContainer<TH2F>("Noise2D", "Noise Map", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hNoise2D, Noise2D, "Column", "Row");
}

void RD53SCurveHistograms::fillOccupancy (const DetectorDataContainer& data, int VCAL_HIGH)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  auto* hOcc2D = Occupancy2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH2F>>().fTheHistogram;

	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      // if (this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col) == true)
		hOcc2D->Fill(VCAL_HIGH, cChip->getChannel<Occupancy>(row, col).fOccupancy);
	}
}

void RD53SCurveHistograms::fillThresholdNoise (const DetectorDataContainer& data)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  auto* Threshold1DHist = Threshold1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
	  auto* Noise1DHist     = Noise1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
	  auto* Threshold2DHist = Threshold2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
	  auto* Noise2DHist     = Noise2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;

	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      if (cChip->getChannel<ThresholdAndNoise>(row, col).fNoise != 0)
		{
		  Threshold1DHist->Fill(cChip->getChannel<ThresholdAndNoise>(row, col).fThreshold);
		  Noise1DHist->Fill(cChip->getChannel<ThresholdAndNoise>(row, col).fNoise);
		  Threshold2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<ThresholdAndNoise>(row, col).fThreshold);
		  Noise2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<ThresholdAndNoise>(row, col).fNoise);
		}
	}
}

void RD53SCurveHistograms::process ()
{
  draw<TH2F>(Occupancy2D, "gcolz", true, "Charge (electrons)");
  draw<TH1F>(Threshold1D, "", true, "Charge (electrons)");
  draw<TH1F>(Noise1D, "", true, "Charge (electrons)");
  draw<TH2F>(Threshold2D, "gcolz");
  draw<TH2F>(Noise2D, "gcolz");
}
