/*!
  \file                  RD53PixelAlive.h
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53PixelAlive_h_
#define _RD53PixelAlive_h_

#include "../Utils/Container.h"
#include "../Utils/Occupancy.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "Tool.h"

#include "TH2F.h"


// #############
// # CONSTANTS #
// #############
#define NHISTO 1


using namespace Ph2_System;

// #########################
// # PixelAlive test suite #
// #########################
class PixelAlive : public Tool
{
 public:
  PixelAlive(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nTrig, bool inject);
  ~PixelAlive();

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
  
  bool inject;

  std::bitset<RD53::nRows * RD53::nCols> customBitset;
  ChannelGroup<RD53::nRows,RD53::nCols>* customChannelGroup;
  
  TFile*   theFile;
  TCanvas* theCanvas;
  std::vector<TH2F*> theOccupancy;
};

#endif
