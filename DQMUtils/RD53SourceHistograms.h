/*!
  \file                  RD53SourceHistograms.h
  \brief                 Header file of Source calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53SourceHistograms_H
#define RD53SourceHistograms_H

#include "../Utils/Containable.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>
#include <TH2F.h>


class RD53SourceHistograms : public DQMHistogramBase
{
 public:
 RD53SourceHistograms(int startValue, int stopValue, int nSteps, int duration)
   : nSteps     (nSteps)
   , startValue (startValue)
   , stopValue  (stopValue)
   , duration   (duration)
  {}

  void book               (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, std::map<std::string, double> pSettingsMap) override;
  void process            ()                                                                                                                  override;
  bool fill               (std::vector<char>& dataBuffer)                                                                                     override { return false; };
  void reset              (void)                                                                                                              override {};

  void fill(const DetectorDataContainer& data, int Vthreshold);

 private:
  DetectorDataContainer TrigRate;

//   size_t nEvents;
  size_t nSteps;
  size_t startValue;
  size_t stopValue;
  int duration;
//   size_t ROWstart;
//   size_t ROWstop;
//   size_t COLstart;
//   size_t COLstop;
};

#endif
