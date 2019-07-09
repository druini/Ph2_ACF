/*!
  \file                  RD53PixelAliveHistograms.h
  \brief                 Header file of PixelAlive calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53PixelAliveHistograms_h_
#define _RD53PixelAliveHistograms_h_

#include "../RootUtils/RootContainerFactory.h"
#include "../RootUtils/HistContainer.h"
#include "../Utils/GenericDataVector.h"
#include "../Utils/OccupancyAndPh.h"
#include "RD53HistogramsBase.h"

#include <TH1F.h>
#include <TH2F.h>

class RD53PixelAliveHistograms : RD53HistogramsBase
{
 public:
 RD53PixelAliveHistograms (size_t nEvents) : nEvents(nEvents) {}

  void book (TFile* theOutputFile, const DetectorContainer& theDetectorStructure);
  
  void fill (const DetectorDataContainer& data);

  void process();

 private:
  DetectorDataContainer Occupancy1D;
  DetectorDataContainer Occupancy2D;
  DetectorDataContainer ToT;
  DetectorDataContainer BCID;
  DetectorDataContainer TriggerID;
  
  size_t nEvents;
};

#endif
