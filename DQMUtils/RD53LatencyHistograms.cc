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

void LatencyHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap settingsMap)
{
  ContainerFactory::copyStructure(theDetectorStructure, DetectorData);


  // #######################
  // # Retrieve parameters #
  // #######################
  startValue = this->findValueInSettings(settingsMap,"LatencyStart");
  stopValue  = this->findValueInSettings(settingsMap,"LatencyStop");


  auto hLatency = CanvasContainer<TH1F>("Latency", "Latency", stopValue - startValue, startValue, stopValue);
  bookImplementer(theOutputFile, theDetectorStructure, hLatency, Latency, "Latency (n.bx)", "Entries");

  auto hOcc1D = CanvasContainer<TH1F>("LatencyScan", "Latency Scan", stopValue - startValue, startValue, stopValue);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc1D, Occupancy1D, "Latency (n.bx)", "Efficiency");
}

bool LatencyHistograms::fill (std::vector<char>& dataBuffer)
{
  ChannelContainerStream<GenericDataVector> theStreamer       ("RD53Latency");
  ChannelContainerStream<RegisterValue>     theLatencyStreamer("RD53Latency");

  if (theStreamer.attachBuffer(&dataBuffer))
    {
      theStreamer.decodeChipData(DetectorData);
      // LatencyHistograms::fill(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }
  else if (theLatencyStreamer.attachBuffer(&dataBuffer))
    {
      theLatencyStreamer.decodeChipData(DetectorData);
      // LatencyHistograms::fill(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }

  return false;
}

void LatencyHistograms::fill (const DetectorDataContainer& OccupancyContainer, const DetectorDataContainer& LatencyContainer)
{
  for (const auto cBoard : OccupancyContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* Occupancy1DHist = Occupancy1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* LatencyHist     = Latency.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          for (size_t i = startValue; i < stopValue; i++)
            Occupancy1DHist->SetBinContent(Occupancy1DHist->FindBin(i),cChip->getSummary<GenericDataVector>().data1[i-startValue]);
          LatencyHist->Fill(LatencyContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue);
        }
}

void LatencyHistograms::process ()
{
  draw<TH1F>(Occupancy1D);
  draw<TH1F>(Latency);
}
