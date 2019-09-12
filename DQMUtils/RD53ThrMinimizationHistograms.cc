/*!
  \file                  RD53ThrMinimizationHistograms.cc
  \brief                 Implementation of ThrMinimization calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrMinimizationHistograms.h"

using namespace Ph2_HwDescription;

void RD53ThrMinimizationHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap)
{
  uint16_t rangeThreshold = RD53::setBits(static_cast<RD53*>(theDetectorStructure.at(0)->at(0)->at(0))->getNumberOfBits("Vthreshold_LIN"))+1;

  auto hThrehsold = CanvasContainer<TH1F>("Threhsold", "Threhsold", rangeThreshold, 0, rangeThreshold);
  bookImplementer(theOutputFile, theDetectorStructure, hThrehsold, Threhsold, "Threhsold", "Entries");
}

void RD53ThrMinimizationHistograms::fill(const DetectorDataContainer& data)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* hThrehsold = Threhsold.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          hThrehsold->Fill(cChip->getSummary<RegisterValue>().fRegisterValue);
        }
}

void RD53ThrMinimizationHistograms::process ()
{
  draw<TH1F>(Threhsold);
}
