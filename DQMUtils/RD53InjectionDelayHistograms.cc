/*!
  \file                  RD53InjectionDelayHistograms.cc
  \brief                 Implementation of InjectionDelay calibration histograms
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53InjectionDelayHistograms.h"

using namespace Ph2_HwDescription;

void InjectionDelayHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap settingsMap)
{
  ContainerFactory::copyStructure(theDetectorStructure, DetectorData);


  // #######################
  // # Retrieve parameters #
  // #######################
  startValue = 0;
  stopValue  = 2*(RD53::setBits(static_cast<RD53*>(theDetectorStructure.at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"))+1) - 1;


  auto hInjectionDelay = CanvasContainer<TH1F>("InjectionDelay", "Injection Delay", stopValue - startValue + 1, startValue, stopValue + 1);
  bookImplementer(theOutputFile, theDetectorStructure, hInjectionDelay, InjectionDelay, "Injection Delay (1.5625 ns)", "Entries");

  auto hOcc1D = CanvasContainer<TH1F>("InjDelayScan", "Injection Delay Scan", stopValue - startValue + 1, startValue, stopValue + 1);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc1D, Occupancy1D, "Injection Delay (1.5625 ns)", "Efficiency");
}

bool InjectionDelayHistograms::fill (std::vector<char>& dataBuffer)
{
  ChannelContainerStream<GenericDataVector> theOccStreamer           ("InjectionDelayOcc");
  ChannelContainerStream<uint16_t>          theInjectionDelayStreamer("InjectionDelayInjDelay");

  if (theOccStreamer.attachBuffer(&dataBuffer))
    {
      theOccStreamer.decodeChipData(DetectorData);
      InjectionDelayHistograms::fillOccupancy(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }
  else if (theInjectionDelayStreamer.attachBuffer(&dataBuffer))
    {
      theInjectionDelayStreamer.decodeChipData(DetectorData);
      InjectionDelayHistograms::fillInjectionDelay(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }

  return false;
}

void InjectionDelayHistograms::fillOccupancy (const DetectorDataContainer& OccupancyContainer)
{
  for (const auto cBoard : OccupancyContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* Occupancy1DHist = Occupancy1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          for (size_t i = startValue; i <= stopValue; i++)
            Occupancy1DHist->SetBinContent(Occupancy1DHist->FindBin(i),cChip->getSummary<GenericDataVector>().data1[i-startValue]);
        }
}

void InjectionDelayHistograms::fillInjectionDelay (const DetectorDataContainer& InjectionDelayContainer)
{
  for (const auto cBoard : InjectionDelayContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* InjectionDelayHist = InjectionDelay.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          InjectionDelayHist->Fill(cChip->getSummary<uint16_t>());
        }
}

void InjectionDelayHistograms::process ()
{
  draw<TH1F>(Occupancy1D);
  draw<TH1F>(InjectionDelay);
}
