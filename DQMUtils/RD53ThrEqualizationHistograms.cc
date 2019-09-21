/*!
  \file                  RD53ThrEqualizationHistograms.cc
  \brief                 Implementation of ThrEqualization calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrEqualizationHistograms.h"

using namespace Ph2_HwDescription;

void ThrEqualizationHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap settingsMap)
{
  ContainerFactory::copyStructure(theDetectorStructure, DetectorData);


  // #######################
  // # Retrieve parameters #
  // #######################
  nEvents     = this->findValueInSettings(settingsMap,"nEvents");
  VCalHnsteps = this->findValueInSettings(settingsMap,"VCalHnsteps");


  size_t TDACsize = RD53::setBits(RD53PixelEncoder::NBIT_TDAC) + 1;

  auto hThrEqualization = CanvasContainer<TH1F>("ThrEqualization", "ThrEqualization", nEvents*VCalHnsteps + 1, 0, 1 + 1. / (nEvents*VCalHnsteps));
  bookImplementer(theOutputFile, theDetectorStructure, hThrEqualization, ThrEqualization, "Efficiency", "Entries");

  auto hTDAC = CanvasContainer<TH1F>("TDAC", "TDAC", TDACsize, 0, TDACsize);
  bookImplementer(theOutputFile, theDetectorStructure, hTDAC, TDAC, "TDAC", "Entries");
}

bool ThrEqualizationHistograms::fill (std::vector<char>& dataBuffer)
{
  ChannelContainerStream<OccupancyAndPh> theOccStreamer ("ThrEqualizationOcc");
  ChannelContainerStream<RegisterValue>  theTDACStreamer("ThrEqualizationTDAC");

  if (theOccStreamer.attachBuffer(&dataBuffer))
    {
      theOccStreamer.decodeChipData(DetectorData);
      ThrEqualizationHistograms::fillOccupancy(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }
  else if (theTDACStreamer.attachBuffer(&dataBuffer))
    {
      theTDACStreamer.decodeChipData(DetectorData);
      ThrEqualizationHistograms::fillTDAC(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }

  return false;
}

void ThrEqualizationHistograms::fillOccupancy (const DetectorDataContainer& OccupancyContainer)
{
  for (const auto cBoard : OccupancyContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* hThrEqualization = ThrEqualization.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              if (cChip->getChannel<OccupancyAndPh>(row, col).isEnabled == true)
                hThrEqualization->Fill(cChip->getChannel<OccupancyAndPh>(row, col).fOccupancy);
        }
}

void ThrEqualizationHistograms::fillTDAC (const DetectorDataContainer& TDACContainer)
{
  for (const auto cBoard : TDACContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* hTDAC = TDAC.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              if (cChip->getChannel<RegisterValue>(row, col).isEnabled == true)
                hTDAC->Fill(cChip->getChannel<RegisterValue>(row, col).fRegisterValue);
        }
}

void ThrEqualizationHistograms::process ()
{
  draw<TH1F>(ThrEqualization);
  draw<TH1F>(TDAC);
}
