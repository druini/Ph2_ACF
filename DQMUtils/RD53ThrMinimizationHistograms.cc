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

void ThrMinimizationHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const Ph2_System::SettingsMap& settingsMap)
{
  ContainerFactory::copyStructure(theDetectorStructure, DetectorData);


  uint16_t rangeThreshold = RD53::setBits(static_cast<RD53*>(theDetectorStructure.at(0)->at(0)->at(0))->getNumberOfBits("Vthreshold_LIN")) + 1;

  auto hThrehsold = CanvasContainer<TH1F>("Threhsold", "Threhsold", rangeThreshold, 0, rangeThreshold);
  bookImplementer(theOutputFile, theDetectorStructure, hThrehsold, Threhsold, "Threhsold", "Entries");
}

bool ThrMinimizationHistograms::fill (std::vector<char>& dataBuffer)
{
  ChipContainerStream<EmptyContainer,uint16_t> theThrStreamer("ThrMinimization"); //@TMP@

  if(theThrStreamer.attachBuffer(&dataBuffer))
    {
      theThrStreamer.decodeChipData(DetectorData);
      ThrMinimizationHistograms::fill(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }

  return false;
}

void ThrMinimizationHistograms::fill (const DetectorDataContainer& DataContainer)
{
  for (const auto cBoard : DataContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          if (cChip->getSummaryContainer<uint16_t>() == nullptr) continue;

          auto* hThrehsold = Threhsold.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          hThrehsold->Fill(cChip->getSummary<uint16_t>());
        }
}

void ThrMinimizationHistograms::process ()
{
  draw<TH1F>(Threhsold);
}
