/*!
  \file                  RD53PixelAliveHistograms.h
  \brief                 Header file of PixelAlive calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53PixelAliveHistograms_H
#define RD53PixelAliveHistograms_H

#include "../System/SystemController.h"
#include "../Utils/GenericDataVector.h"
#include "../Utils/OccupancyAndPh.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>
#include <TH2F.h>


class RD53PixelAliveHistograms : public DQMHistogramBase
{
 public:
  RD53PixelAliveHistograms (size_t nEvents) : nEvents(nEvents) {}

  void book    (TFile* theOutputFile, const DetectorContainer& theDetectorStructure, Ph2_System::SettingsMap pSettingsMap) override;
  void process ()                                                                                                          override;
  bool fill    (std::vector<char>& dataBuffer)                                                                             override { return false; };
  void reset   (void)                                                                                                      override {};

  void fill    (const DetectorDataContainer& data);

 private:
  DetectorDataContainer Occupancy1D;
  DetectorDataContainer Occupancy2D;
  DetectorDataContainer ErrorReadOut2D;
  DetectorDataContainer ToT;
  DetectorDataContainer BCID;
  DetectorDataContainer TriggerID;
  
  size_t nEvents;
};

#endif
