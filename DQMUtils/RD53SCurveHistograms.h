/*!
  \file                  RD53SCurveHistograms.h
  \brief                 Header file of SCurve calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53SCurveHistograms_H
#define RD53SCurveHistograms_H

#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/Occupancy.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>
#include <TH2F.h>


class RD53SCurveHistograms : public DQMHistogramBase
{
 public:
 RD53SCurveHistograms(int nEvents, int startValue, int stopValue, int nSteps)
   : nEvents    (nEvents)
   , nSteps     (nSteps)
   , startValue (startValue)
   , stopValue  (stopValue)
  {}

  void book               (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, double> pSettingsMap) override;
  void process            ()                                                                                                                  override;
  bool fill               (std::vector<char>& dataBuffer)                                                                                     override { return false; };
  void reset              (void)                                                                                                              override {};

  void fillOccupancy      (const DetectorDataContainer& data, int VCAL_HIGH);
  void fillThresholdNoise (const DetectorDataContainer& data);

 private:
  DetectorDataContainer Occupancy2D;
  DetectorDataContainer Threshold1D;
  DetectorDataContainer Noise1D;
  DetectorDataContainer Threshold2D;
  DetectorDataContainer Noise2D;

  size_t nEvents;
  size_t nSteps;
  size_t startValue;
  size_t stopValue;
  size_t ROWstart;
  size_t ROWstop;
  size_t COLstart;
  size_t COLstop;
};

#endif
