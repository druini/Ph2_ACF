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
#include "../Utils/DataContainer.h"
#include "../Utils/Occupancy.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/ThresholdAndNoise.h"
#include "Tool.h"

#include "TH2F.h"


using namespace Ph2_System;

// #####################
// # SCurve test suite #
// #####################
class SCurve : public Tool
{
 public:
  SCurve(const char* fName, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps);
  ~SCurve();

  void InitHisto();
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
  size_t startValue;
  size_t stopValue;
  size_t nSteps;

  std::vector<uint16_t> dacList;

  std::vector<DetectorDataContainer*> detectorContainerVector;
  DetectorDataContainer*              theThresholdAndNoiseContainer;

  void ComputeStats (std::vector<float>& measurements, size_t offset, float& nHits, float& mean, float& rms);


  // ########
  // # ROOT #
  // ########
  TFile* theFile;
  std::vector<TCanvas*> theCanvasOcc;
  std::vector<TH2F*>    theOccupancy;
  std::vector<TCanvas*> theCanvasTh1D;
  std::vector<TH1F*>    theThreshold1D;
  std::vector<TCanvas*> theCanvasNo1D;
  std::vector<TH1F*>    theNoise1D;
  std::vector<TCanvas*> theCanvasTh2D;
  std::vector<TH2F*>    theThreshold2D;
  std::vector<TCanvas*> theCanvasNo2D;
  std::vector<TH2F*>    theNoise2D;
};

#endif
