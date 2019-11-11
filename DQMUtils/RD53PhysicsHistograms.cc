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

  size_t ToTsize   = RD53::setBits(RD53EvtEncoder::NBIT_TOT / RD53Constants::NPIX_REGION) + 1;
  size_t BCIDsize  = RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1;
  size_t TrgIDsize = RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1;

  auto hToT1D = CanvasContainer<TH1F>("ToT1D", "ToT Distribution", ToTsize, 0, ToTsize);
  bookImplementer(theOutputFile, theDetectorStructure, hToT1D, ToT1D, "ToT", "Entries");

  auto hToT2D = CanvasContainer<TH2F>("ToT2D", "ToT Distribution", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hToT2D, ToT2D, "Columns", "Rows");

  auto hOcc2D = CanvasContainer<TH2F>("Occ2D", "Occupancy", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc2D, Occupancy2D, "Columns", "Rows");

  auto hErr2D = CanvasContainer<TH2F>("ReadoutErrors", "Readout Errors", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hErr2D, ErrorReadOut2D, "Columns", "Rows");

  auto hBCID = CanvasContainer<TH1F>("BCID", "BCID", BCIDsize, 0, BCIDsize);
  bookImplementer(theOutputFile, theDetectorStructure, hBCID, BCID, "#DeltaBCID", "Entries");

  auto hTrigID = CanvasContainer<TH1F>("TriggerID", "TriggerID", TrgIDsize, 0, TrgIDsize);
  bookImplementer(theOutputFile, theDetectorStructure, hTrigID, TriggerID, "#DeltaTrigger-ID", "Entries");
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
          auto* ToT1DHist          = ToT1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* ToT2DHist          = ToT2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;
          auto* Occupancy2DHist    = Occupancy2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;
          auto* ErrorReadOut2DHist = ErrorReadOut2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;
          auto* BCIDHist           = BCID.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* TriggerIDHist      = TriggerID.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              {
                if (cChip->getChannel<OccupancyAndPh>(row, col).fOccupancy != 0)
                  {
                    ToT1DHist->Fill(cChip->getChannel<OccupancyAndPh>(row, col).fPh);
                    ToT2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<OccupancyAndPh>(row, col).fPh);
                    Occupancy2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<OccupancyAndPh>(row, col).fOccupancy);
                  }

                if (cChip->getChannel<OccupancyAndPh>(row, col).readoutError == true) ErrorReadOut2DHist->Fill(col + 1, row + 1);
              }

          for (auto i = 1u; i < cChip->getSummary<GenericDataVector,OccupancyAndPh>().data1.size(); i++)
            {
              int deltaBCID = cChip->getSummary<GenericDataVector,OccupancyAndPh>().data1[i] - cChip->getSummary<GenericDataVector,OccupancyAndPh>().data1[i-1];
              BCIDHist->Fill((deltaBCID > 0 ? 0 : RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1) + deltaBCID);
            }

          for (auto i = 1u; i < cChip->getSummary<GenericDataVector,OccupancyAndPh>().data2.size(); i++)
            {
              int deltaTrgID = cChip->getSummary<GenericDataVector,OccupancyAndPh>().data2[i] - cChip->getSummary<GenericDataVector,OccupancyAndPh>().data2[i-1];
              TriggerIDHist->Fill((deltaTrgID > 0 ? 0 : RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1) + deltaTrgID);
            }

        }
}

void PhysicsHistograms::process ()
{
  draw<TH1F>(ToT1D);
  draw<TH2F>(ToT2D, "gcolz");
  draw<TH2F>(Occupancy2D, "gcolz");
  draw<TH2F>(ErrorReadOut2D, "gcolz");
  draw<TH1F>(BCID);
  draw<TH1F>(TriggerID);
}
