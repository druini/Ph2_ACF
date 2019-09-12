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


// #############
// # CONSTANTS #
// #############
#define RESULTDIR "Results" // Directory containing the results


// #####################################
// # Threshold minimization test suite #
// #####################################
class ThrMinimization : public PixelAlive
{
 public:
  ThrMinimization (std::string fileRes,
                   std::string fileReg,
                   size_t rowStart,
                   size_t rowStop,
                   size_t colStart,
                   size_t colStop,
                   size_t nEvents,
                   size_t nEvtsBurst,
                   float targetOccupancy,
                   size_t ThrStart = 0,
                   size_t ThrStop = 0,
                   bool   doFast = false);

  void   run                 ();
  void   draw                (bool display, bool save);
  void   analyze             ();
  size_t getNumberIterations ()
  {
    uint16_t nBitThr        = floor(log2(ThrStop - ThrStart + 1) + 1);
    uint16_t moreIterations = 2;
    return PixelAlive::getNumberIterations()*(nBitThr + moreIterations);
  }

 private:
  std::string fileRes;
  std::string fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t nEvtsBurst;
  size_t ThrStart;
  size_t ThrStop;
  float  targetOccupancy;
  bool   doFast;

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
