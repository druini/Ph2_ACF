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

using namespace Ph2_HwDescription;

void GainOptimizationHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const Ph2_System::SettingsMap& settingsMap)
{
  ContainerFactory::copyStructure(theDetectorStructure, DetectorData);


  uint16_t rangeKrumCurr = RD53::setBits(static_cast<RD53*>(theDetectorStructure.at(0)->at(0)->at(0))->getNumberOfBits("KRUM_CURR_LIN")) + 1;

  auto hOcc2D = CanvasContainer<TH1F>("KrumCurr", "KrumCurr", rangeKrumCurr, 0, rangeKrumCurr);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc2D, KrumCurr, "Krummenacher Current", "Entries");
}

bool GainOptimizationHistograms::fill (std::vector<char>& dataBuffer)
{
  ChipContainerStream<EmptyContainer,uint16_t> theKrumStreamer("GainOptimization"); // @TMP@

  if(theKrumStreamer.attachBuffer(&dataBuffer))
    {
      theKrumStreamer.decodeChipData(DetectorData);
      GainOptimizationHistograms::fill(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }

  return false;
}

void GainOptimizationHistograms::fill (const DetectorDataContainer& DataContainer)
{
  for (const auto cBoard : DataContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          if (cChip->getSummaryContainer<uint16_t>() == nullptr) continue;

          auto* hKrumCurr = KrumCurr.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          hKrumCurr->Fill(cChip->getSummary<uint16_t>());
        }
}

void GainOptimizationHistograms::process ()
{
  draw<TH1F>(KrumCurr);
}
