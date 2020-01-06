/*!
  \file                  RD53LatencyHistograms.cc
  \brief                 Implementation of Latency calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53LatencyHistograms.h"

using namespace Ph2_HwDescription;

void LatencyHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const Ph2_System::SettingsMap& settingsMap)
{
  ContainerFactory::copyStructure(theDetectorStructure, DetectorData);


  // #######################
  // # Retrieve parameters #
  // #######################
  startValue = this->findValueInSettings(settingsMap,"LatencyStart");
  stopValue  = this->findValueInSettings(settingsMap,"LatencyStop");


  auto hLatency = CanvasContainer<TH1F>("Latency", "Latency", stopValue - startValue + 1, startValue, stopValue + 1);
  bookImplementer(theOutputFile, theDetectorStructure, hLatency, Latency, "Latency (n.bx)", "Entries");

  auto hOcc1D = CanvasContainer<TH1F>("LatencyScan", "Latency Scan", stopValue - startValue + 1, startValue, stopValue + 1);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc1D, Occupancy1D, "Latency (n.bx)", "Efficiency");
}

bool LatencyHistograms::fill (std::vector<char>& dataBuffer)
{
  const size_t LatencySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;

  ChipContainerStream<EmptyContainer,GenericDataArray<LatencySize>> theOccStreamer    ("LatencyOcc"); // @TMP@
  ChipContainerStream<EmptyContainer,uint16_t>                      theLatencyStreamer("LatencyLatency"); // @TMP@

  if (theOccStreamer.attachBuffer(&dataBuffer))
    {
      theOccStreamer.decodeChipData(DetectorData);
      LatencyHistograms::fillOccupancy(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }
  else if (theLatencyStreamer.attachBuffer(&dataBuffer))
    {
      theLatencyStreamer.decodeChipData(DetectorData);
      LatencyHistograms::fillLatency(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }

  return false;
}

void LatencyHistograms::fillOccupancy (const DetectorDataContainer& OccupancyContainer)
{
  const size_t LatencySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;

  for (const auto cBoard : OccupancyContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          if (cChip->getSummaryContainer<GenericDataArray<LatencySize>>() == nullptr) continue;

          auto* Occupancy1DHist = Occupancy1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          for (size_t i = startValue; i <= stopValue; i++)
            Occupancy1DHist->SetBinContent(Occupancy1DHist->FindBin(i), cChip->getSummary<GenericDataArray<LatencySize>>().data[i-startValue]);
        }
}

void LatencyHistograms::fillLatency (const DetectorDataContainer& LatencyContainer)
{
  for (const auto cBoard : LatencyContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          if (cChip->getSummaryContainer<uint16_t>() == nullptr) continue;

          auto* LatencyHist = Latency.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          LatencyHist->Fill(cChip->getSummary<uint16_t>());
        }
}

void LatencyHistograms::process ()
{
  draw<TH1F>(Occupancy1D);
  draw<TH1F>(Latency);
}
