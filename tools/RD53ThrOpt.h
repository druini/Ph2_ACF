/*!
  \file                  RD53ThrOpt.h
  \brief                 Implementaion of threshold equalization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53ThrOpt_h_
#define _RD53ThrOpt_h_

#include "../Utils/Container.h"
#include "../Utils/DataContainer.h"
#include "../Utils/OccupancyPhTrim.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/ThresholdAndNoise.h"
#include "Tool.h"

#include "TH2F.h"


// #############
// # CONSTANTS #
// #############
#define TARGETeff 0.56 // Target efficiency for optimization algorithm


using namespace Ph2_System;

// #####################################
// # Threshold equalization test suite #
// #####################################
class ThrOpt : public Tool
{
 public:
  ThrOpt(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nEvts);
  ~ThrOpt();

  void InitHisto();
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
  size_t nEvents;
  float  targetTh;


  // ########
  // # ROOT #
  // ########
  TFile* theFile;
  std::vector<TCanvas*> theCanvasOcc;
  std::vector<TH1F*>    theOccupancy;
  std::vector<TCanvas*> theCanvasTDAC;
  std::vector<TH1F*>    theTDAC;
};

#endif
