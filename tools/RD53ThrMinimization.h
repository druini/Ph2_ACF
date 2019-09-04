/*!
  \file                  RD53ThrMinimization.h
  \brief                 Implementaion of threshold minimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53ThrMinimization_H
#define RD53ThrMinimization_H

#include "../DQMUtils/RD53ThrMinimizationHistograms.h"
#include "RD53PixelAlive.h"


// #####################################
// # Threshold minimization test suite #
// #####################################
class ThrMinimization : public PixelAlive
{
 public:
  ThrMinimization  (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nEvents, size_t nEvtsBurst, float targetOccupancy, size_t ThrStart = 0, size_t ThrStop = 0);

  void   run                 ();
  void   draw                (bool display, bool save);
  void   analyze             ();
  size_t getNumberIterations () { return RD53ChannelGroupHandler::getNumberOfGroups(true)*floor(log2(ThrStop - ThrStart + 1) + 3) * nEvents/nEvtsBurst; }

 private:
  const char* fileRes;
  const char* fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t nEvtsBurst;
  size_t ThrStart;
  size_t ThrStop;
  float  targetOccupancy;

  DetectorDataContainer theThrContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void bitWiseScan     (const std::string& dacName, uint32_t nEvents, const float& target, uint16_t ThrStart, uint16_t ThrStop);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  RD53ThrMinimizationHistograms histos;
};

#endif
