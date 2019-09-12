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

void RD53ThrEqualizationHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap)
{
  size_t TDACsize = RD53::setBits(RD53PixelEncoder::NBIT_TDAC) + 1;

  auto hThrEqualization = CanvasContainer<TH1F>("ThrEqualization", "ThrEqualization", nEvents / 2 + 1, 0, 1 + 2. / nEvents);
  bookImplementer(theOutputFile, theDetectorStructure, hThrEqualization, ThrEqualization, "Efficiency", "Entries");

  auto hTDAC = CanvasContainer<TH1F>("TDAC", "TDAC", TDACsize, 0, TDACsize);
  bookImplementer(theOutputFile, theDetectorStructure, hTDAC, TDAC, "TDAC", "Entries");
}

void RD53ThrEqualizationHistograms::fill (const DetectorDataContainer& OccupancyContainer, const DetectorDataContainer& TDACContainer)
{
  for (const auto cBoard : OccupancyContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* hThrEqualization = ThrEqualization.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* hTDAC            = TDAC.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              if (cChip->getChannel<OccupancyAndPh>(row, col).isEnabled == true)
                {
                  hThrEqualization->Fill(OccupancyContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy);
                  hTDAC->Fill(TDACContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<RegisterValue>(row,col).fRegisterValue);
                }
        }
}

void RD53ThrEqualizationHistograms::process ()
{
  draw<TH1F>(ThrEqualization);
  draw<TH2F>(TDAC);
}
