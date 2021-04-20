/*!
        \file                DQMHistogramLatencyScan.h
        \brief               base class to create and fill monitoring histograms
        \author              Fabio Ravera, Lorenzo Uplegger
        \version             1.0
        \date                6/5/19
        Support :            mail to : fabio.ravera@cern.ch
 */

#include "../DQMUtils/DQMHistogramLatencyScan.h"
#include "../RootUtils/RootContainerFactory.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/Occupancy.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/Utilities.h"
#include "../Utils/GenericDataArray.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"

//========================================================================================================================
DQMHistogramLatencyScan::DQMHistogramLatencyScan() {

}

//========================================================================================================================
DQMHistogramLatencyScan::~DQMHistogramLatencyScan() {}

//========================================================================================================================
void DQMHistogramLatencyScan::book(TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const Ph2_System::SettingsMap& pSettingsMap)
{
    //need to get settings from settings map
    parseSettings(pSettingsMap);

    uint32_t fTDCBins = 8; //from LatencyScan.h

    ContainerFactory::copyStructure(theDetectorStructure, fDetectorData);
    LOG(INFO) << "Setting histograms with range " << fLatencyRange << " and start value " << fStartLatency;

    HistContainer<TH1F> hLatency("LatencyValue", "Latency Value", fLatencyRange, fStartLatency, fStartLatency+fLatencyRange);
    RootContainerFactory::bookHybridHistograms(theOutputFile, theDetectorStructure, fLatencyHistograms, hLatency);

    HistContainer<TH1F> hStub("StubLatencyValue", "Stub Latency Value", fLatencyRange, fStartLatency, fStartLatency+fLatencyRange);
    RootContainerFactory::bookHybridHistograms(theOutputFile, theDetectorStructure, fStubLatencyHistograms, hStub);

    HistContainer<TH1F> hStubScan("StubScanLatencyValue", "Stub Scan Latency Value", fLatencyRange, fStartLatency, fStartLatency+fLatencyRange);
    RootContainerFactory::bookHybridHistograms(theOutputFile, theDetectorStructure, fStubScanLatencyHistograms, hStubScan);

    HistContainer<TH2F> hLatencyScan2D("LatencyScan2D", "LatencyScan2D", 1, 0, 1, 1, 0, 1);
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fLatencyScan2DHistograms, hLatencyScan2D);
    
    HistContainer<TH1F> hTriggerTDC("TriggerTDC", "Trigger TDC", fTDCBins, -0.5, fTDCBins - 0.5);
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fTriggerTDCHistograms, hTriggerTDC);




}

//========================================================================================================================
bool DQMHistogramLatencyScan::fill(std::vector<char>& dataBuffer) {

    HybridContainerStream< EmptyContainer, EmptyContainer,  GenericDataArray<VECSIZE, uint32_t> > theLatencyStream("LatencyScan");
    HybridContainerStream< EmptyContainer, EmptyContainer,  GenericDataArray<VECSIZE, uint32_t> > theStubStream("LatencyScanStub");
    HybridContainerStream< EmptyContainer, EmptyContainer,  GenericDataArray<VECSIZE, uint32_t> > theStubScanStream("LatencyScanStubScan");
    HybridContainerStream< EmptyContainer, EmptyContainer,  GenericDataArray<TDCBINS, uint32_t> > theTriggerTDCStream("LatencyScanTriggerTDC");

    if(theLatencyStream.attachBuffer(&dataBuffer))
    {
        std::cout << "Matched Latency Stream!!!!!\n";
        theLatencyStream.decodeHybridData(fDetectorData);
        fillLatencyPlots(fDetectorData);
        fDetectorData.cleanDataStored();
        return true;
    }

    if(theTriggerTDCStream.attachBuffer(&dataBuffer))
    {
        std::cout << "Matched TriggerTDC!!!!!\n";
        theTriggerTDCStream.decodeHybridData(fDetectorData);
        fillTriggerTDCPlots(fDetectorData);
        fDetectorData.cleanDataStored();
        return true;
    }

    if(theStubStream.attachBuffer(&dataBuffer))
    {
        std::cout << "Matched Stub Latency!!!!!\n";
        theStubStream.decodeHybridData(fDetectorData);
        fillStubLatencyPlots(fDetectorData);
        fDetectorData.cleanDataStored();
        return true;
    }
    
    if(theStubScanStream.attachBuffer(&dataBuffer))
    {
        std::cout << "Matched Stub Latency!!!!!\n";
        theStubScanStream.decodeHybridData(fDetectorData);
        fillStubScanLatencyPlots(fDetectorData);
        fDetectorData.cleanDataStored();
        return true;
    }


    return false;




 }

//========================================================================================================================
void DQMHistogramLatencyScan::process() { 

    for(auto board: fLatencyHistograms)
    {
        for(auto opticalGroup: *board)
        {
            for(auto hybrid: *opticalGroup)
            {
                TCanvas* latencyCanvas    = new TCanvas(("Latency_" + std::to_string(hybrid->getId())).data(), ("Latency " + std::to_string(hybrid->getId())).data(), 500,500);

                //latencyCanvas->DivideSquare(hybrid->size());

                    latencyCanvas->cd();
                    TH1F* latencyHistogram = hybrid->getSummary<HistContainer<TH1F>>().fTheHistogram;
                    LOG(INFO) << "Drawing histogram with integral " << latencyHistogram->Integral();
                    latencyHistogram->GetXaxis()->SetTitle("Trigger Latency");
                    latencyHistogram->GetYaxis()->SetTitle("# of hits");
                    latencyHistogram->DrawCopy();

            }
        }
    }    
}

