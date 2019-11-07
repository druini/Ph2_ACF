/*!
  \file                  RD53PhysicsHistograms.cc
  \brief                 Implementation of Physics histograms
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PhysicsHistograms.h"

using namespace Ph2_HwDescription;

void PhysicsHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const Ph2_System::SettingsMap& settingsMap)
{
  ContainerFactory::copyStructure(theDetectorStructure, DetectorData);

  auto hPh2D = CanvasContainer<TH2F>("Physics", "Physics", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hPh2D, Ph2D, "Columns", "Rows");

  auto hOcc2D = CanvasContainer<TH2F>("Physics", "Physics", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc2D, Occupancy2D, "Columns", "Rows");

  auto hErr2D = CanvasContainer<TH2F>("ReadoutErrors", "Readout Errors", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hErr2D, ErrorReadOut2D, "Columns", "Rows");
}

bool PhysicsHistograms::fill (std::vector<char>& dataBuffer)
{
  ChannelContainerStream<OccupancyAndPh> theOccStreamer("Physics");

  if (theOccStreamer.attachBuffer(&dataBuffer))
    {
      theOccStreamer.decodeChipData(DetectorData);
      PhysicsHistograms::fill(DetectorData);
      DetectorData.cleanDataStored();
      return true;
    }

  return false;
}

void PhysicsHistograms::fill (const DetectorDataContainer& DataContainer)
{
  for (const auto cBoard : DataContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* Ph2DHist           = Ph2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;
          auto* Occupancy2DHist    = Occupancy2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;
          auto* ErrorReadOut2DHist = ErrorReadOut2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              {
                if (cChip->getChannel<OccupancyAndPh>(row, col).fOccupancy != 0)
                  {
                    Ph2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<OccupancyAndPh>(row, col).fPh);
                    Occupancy2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<OccupancyAndPh>(row, col).fOccupancy);
                  }

                if (cChip->getChannel<OccupancyAndPh>(row, col).readoutError == true) ErrorReadOut2DHist->Fill(col + 1, row + 1);
              }
        }
}

void PhysicsHistograms::process ()
{
  draw<TH2F>(Ph2D, "gcolz");
  draw<TH2F>(Occupancy2D, "gcolz");
  draw<TH2F>(ErrorReadOut2D, "gcolz");
}
