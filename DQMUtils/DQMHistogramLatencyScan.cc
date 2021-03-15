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
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"

//========================================================================================================================
DQMHistogramLatencyScan::DQMHistogramLatencyScan() {
    fStartLatency = 0;
    fStartLatency = 0;

}

//========================================================================================================================
DQMHistogramLatencyScan::~DQMHistogramLatencyScan() {}

//========================================================================================================================
void DQMHistogramLatencyScan::book(TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const Ph2_System::SettingsMap& pSettingsMap)
{

    uint32_t fTDCBins = 8; //from LatencyScan.h

    ContainerFactory::copyStructure(theDetectorStructure, fDetectorData);

    HistContainer<TH1F> hLatency("LatencyValue", "Latency Value", 1, 0, 1);
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fLatencyHistograms, hLatency);

    HistContainer<TH1F> hStub("StubValue", "Stub Value", 1, 0, 1);
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fStubHistograms, hStub);

    HistContainer<TH2F> hLatencyScan2D("LatencyScan2D", "LatencyScan2D", 1, 0, 1, 1, 0, 1);
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fLatencyScan2DHistograms, hLatencyScan2D);
    
    HistContainer<TH1F> hTriggerTDC("TriggerTDC", "Trigger TDC", fTDCBins, -0.5, fTDCBins - 0.5);
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fTriggerTDC, hTriggerTDC);




}

//========================================================================================================================
bool DQMHistogramLatencyScan::fill(std::vector<char>& dataBuffer) { return false; }

//========================================================================================================================
void DQMHistogramLatencyScan::process() {

    for(auto board: fLatencyHistograms)
    {
        for(auto opticalGroup: *board)
        {
            for(auto hybrid: *opticalGroup)
            {
                TCanvas* latencyCanvas    = new TCanvas(("Latency_" + std::to_string(hybrid->getId())).data(), ("Latency " + std::to_string(hybrid->getId())).data(), 10, 0, 500, 500);

                latencyCanvas->DivideSquare(hybrid->size());

                for(auto chip: *hybrid)
                {
                    latencyCanvas->cd(chip->getIndex() + 1);
                    TH1F* latencyHistogram = chip->getSummary<HistContainer<TH1F>>().fTheHistogram;
                    latencyHistogram->GetXaxis()->SetTitle("Trigger Latency");
                    latencyHistogram->GetYaxis()->SetTitle("# of hits");
                    latencyHistogram->DrawCopy();

                }
            }
        }
    }    
}

//========================================================================================================================

void DQMHistogramLatencyScan::reset(void) {}

void DQMHistogramLatencyScan::fillLatency(DetectorDataContainer& theLatency)
{ 

}
void DQMHistogramLatencyScan::fillStubLatency(DetectorDataContainer& theStubLatency)
{

}
void DQMHistogramLatencyScan::fill2DLatency(DetectorDataContainer& the2DLatency)
{

}
void DQMHistogramLatencyScan::fillTriggerTDC(DetectorDataContainer& theTriggerTDC)
{

}
