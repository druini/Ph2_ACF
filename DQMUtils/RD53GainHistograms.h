/*!
  \file                  RD53GainHistograms.h
  \brief                 Header file of Gain calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53GainHistograms_H
#define RD53GainHistograms_H

#include "../System/SystemController.h"
#include "../Utils/GainAndIntercept.h"
#include "../Utils/OccupancyAndPh.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>
#include <TH2F.h>

// #############
// # CONSTANTS #
// #############
#define INTERCEPT_HALFRANGE 6 // [ToT]


class RD53GainHistograms : public DQMHistogramBase
{
 public:
 RD53GainHistograms (int nEvents, int startValue, int stopValue, int nSteps)
   : nEvents    (nEvents)
   , nSteps     (nSteps)
   , startValue (startValue)
   , stopValue  (stopValue)
  {}
  
  void book              (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap) override;
  void process           ()                                                                                                          override;
  bool fill              (std::vector<char>& dataBuffer)                                                                             override { return false; };
  void reset             (void)                                                                                                      override {};

  void fillOccupancy     (const DetectorDataContainer& data, int VCAL_HIGH);
  void fillGainIntercept (const DetectorDataContainer& data);

 private:
  DetectorDataContainer Occupancy2D;
  DetectorDataContainer Gain1D;
  DetectorDataContainer Intercept1D;
  DetectorDataContainer Gain2D;
  DetectorDataContainer Intercept2D;

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
