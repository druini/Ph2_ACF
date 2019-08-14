/*!
  \file                  RD53LatencyHistograms.h
  \brief                 Header file of Latency calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53LatencyHistograms_H
#define RD53LatencyHistograms_H

#include "../Utils/GenericDataVector.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>
#include <TH2F.h>


class RD53LatencyHistograms : public DQMHistogramBase
{
 public:
 RD53LatencyHistograms (size_t startValue, size_t stopValue) : startValue(startValue), stopValue(stopValue) {}

  void book    (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, uint32_t> pSettingsMap) override;
  void process ()                                                                                                                  override;
  bool fill    (std::vector<char>& dataBuffer)                                                                                     override { return false; };
  void reset   (void)                                                                                                              override {};

  void fill    (const DetectorDataContainer& data);

 private:
  DetectorDataContainer Occupancy1D;
  
  size_t startValue;
  size_t stopValue;
};

#endif