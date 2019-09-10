/*!
  \file                  RD53ThrEqualizationHistograms.h
  \brief                 Header file of ThrEqualization calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53ThrEqualizationHistograms_H
#define RD53ThrEqualizationHistograms_H

#include "../System/SystemController.h"
#include "../Utils/OccupancyAndPh.h"
#include "../Utils/RegisterValue.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>
#include <TH2F.h>


class RD53ThrEqualizationHistograms : public DQMHistogramBase
{
 public:
  RD53ThrEqualizationHistograms (size_t nEvents) : nEvents(nEvents) {}

  void book    (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap) override;
  void process ()                                                                                                          override;
  bool fill    (std::vector<char>& dataBuffer)                                                                             override { return false; };
  void reset   (void)                                                                                                      override {};

  void fill    (const DetectorDataContainer& OccupancyContainer, const DetectorDataContainer& TDACContainer);

 private:
  DetectorDataContainer ThrEqualization;
  DetectorDataContainer TDAC;

  size_t nEvents;
};

#endif
