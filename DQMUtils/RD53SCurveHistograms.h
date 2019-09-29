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

#include "../System/SystemController.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>
#include <TH2F.h>


// #############
// # CONSTANTS #
// #############
#define ISDISABLED -1.0 // Encoding disabled channels
#define FITERROR   -2.0 // Encoding fit errors


class SCurveHistograms : public DQMHistogramBase
{
 public:
  void book          (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap settingsMap) override;
  void process       ()                                                                                                         override;
  bool fill          (std::vector<char>& dataBuffer)                                                                            override;
  void reset         ()                                                                                                         override {};

  void fillOccupancy   (const DetectorDataContainer& OccupancyContainer, int DELTA_VCAL);
  void fillThrAndNoise (const DetectorDataContainer& ThrAndNoiseContainer);

 private:
  DetectorDataContainer DetectorData;

  DetectorDataContainer Occupancy2D;
  DetectorDataContainer ErrorReadOut2D;
  DetectorDataContainer ErrorFit2D;
  DetectorDataContainer Threshold1D;
  DetectorDataContainer Noise1D;
  DetectorDataContainer Threshold2D;
  DetectorDataContainer Noise2D;

  size_t nEvents;
  size_t nSteps;
  size_t startValue;
  size_t stopValue;
  size_t offset;
};

#endif
