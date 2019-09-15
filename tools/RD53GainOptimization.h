/*!
  \file                  RD53GainOptimization.h
  \brief                 Implementaion of gain optimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53GainOptimization_H
#define RD53GainOptimization_H

#include "../DQMUtils/RD53GainOptimizationHistograms.h"
#include "RD53Gain.h"


// #############
// # CONSTANTS #
// #############
#define RESULTDIR "Results" // Directory containing the results


// ################################
// # Gain optimization test suite #
// ################################
class GainOptimization : public Gain
{
 public:
  GainOptimization (std::string fileRes,
                    std::string fileReg,
                    size_t rowStart,
                    size_t rowStop,
                    size_t colStart,
                    size_t colStop,
                    size_t nEvents,
                    size_t startValue,
                    size_t stopValue,
                    size_t nSteps,
                    size_t offset,
                    float  targetCharge,
                    size_t KrumCurrStart,
                    size_t KrumCurrStop,
                    bool   doFast = false);

  void   run                 ();
  void   analyze             ();
  void   draw                (bool display, bool save);
  size_t getNumberIterations ()
  {
    uint16_t nBitKrumCurr   = floor(log2(KrumCurrStop - KrumCurrStart + 1) + 1);
    uint16_t moreIterations = 2;
    return Gain::getNumberIterations()*(nBitKrumCurr + moreIterations);
  }

 private:
  std::string fileRes;
  std::string fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t startValue;
  size_t stopValue;
  size_t nSteps;
  float  targetCharge;
  size_t KrumCurrStart;
  size_t KrumCurrStop;
  bool   doFast;

  DetectorDataContainer theKrumCurrContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void bitWiseScan     (const std::string& regName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  RD53GainOptimizationHistograms histos;
};

#endif
