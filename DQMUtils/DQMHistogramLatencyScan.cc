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
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fDetectorLatencyHistograms, hLatency);

    HistContainer<TH1F> hStub("StubValue", "Stub Value", 1, 0, 1);
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fDetectorStubHistograms, hStub);

    HistContainer<TH2F> hLatencyScan2D("LatencyScan2D", "LatencyScan2D", 1, 0, 1, 1, 0, 1);
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fDetectorLatencyScan2DHistograms, hLatencyScan2D);
    
    HistContainer<TH1F> hTriggerTDC("TriggerTDC", "Trigger TDC", fTDCBins, -0.5, fTDCBins - 0.5);
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fTriggerTDC, hTriggerTDC);
    
    for(auto cBoard: *fDetectorContainer)
    {
        uint32_t cBoardId = cBoard->getId();

        TH1F* cTriggerTDC = new TH1F(Form("h_BeBoard_triggerTDC_Be%d", cBoardId), Form("Trigger TDC BE%d; Trigger TDC; # of Hits", cBoardId), fTDCBins, -0.5, fTDCBins - 0.5);

        cTriggerTDC->SetFillColor(4);
        cTriggerTDC->SetFillStyle(3001);
        bookHistogram(cBoard, "triggerTDC", cTriggerTDC);

        for(auto cOpticalGroup: *cBoard)
        {
            for(auto cFe: *cOpticalGroup)
            {
                uint32_t cFeId = cFe->getId();

                TCanvas* ctmpCanvas = new TCanvas(Form("c_online_canvas_fe%d", cFeId), Form("FE%d  Online Canvas", cFeId));
                // ctmpCanvas->Divide( 2, 2 );
                fCanvasMap[cFe] = ctmpCanvas;

                fNCbc = cFe->size();

                // 1D Hist forlatency scan
                TString  cName = Form("h_hybrid_latency_Fe%d", cFeId);
                TObject* cObj  = gROOT->FindObject(cName);

                if(cObj) delete cObj;

                TH1F* cLatHist = nullptr;

                cLatHist = new TH1F(cName, Form("Latency FE%d; Latency; # of Hits", cFeId), (fLatencyRange), fStartLatency - 0.5, fStartLatency + (fLatencyRange)-0.5);

                cLatHist->GetXaxis()->SetTitle("Trigger Latency");
                cLatHist->SetFillColor(4);
                cLatHist->SetFillStyle(3001);
                bookHistogram(cFe, "hybrid_latency", cLatHist);

                cName = Form("h_hybrid_stub_latency_Fe%d", cFeId);
                cObj  = gROOT->FindObject(cName);

                if(cObj) delete cObj;

                TH1F* cStubHist = new TH1F(cName, Form("Stub Lateny FE%d; Stub Lateny; # of Stubs", cFeId), fLatencyRange, fStartLatency, fStartLatency + fLatencyRange);
                cStubHist->SetMarkerStyle(2);
                bookHistogram(cFe, "hybrid_stub_latency", cStubHist);

                cName                = Form("h_hybrid_latency_2D_Fe%d", cFeId);
                TH2D* cLatencyScan2D = new TH2D(cName,
                                                Form("Latency FE%d; Stub Latency; L1 Latency; # of Events w/ no Hits and no Stubs", cFeId),
                                                fLatencyRange,
                                                fStartLatency - 0.5,
                                                fStartLatency + (fLatencyRange)-0.5,
                                                fLatencyRange,
                                                fStartLatency - 0.5,
                                                fStartLatency + (fLatencyRange)-0.5);
                bookHistogram(cFe, "hybrid_latency_2D", cLatencyScan2D);
            }
        }
    }


}

//========================================================================================================================
bool DQMHistogramLatencyScan::fill(std::vector<char>& dataBuffer) { return false; }

//========================================================================================================================
void DQMHistogramLatencyScan::process() {}

//========================================================================================================================

void DQMHistogramLatencyScan::reset(void) {}
