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

#include "RD53PixelAlive.h"

#ifdef __USE_ROOT__
#include "../DQMUtils/RD53ThrMinimizationHistograms.h"
#endif


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
  void Start (int currentRun)  override;
  void Stop  ()                override;
  void ConfigureCalibration () override;

  void   sendData            ();
  void   initialize          (const std::string fileRes_, const std::string fileReg_);
  void   run                 ();
  void   draw                ();
  void   analyze             ();
  size_t getNumberIterations ()
  {
    uint16_t nBitThr        = floor(log2(ThrStop - ThrStart + 1) + 1);
    uint16_t moreIterations = 2;
    return PixelAlive::getNumberIterations()*(nBitThr + moreIterations);
  }


 private:
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  float  targetOccupancy;
  size_t ThrStart;
  size_t ThrStop;

  DetectorDataContainer theThrContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void bitWiseScan     (const std::string& regName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
#ifdef __USE_ROOT__
  ThrMinimizationHistograms histos;
#endif


 protected:
  std::string fileRes;
  std::string fileReg;
  bool doUpdateChip;
  bool doDisplay;
  bool saveBinaryData;
};

#endif