//========================================================================================================================

void DQMHistogramLatencyScan::reset(void) {}

void DQMHistogramLatencyScan::fillLatencyPlots(DetectorDataContainer& theLatency)
{ 
    for(auto board: theLatency)
    {
        for(auto opticalGroup: *board)
        {
            for(auto hybrid: *opticalGroup)
            {
                if(!hybrid->hasSummary()) continue;
                TH1F* hybridLatencyHistogram =
                    fLatencyHistograms.at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;

                for(uint32_t i = 0; i < fLatencyRange ; i++){

                    uint32_t lat = i + fStartLatency;
                    uint32_t hits = hybrid->getSummary<GenericDataArray<VECSIZE, uint32_t>>()[i];
                    
                    float error = 0;
                    if (hits > 0 ) error = sqrt(float(hits));

                    LOG(INFO) << "filling hybrid " << hybrid->getIndex() << " with latency " << lat << " and hits " << hits << " and error " << error;
                    hybridLatencyHistogram->SetBinContent(i, hits);
                    hybridLatencyHistogram->SetBinError(i, error);
                }
            }

        }
        
    }

}
void DQMHistogramLatencyScan::fillStubLatencyPlots(DetectorDataContainer& theStubLatency)
{
    for(auto board: theStubLatency)
    {
        for(auto opticalGroup: *board)
        {
            for(auto hybrid: *opticalGroup)
            {
                if(!hybrid->hasSummary()) continue;
                TH1F* hybridLatencyHistogram =
                    fStubLatencyHistograms.at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;

                for(uint32_t i = 0; i < fLatencyRange ; i++){

                    uint32_t lat = i + fStartLatency;
                    uint32_t hits = hybrid->getSummary<GenericDataArray<VECSIZE, uint32_t>>()[i];
                    
                    float error = 0;
                    if (hits > 0 ) error = sqrt(float(hits));

                    LOG(INFO) << "filling hybrid " << hybrid->getIndex() << " with latency " << lat << " and hits " << hits << " and error " << error;
                    hybridLatencyHistogram->SetBinContent(i, hits);
                    hybridLatencyHistogram->SetBinError(i, error);
                }
            }

        }
        
    }
}

void DQMHistogramLatencyScan::fillStubScanLatencyPlots(DetectorDataContainer& theStubScanLatency)
{
    for(auto board: theStubScanLatency)
    {
        for(auto opticalGroup: *board)
        {
            for(auto hybrid: *opticalGroup)
            {
                if(!hybrid->hasSummary()) continue;
                TH1F* hybridLatencyHistogram =
                    fStubScanLatencyHistograms.at(board->getIndex())->at(opticalGroup->getIndex())->at(hybrid->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;

                for(uint32_t i = 0; i < fLatencyRange ; i++){

                    uint32_t lat = i + fStartLatency;
                    uint32_t hits = hybrid->getSummary<GenericDataArray<VECSIZE, uint32_t>>()[i];
                    
                    float error = 0;
                    if (hits > 0 ) error = sqrt(float(hits));

                    LOG(INFO) << "filling hybrid " << hybrid->getIndex() << " with latency " << lat << " and hits " << hits << " and error " << error;
                    hybridLatencyHistogram->SetBinContent(i, hits);
                    hybridLatencyHistogram->SetBinError(i, error);
                }
            }

        }
    }
}


void DQMHistogramLatencyScan::fill2DLatency(DetectorDataContainer& the2DLatency)
{

}
void DQMHistogramLatencyScan::fillTriggerTDCPlots(DetectorDataContainer& theTriggerTDC)
{
    for(auto board: theTriggerTDC)
    {

        for(uint32_t tdcValue = 0; tdcValue < TDCBINS; ++tdcValue)
        {
            LOG(INFO) << "tdc value " << tdcValue;
            auto sum = board->at(0)->at(0)->getSummary<GenericDataArray<TDCBINS, uint32_t>>();
            LOG(INFO) << "with value " << sum[tdcValue];
            TH1F* boardTriggerTDCHistogram =
                    fTriggerTDCHistograms.at(board->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
            boardTriggerTDCHistogram->SetBinContent(tdcValue + 1, sum[tdcValue]); 
        }
    }

}

void DQMHistogramLatencyScan::parseSettings(const Ph2_System::SettingsMap& pSettingsMap)
{
    auto cSetting = pSettingsMap.find("StartLatency");
    if(cSetting != std::end(pSettingsMap))
        fStartLatency = cSetting->second;
    else
        fStartLatency = 0;

    cSetting = pSettingsMap.find("LatencyRange");
    if(cSetting != std::end(pSettingsMap))
        fLatencyRange = cSetting->second;
    else
        fLatencyRange = 512;

    cSetting = pSettingsMap.find("Nevents");
    if(cSetting != std::end(pSettingsMap))
        fNEvents = cSetting->second;
    else
        fNEvents = 100;
}
