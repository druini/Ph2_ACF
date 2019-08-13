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

#include "../Utils/EmptyContainer.h"
#include "../DQMUtils/RD53GainOptimizationHistograms.h"
#include "RD53Gain.h"


// ################################
// # Gain optimization test suite #
// ################################
class GainOptimization : public Gain
{
 public:
  GainOptimization (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop,  size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps, size_t offset, float targetCharge, size_t KrumCurrStart = 0, size_t KrumCurrStop = 0);

  void run  ();
  void draw (bool display, bool save);

 private:
  const char* fileRes;
  const char* fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t startValue;
  size_t stopValue;
  size_t nSteps;
  size_t KrumCurrStart;
  size_t KrumCurrStop;
  float  targetCharge;
  
  DetectorDataContainer theKrumCurrContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void bitWiseScan     (const std::string& dacName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  RD53GainOptimizationHistograms histos;
};

#endif
