/*!
  \file                  RD53GainHistograms.cc
  \brief                 Implementation of Gain calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53GainHistograms.h"


// #############
// # CONSTANTS #
// #############
#define INTERCEPT_HALFRANGE 6 // [ToT]

using namespace Ph2_HwDescription;

void RD53GainHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, uint32_t> pSettingsMap)
{
  auto hOcc2D = HistContainer<TH2F>("Gain", "Gain", nSteps,startValue,stopValue, nEvents / 2, 0, RD53::setBits(RD53EvtEncoder::NBIT_TOT / NPIX_REGION));
  bookImplementer(theOutputFile, theDetectorStructure, hOcc2D, Occupancy2D, "#DeltaVCal", "ToT");

  auto hGain1D = HistContainer<TH1F>("Gain1D", "Gain1D", 100, 0, 20e-3);
  bookImplementer(theOutputFile, theDetectorStructure, hGain1D, Gain1D, "Gain (ToT/VCal)", "Entries");

  auto hIntercept1D = HistContainer<TH1F>("Intercept1D", "Intercept1D", 100, -INTERCEPT_HALFRANGE, INTERCEPT_HALFRANGE);
  bookImplementer(theOutputFile, theDetectorStructure, hIntercept1D, Intercept1D, "Intercept (ToT)", "Entries");

  auto hGain2D = HistContainer<TH2F>("Gain2D", "Gain Map", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hGain2D, Gain2D, "Column", "Row");

  auto hIntercept2D = HistContainer<TH2F>("Intercept2D", "Intercept Map", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hIntercept2D, Intercept2D, "Column", "Row");


  // ###########################
  // # Retrieve scanned region #
  // ###########################
  ROWstart = this->findValue(pSettingsMap,"ROWstart");
  ROWstop  = this->findValue(pSettingsMap,"ROWstop");
  COLstart = this->findValue(pSettingsMap,"COLstart");
  COLstop  = this->findValue(pSettingsMap,"COLstop");
}

void RD53GainHistograms::fillOccupancy (const DetectorDataContainer& data, int VCAL_HIGH)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  auto* hOcc2D = Occupancy2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH2F>>().fTheHistogram;

	  for (auto row = 0u; row < RD53::nRows; row++)
	    for (auto col = 0u; col < RD53::nCols; col++)
	      if ((row >= ROWstart) && (row <= ROWstop) && (col >= COLstart) && (col <= COLstop))
            hOcc2D->Fill(VCAL_HIGH, cChip->getChannel<OccupancyAndPh>(row,col).fPh);
	}
}

void RD53GainHistograms::fillGainIntercept (const DetectorDataContainer& data)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  auto* Gain1DHist = Gain1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
	  auto* Intercept1DHist     = Intercept1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
	  auto* Gain2DHist = Gain2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
	  auto* Intercept2DHist     = Intercept2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;

	  for (auto row = 0u; row < RD53::nRows; row++)
	    for (auto col = 0u; col < RD53::nCols; col++)
	      if (cChip->getChannel<GainAndIntercept>(row, col).fIntercept != 0)
		{
		  Gain1DHist->Fill(cChip->getChannel<GainAndIntercept>(row, col).fGain);
		  Intercept1DHist->Fill(cChip->getChannel<GainAndIntercept>(row, col).fIntercept);
		  Gain2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<GainAndIntercept>(row, col).fGain);
		  Intercept2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<GainAndIntercept>(row, col).fIntercept);
		}
	}
}

void RD53GainHistograms::process ()
{
  draw<TH2F>(Occupancy2D, "gcolz", true, "Charge (electrons)");
  draw<TH1F>(Gain1D, "", true, "Charge (electrons)");
  draw<TH1F>(Intercept1D, "", true, "Charge (electrons)");
  draw<TH2F>(Gain2D, "gcolz");
  draw<TH2F>(Intercept2D, "gcolz");
}
