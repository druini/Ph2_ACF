/*!
  \file                  RD53SCurve.h
  \brief                 Implementaion pf SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53SCurve_h_
#define _RD53SCurve_h_

#include "../Utils/Container.h"
#include "../Utils/Occupancy.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "Tool.h"

#include "TH2F.h"


using namespace Ph2_System;

// #########################
// # SCurve test suite #
// #########################
class SCurve : public Tool
{
 public:
  SCurve(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nTrig, float startValue, float stopValue, int nSteps);
  ~SCurve();

  void Run();
  void Display();
  void Save();
  
 private:
  const char* fileName;
  size_t rowStart;
  size_t rowEnd;
  size_t colStart;
  size_t colEnd;
  size_t nPixels2Inj;
  size_t nTriggers;
  float startValue;
  float stopValue;
  int nSteps;

  std::bitset<RD53::nRows * RD53::nCols> customBitset;
  ChannelGroup<RD53::nRows,RD53::nCols>* customChannelGroup;
  
  TFile*   theFile;
  TCanvas* theCanvas;
  std::vector<TH2F*> theOccupancy;
};

#endif
