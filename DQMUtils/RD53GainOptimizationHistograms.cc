/*!
  \file                  RD53GainOptimizationHistograms.cc
  \brief                 Implementation of Gain calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53GainOptimizationHistograms.h"


using namespace Ph2_HwDescription;

void RD53GainOptimizationHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, uint32_t> pSettingsMap)
{
  // @TMP@
  auto hOcc2D = HistContainer<TH1F>("KrumCurr", "KrumCurr", 256, 0, 256);
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
