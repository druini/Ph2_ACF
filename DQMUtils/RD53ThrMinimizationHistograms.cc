/*!
  \file                  RD53ThrMinimizationHistograms.cc
  \brief                 Implementation of Gain calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrMinimizationHistograms.h"


// #############
// # CONSTANTS #
// #############
#define INTERCEPT_HALFRANGE 6 // [ToT]

using namespace Ph2_HwDescription;

void RD53ThrMinimizationHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, uint32_t> pSettingsMap)
{
  // @TMP@
  // size_t ThrSize = RD53::setBits(static_cast<RD53*>(cChip)->getNumberOfBits("Vthreshold_LIN"))+1;

  auto hThrehsold = HistContainer<TH1F>("Threhsold", "Threhsold", 1024, 0, 1024);
  bookImplementer(theOutputFile, theDetectorStructure, hThrehsold, Threhsold, "Threhsold", "Entries");
}

void RD53ThrMinimizationHistograms::fill(const DetectorDataContainer& data)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  auto* hThrehsold = Threhsold.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
	  hThrehsold->Fill(cChip->getSummary<RegisterValue,EmptyContainer>().fRegisterValue);
	}
}


void RD53ThrMinimizationHistograms::process ()
{
  draw<TH1F>(Threhsold);
}
