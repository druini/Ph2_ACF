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

void RD53InjectionDelayHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap)
{
  // #######################
  // # Retrieve parameters #
  // #######################
  startValue = this->findValue(pSettingsMap,"InjDelayStart");
  stopValue  = this->findValue(pSettingsMap,"InjDelayStop");


  auto hInjectionDelay = CanvasContainer<TH1F>("InjectionDelay", "Injection Delay", stopValue - startValue, startValue, stopValue);
  bookImplementer(theOutputFile, theDetectorStructure, hInjectionDelay, InjectionDelay, "Injection Delay (1.5625 ns)", "Entries");

  auto hOcc1D = CanvasContainer<TH1F>("RegisterScan", "Register Scan", stopValue - startValue, startValue, stopValue);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc1D, Occupancy1D, "Injection Delay (1.5625 ns)", "Efficiency");
}

void RD53InjectionDelayHistograms::fill (const DetectorDataContainer& OccupancyContainer, const DetectorDataContainer& InjectionDelayContainer)
{
  for (const auto cBoard : OccupancyContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* Occupancy1DHist    = Occupancy1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* InjectionDelayHist = InjectionDelay.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          for (size_t i = startValue; i < stopValue; i++)
            Occupancy1DHist->SetBinContent(Occupancy1DHist->FindBin(i),cChip->getSummary<GenericDataVector>().data1[i-startValue]);
          InjectionDelayHist->Fill(InjectionDelayContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue);
        }
}

void RD53InjectionDelayHistograms::process ()
{
  draw<TH1F>(Occupancy1D);
  draw<TH1F>(InjectionDelay);
}
