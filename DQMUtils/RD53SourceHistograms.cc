/*!
  \file                  RD53SourceHistograms.cc
  \brief                 Implementation of Source calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SourceHistograms.h"

using namespace Ph2_HwDescription;

void RD53SourceHistograms::book(TFile* theOutputFile, const DetectorContainer& theDetectorStructure,
                                std::map<std::string, double> pSettingsMap)
{
    auto hTrigRate = HistContainer<TH1F>("Source", "Source", nSteps, startValue, stopValue);
    bookImplementer(theOutputFile, theDetectorStructure, hTrigRate, TrigRate, "Vthreshold", "Triggers per bunch crossing");
}

void RD53SourceHistograms::fill(const DetectorDataContainer& data, int Vthreshold)
{
    for (const auto cBoard : data)
        for (const auto cModule : *cBoard)
            for (const auto cChip : *cModule) {
                auto* hTrigRate = TrigRate.at(cBoard->getIndex())
                                   ->at(cModule->getIndex())
                                   ->at(cChip->getIndex())
                                   ->getSummary<HistContainer<TH1F> >()
                                   .fTheHistogram;

                LOG (INFO) << "Fill: " << duration << ". " << (duration * 40000.f) << ", " << cChip->getSummary<Containable<size_t>>().data << ", " << (cChip->getSummary<Containable<size_t>>().data / (duration * 40000.f)) << "\n";
                std::cout << "&data " << &(cChip->getSummary<Containable<size_t>>().data) << "\n";
                for (size_t i = 0; i < cChip->getSummary<Containable<size_t>>().data; i++)
                    hTrigRate->Fill(Vthreshold);
                // hTrigRate->SetBinError(Vthreshold, 0);
            }
}

void RD53SourceHistograms::process()
{
    draw<TH1F>(TrigRate, "HIST");
}
