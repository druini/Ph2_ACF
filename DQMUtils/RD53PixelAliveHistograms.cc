/*!
  \file                  RD53PixelAliveHistograms.cc
  \brief                 Implementation of PixelAlive calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAliveHistograms.h"

using namespace Ph2_HwDescription;

void RD53PixelAliveHistograms::book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap)
{
  // #######################
  // # Retrieve parameters #
  // #######################
  nEvents = this->findValue(pSettingsMap,"nEvents");


  size_t ToTsize   = RD53::setBits(RD53EvtEncoder::NBIT_TOT / NPIX_REGION) + 1;
  size_t BCIDsize  = RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1;
  size_t TrgIDsize = RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1;

  auto hOcc1D = CanvasContainer<TH1F>("Occ1D", "Occ1D", nEvents + 1, 0, nEvents + 1);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc1D, Occupancy1D, "Number of hits", "Entries");

  auto hOcc2D = CanvasContainer<TH2F>("PixelAlive", "Pixel Alive", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hOcc2D, Occupancy2D, "Columns", "Rows");

  auto hErr2D = CanvasContainer<TH2F>("ReadoutErrors", "Readout Errors", RD53::nCols, 0, RD53::nCols, RD53::nRows, 0, RD53::nRows);
  bookImplementer(theOutputFile, theDetectorStructure, hErr2D, ErrorReadOut2D, "Columns", "Rows");

  auto hToT = CanvasContainer<TH1F>("ToT", "ToT Distribution", ToTsize, 0, ToTsize);
  bookImplementer(theOutputFile, theDetectorStructure, hToT, ToT, "ToT", "Entries");

  auto hBCID = CanvasContainer<TH1F>("BCID", "BCID", BCIDsize, 0, BCIDsize);
  bookImplementer(theOutputFile, theDetectorStructure, hBCID, BCID, "#DeltaBCID", "Entries");

  auto hTrigID = CanvasContainer<TH1F>("TriggerID", "TriggerID", TrgIDsize, 0, TrgIDsize);
  bookImplementer(theOutputFile, theDetectorStructure, hTrigID, TriggerID, "#DeltaTrigger-ID", "Entries");
}

void RD53PixelAliveHistograms::fill (const DetectorDataContainer& data)
{
  for (const auto cBoard : data)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto* Occupancy1DHist    = Occupancy1D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* Occupancy2DHist    = Occupancy2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;
          auto* ErrorReadOut2DHist = ErrorReadOut2D.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH2F>>().fTheHistogram;
          auto* ToTHist            = ToT.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* BCIDHist           = BCID.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
          auto* TriggerIDHist      = TriggerID.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              {
                if (cChip->getChannel<OccupancyAndPh>(row, col).fOccupancy != 0)
                  {
                    Occupancy1DHist->Fill(cChip->getChannel<OccupancyAndPh>(row, col).fOccupancy * nEvents);
                    Occupancy2DHist->SetBinContent(col + 1, row + 1, cChip->getChannel<OccupancyAndPh>(row, col).fOccupancy);
                    ToTHist->Fill(cChip->getChannel<OccupancyAndPh>(row, col).fPh);
                  }
                if (cChip->getChannel<OccupancyAndPh>(row, col).readoutError == true) ErrorReadOut2DHist->Fill(col + 1, row + 1);
              }

          for (auto i = 1u; i < cChip->getSummary<GenericDataVector, OccupancyAndPh>().data1.size(); i++)
            {
              int deltaBCID = cChip->getSummary<GenericDataVector, OccupancyAndPh>().data1[i] - cChip->getSummary<GenericDataVector, OccupancyAndPh>().data1[i-1];
              BCIDHist->Fill((deltaBCID > 0 ? 0 : RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1) + deltaBCID);
            }

          for (auto i = 1u; i < cChip->getSummary<GenericDataVector, OccupancyAndPh>().data2.size(); i++)
            {
              int deltaTrgID = cChip->getSummary<GenericDataVector, OccupancyAndPh>().data2[i] - cChip->getSummary<GenericDataVector, OccupancyAndPh>().data2[i-1];
              TriggerIDHist->Fill((deltaTrgID > 0 ? 0 : RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1) + deltaTrgID);
            }
        }
}

void RD53PixelAliveHistograms::process ()
{
  draw<TH1F>(Occupancy1D);
  draw<TH2F>(Occupancy2D, "gcolz");
  draw<TH2F>(ErrorReadOut2D, "gcolz");
  draw<TH1F>(ToT);
  draw<TH1F>(BCID);
  draw<TH1F>(TriggerID);
}
