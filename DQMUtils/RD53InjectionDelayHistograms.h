/*!
  \file                  RD53InjectionDelayHistograms.h
  \brief                 Header file of InjectionDelay calibration histograms
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53InjectionDelayHistograms_H
#define RD53InjectionDelayHistograms_H

#include "../System/SystemController.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>


class RD53InjectionDelayHistograms : public DQMHistogramBase
{
 public:
  void book    (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap) override;
  void process ()                                                                                                          override;
  bool fill    (std::vector<char>& dataBuffer)                                                                             override { return false; };
  void reset   (void)                                                                                                      override {};

  void fill    (const DetectorDataContainer& OccupancyContainer, const DetectorDataContainer& InjectionDelayContainer);

 private:
  DetectorDataContainer Occupancy1D;
  DetectorDataContainer InjectionDelay;

  size_t startValue;
  size_t stopValue;
};

#endif
