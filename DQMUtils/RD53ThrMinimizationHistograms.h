/*!
  \file                  RD53ThrMinimizationHistograms.h
  \brief                 Header file of ThrMinimization calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53ThrMinimizationHistograms_H
#define RD53ThrMinimizationHistograms_H

#include "../System/SystemController.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>


class ThrMinimizationHistograms : public DQMHistogramBase
{
 public:
  void book    (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap) override;
  void process ()                                                                                                          override;
  bool fill    (std::vector<char>& dataBuffer)                                                                             override { return false; };
  void reset   ()                                                                                                          override {};

  void fill    (const DetectorDataContainer& data);

 private:
  DetectorDataContainer Threhsold;
};

#endif
