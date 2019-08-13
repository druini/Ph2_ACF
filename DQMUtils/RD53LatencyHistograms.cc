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

void RD53LatencyHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, uint32_t> pSettingsMap)
{
  auto hOcc1D = HistContainer<TH1F>("Latency", "Latency", stopValue - startValue, startValue, stopValue);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc1D, Occupancy1D, "Latency [n.bx]", "Entries");
}

void RD53LatencyHistograms::fill (const DetectorDataContainer& data)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  auto* Occupancy1DHist = Occupancy1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
          
	  for (size_t i = startValue; i < stopValue; i++)
	    Occupancy1DHist->SetBinContent(Occupancy1DHist->FindBin(i),cChip->getSummary<GenericDataVector>().data1[i-startValue]);
	}
}

void RD53LatencyHistograms::process ()
{
  draw<TH1F>(Occupancy1D);
}
