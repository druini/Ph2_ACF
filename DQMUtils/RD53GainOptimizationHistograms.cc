/*!
  \file                  RD53GainOptimizationHistograms.cc
  \brief                 Implementation of Gain optimization calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53GainOptimizationHistograms.h"

void RD53GainOptimizationHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, uint32_t> pSettingsMap)
{
  auto hOcc2D = HistContainer<TH1F>("KrumCurr", "KrumCurr", rangeKrumCurr, 0, rangeKrumCurr);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc2D, KrumCurr, "Krummenacher Current", "Entries");
}

void RD53GainOptimizationHistograms::fill(const DetectorDataContainer& data)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  auto* hKrumCurr = KrumCurr.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
	  hKrumCurr->Fill(cChip->getSummary<RegisterValue,EmptyContainer>().fRegisterValue);
	}
}

void RD53GainOptimizationHistograms::process ()
{
  draw<TH1F>(KrumCurr);
}
