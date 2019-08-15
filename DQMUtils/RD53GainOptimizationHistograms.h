/*!
  \file                  RD53GainOptimizationHistograms.h
  \brief                 Header file of Gain optimization calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53GainOptimizationHistograms_H
#define RD53GainOptimizationHistograms_H

#include "../Utils/EmptyContainer.h"
#include "../Utils/RegisterValue.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>
#include <TH2F.h>


class RD53GainOptimizationHistograms : public DQMHistogramBase
{
 public:
  RD53GainOptimizationHistograms () {}
  
  void book    (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, double> pSettingsMap) override;
  void process ()                                                                                                                  override;
  bool fill    (std::vector<char>& dataBuffer)                                                                                     override { return false; };
  void reset   (void)                                                                                                              override {};
  
  void fill    (const DetectorDataContainer& data);
  
 private:
  DetectorDataContainer KrumCurr;
};

#endif
