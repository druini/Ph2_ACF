/*!
  \file                  RD53SCurve.h
  \brief                 Implementaion of SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53SCurve_h_
#define _RD53SCurve_h_

#include "../Utils/Container.h"
#include "../Utils/OccupancyAndToT.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "Tool.h"

#include "TH2F.h"


// #############
// # CONSTANTS #
// #############
#define NHISTO 1


using namespace Ph2_System;

// #####################
// # SCurve test suite #
// #####################
class SCurve : public Tool
{
 public:
  SCurve(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nEvts, float startValue, float stopValue, size_t nSteps);
  ~SCurve();

  void Run();
  void Display();
  void Analyze();
  void Save();

 private:
  const char* fileName;
  size_t rowStart;
  size_t rowEnd;
  size_t colStart;
  size_t colEnd;
  size_t nPixels2Inj;
  size_t nEvents;

  float  startValue;
  float  stopValue;
  size_t nSteps;

  std::vector<uint16_t> dacList;

  std::bitset<RD53::nRows * RD53::nCols> customBitset;
  ChannelGroup<RD53::nRows,RD53::nCols>* customChannelGroup;
  std::vector<DetectorContainer*> detectorContainerVector;

  void ComputeStats(std::vector<double>& measurements, double& mean, double& rms);

  TFile*   theFile;
  TCanvas* theCanvas;
  TCanvas* theCanvasTh1D;
  TCanvas* theCanvasNo1D;
  TCanvas* theCanvasTh2D;
  TCanvas* theCanvasNo2D;
  std::vector<TH2F*> theOccupancy;
  TH1F* theNoise1D;
  TH1F* theThreshold1D;
  TH2F* theNoise2D;
  TH2F* theThreshold2D;
};

#endif
