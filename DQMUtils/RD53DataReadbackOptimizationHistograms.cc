/*!
  \file                  RD53DataReadbackOptimizationHistograms.cc
  \brief                 Implementation of data readback optimization histograms
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53DataReadbackOptimizationHistograms.h"

using namespace Ph2_HwDescription;

void DataReadbackOptimizationHistograms::book(TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const Ph2_System::SettingsMap& settingsMap)
{
    ContainerFactory::copyStructure(theDetectorStructure, DetectorData);

    // #######################
    // # Retrieve parameters #
    // #######################
    startValueTAP0 = this->findValueInSettings(settingsMap, "StartValueTAP0");
    stopValueTAP0  = this->findValueInSettings(settingsMap, "StopValueTAP0");
    startValueTAP1 = this->findValueInSettings(settingsMap, "StartValueTAP1");
    stopValueTAP1  = this->findValueInSettings(settingsMap, "StopValueTAP1");
    startValueTAP2 = this->findValueInSettings(settingsMap, "StartValueTAP2");
    stopValueTAP2  = this->findValueInSettings(settingsMap, "StopValueTAP2");

    auto hTAP0scan = CanvasContainer<TH1F>("TAP0scan", "TAP0 scan", stopValueTAP0 - startValueTAP0 + 1, startValueTAP0, stopValueTAP0 + 1);
    bookImplementer(theOutputFile, theDetectorStructure, TAP0scan, hTAP0scan, "TAP0 - driver", "Bit Error Rate");
    auto hTAP0 = CanvasContainer<TH1F>("TAP0", "TAP0 - driver", stopValueTAP0 - startValueTAP0 + 1, startValueTAP0, stopValueTAP0 + 1);
    bookImplementer(theOutputFile, theDetectorStructure, TAP0, hTAP0, "TAP0 - driver", "Bit Error Rate");

    auto hTAP1scan = CanvasContainer<TH1F>("TAP1scan", "TAP1 scan", stopValueTAP1 - startValueTAP1 + 1, startValueTAP1, stopValueTAP1 + 1);
    bookImplementer(theOutputFile, theDetectorStructure, TAP1scan, hTAP1scan, "TAP1 - pre-emphasis-1", "Bit Error Rate");
    auto hTAP1 = CanvasContainer<TH1F>("TAP1", "TAP1 - pre-emphasis-1", stopValueTAP1 - startValueTAP1 + 1, startValueTAP1, stopValueTAP1 + 1);
    bookImplementer(theOutputFile, theDetectorStructure, TAP1, hTAP1, "TAP1 - pre-emphasis-1", "Bit Error Rate");

    auto hTAP2scan = CanvasContainer<TH1F>("TAP2scan", "TAP2 scan", stopValueTAP2 - startValueTAP2 + 1, startValueTAP2, stopValueTAP2 + 1);
    bookImplementer(theOutputFile, theDetectorStructure, TAP2scan, hTAP2scan, "TAP2 - pre-emphasis-2", "Bit Error Rate");
    auto hTAP2 = CanvasContainer<TH1F>("TAP2", "TAP2 - pre-emphasis-2", stopValueTAP2 - startValueTAP2 + 1, startValueTAP2, stopValueTAP2 + 1);
    bookImplementer(theOutputFile, theDetectorStructure, TAP2, hTAP2, "TAP2 - pre-emphasis-2", "Bit Error Rate");
}

bool DataReadbackOptimizationHistograms::fill(std::vector<char>& dataBuffer)
{
    const size_t TAPsize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    ChipContainerStream<EmptyContainer, GenericDataArray<TAPsize>> theTAP0scanStreamer("TAP0scan");
    ChipContainerStream<EmptyContainer, uint16_t>                  theTAP0Streamer("TAP0");
    ChipContainerStream<EmptyContainer, GenericDataArray<TAPsize>> theTAP1scanStreamer("TAP1scan");
    ChipContainerStream<EmptyContainer, uint16_t>                  theTAP1Streamer("TAP1");
    ChipContainerStream<EmptyContainer, GenericDataArray<TAPsize>> theTAP2scanStreamer("TAP2scan");
    ChipContainerStream<EmptyContainer, uint16_t>                  theTAP2Streamer("TAP2");

    if(theTAP0scanStreamer.attachBuffer(&dataBuffer))
    {
        theTAP0scanStreamer.decodeChipData(DetectorData);
        DataReadbackOptimizationHistograms::fillScanTAP0(DetectorData);
        DetectorData.cleanDataStored();
        return true;
    }
    else if(theTAP0Streamer.attachBuffer(&dataBuffer))
    {
        theTAP0Streamer.decodeChipData(DetectorData);
        DataReadbackOptimizationHistograms::fillTAP0(DetectorData);
        DetectorData.cleanDataStored();
        return true;
    }
    else if(theTAP1scanStreamer.attachBuffer(&dataBuffer))
    {
        theTAP1scanStreamer.decodeChipData(DetectorData);
        DataReadbackOptimizationHistograms::fillScanTAP1(DetectorData);
        DetectorData.cleanDataStored();
        return true;
    }
    else if(theTAP1Streamer.attachBuffer(&dataBuffer))
    {
        theTAP1Streamer.decodeChipData(DetectorData);
        DataReadbackOptimizationHistograms::fillTAP1(DetectorData);
        DetectorData.cleanDataStored();
        return true;
    }
    else if(theTAP2scanStreamer.attachBuffer(&dataBuffer))
    {
        theTAP2scanStreamer.decodeChipData(DetectorData);
        DataReadbackOptimizationHistograms::fillScanTAP2(DetectorData);
        DetectorData.cleanDataStored();
        return true;
    }
    else if(theTAP2Streamer.attachBuffer(&dataBuffer))
    {
        theTAP2Streamer.decodeChipData(DetectorData);
        DataReadbackOptimizationHistograms::fillTAP2(DetectorData);
        DetectorData.cleanDataStored();
        return true;
    }

    return false;
}

void DataReadbackOptimizationHistograms::fillScanTAP0(const DetectorDataContainer& TAP0scanContainer)
{
    const size_t TAPsize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    for(const auto cBoard: TAP0scanContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    if(cChip->getSummaryContainer<GenericDataArray<TAPsize>>() == nullptr) continue;

                    auto* TAP0scanHist =
                        TAP0scan.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

                    for(size_t i = startValueTAP0; i <= stopValueTAP0; i++)
                        TAP0scanHist->SetBinContent(TAP0scanHist->FindBin(i), cChip->getSummary<GenericDataArray<TAPsize>>().data[i - startValueTAP0]);
                }
}

void DataReadbackOptimizationHistograms::fillTAP0(const DetectorDataContainer& TAP0Container)
{
    for(const auto cBoard: TAP0Container)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    if(cChip->getSummaryContainer<uint16_t>() == nullptr) continue;

                    auto* TAP0Hist = TAP0.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

                    TAP0Hist->Fill(cChip->getSummary<uint16_t>());
                }
}

void DataReadbackOptimizationHistograms::fillScanTAP1(const DetectorDataContainer& TAP1scanContainer)
{
    const size_t TAPsize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    for(const auto cBoard: TAP1scanContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    if(cChip->getSummaryContainer<GenericDataArray<TAPsize>>() == nullptr) continue;

                    auto* TAP1scanHist =
                        TAP1scan.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

                    for(size_t i = startValueTAP1; i <= stopValueTAP1; i++)
                        TAP1scanHist->SetBinContent(TAP1scanHist->FindBin(i), cChip->getSummary<GenericDataArray<TAPsize>>().data[i - startValueTAP1]);
                }
}

void DataReadbackOptimizationHistograms::fillTAP1(const DetectorDataContainer& TAP1Container)
{
    for(const auto cBoard: TAP1Container)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    if(cChip->getSummaryContainer<uint16_t>() == nullptr) continue;

                    auto* TAP1Hist = TAP1.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

                    TAP1Hist->Fill(cChip->getSummary<uint16_t>());
                }
}

void DataReadbackOptimizationHistograms::fillScanTAP2(const DetectorDataContainer& TAP2scanContainer)
{
    const size_t TAPsize = RD53Shared::setBits(RD53Shared::MAXBITCHIPREG) + 1;

    for(const auto cBoard: TAP2scanContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    if(cChip->getSummaryContainer<GenericDataArray<TAPsize>>() == nullptr) continue;

                    auto* TAP2scanHist =
                        TAP2scan.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

                    for(size_t i = startValueTAP2; i <= stopValueTAP2; i++)
                        TAP2scanHist->SetBinContent(TAP2scanHist->FindBin(i), cChip->getSummary<GenericDataArray<TAPsize>>().data[i - startValueTAP2]);
                }
}

void DataReadbackOptimizationHistograms::fillTAP2(const DetectorDataContainer& TAP2Container)
{
    for(const auto cBoard: TAP2Container)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    if(cChip->getSummaryContainer<uint16_t>() == nullptr) continue;

                    auto* TAP2Hist = TAP2.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;

                    TAP2Hist->Fill(cChip->getSummary<uint16_t>());
                }
}

void DataReadbackOptimizationHistograms::process()
{
    draw<TH1F>(TAP0scan);
    draw<TH1F>(TAP0);
    draw<TH1F>(TAP1scan);
    draw<TH1F>(TAP1);
    draw<TH1F>(TAP2scan);
    draw<TH1F>(TAP2);
}
