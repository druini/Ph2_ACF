/*!
  \file                  RD53GainOptimization.h
  \brief                 Implementaion of gain optimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53GainOptimization_h_
#define _RD53GainOptimization_h_

#include "../Utils/EmptyContainer.h"
#include "RD53Gain.h"


using namespace Ph2_System;

// ################################
// # Gain optimization test suite #
// ################################
class GainOptimization : public Gain
{
 public:
  GainOptimization  (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nPixels2Inj, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps, float targetCharge, size_t KrumCurrStart = 0, size_t KrumCurrStop = 0);
  ~GainOptimization ();

  void Run  ();
  void Draw (bool display, bool save);

 private:
  const char* fileRes;
  const char* fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nPixels2Inj;
  size_t nEvents;
  size_t startValue;
  size_t stopValue;
  size_t nSteps;
  size_t KrumCurrStart;
  size_t KrumCurrStop;
  float  targetCharge;
  
  DetectorDataContainer theKrumCurrContainer;

  void InitHisto       ();
  void FillHisto       ();
  void Display         ();
  void Save            ();
  void bitWiseScan     (const std::string& dacName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue);
  void ChipErrorReport ();


  // ########
  // # ROOT #
  // ########
  TFile* theFile;
  std::vector<TCanvas*> theCanvasKrumCurr;
  std::vector<TH1F*>    theKrumCurr;
};

#endif
