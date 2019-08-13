/*!
  \file                  RD53ThrMinimizationHistograms.h
  \brief                 Header file of Gain calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53ThrMinimizationHistograms_H
#define RD53ThrMinimizationHistograms_H

#include "../Utils/EmptyContainer.h"
#include "../Utils/RegisterValue.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>


class RD53ThrMinimizationHistograms : public DQMHistogramBase
{
 public:
 RD53ThrMinimizationHistograms() //(int nEvents, int startValue, int stopValue, int nSteps)
//    : nEvents    (nEvents)
//    , nSteps     (nSteps)
//    , startValue (startValue)
//    , stopValue  (stopValue)
  {}

  void book               (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, uint32_t> pSettingsMap) override;
  void process            ()                                                                                                                  override;
  bool fill               (std::vector<char>& dataBuffer)                                                                                     override { return false; };
  void reset              (void)                                                                                                              override {};

  void fill  (const DetectorDataContainer& data);

 private:
  DetectorDataContainer Threhsold;

//   size_t nEvents;
//   size_t nSteps;
//   size_t startValue;
//   size_t stopValue;
//   size_t ROWstart;
//   size_t ROWstop;
//   size_t COLstart;
//   size_t COLstop;
};

#endif
