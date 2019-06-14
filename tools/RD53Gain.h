/*!
  \file                  RD53Gain.h
  \brief                 Implementaion of Gain scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53Gain_h_
#define _RD53Gain_h_

#include "../Utils/Container.h"
#include "../Utils/DataContainer.h"
#include "../Utils/OccupancyAndToT.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/GainAndIntercept.h"
#include "Tool.h"

#include "TH2F.h"


using namespace Ph2_System;

// ##########################
// # Gain measurement suite #
// ##########################
class Gain : public Tool
{
 public:
  Gain(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nEvts, size_t startValue, size_t stopValue, size_t nSteps);
  ~Gain();

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

  // std::bitset<RD53::nRows * RD53::nCols> customBitset;
  // ChannelGroup<RD53::nRows,RD53::nCols>* customChannelGroup;
  std::vector<DetectorDataContainer*>    detectorContainerVector;
  DetectorDataContainer*                 theGainAndInterceptContainer;

  void ComputeStats (std::vector<float>& x, std::vector<float>& y, std::vector<float>& e, double& gain, double& gainErr, double& intercept, double& interceptErr);


  // ########
  // # ROOT #
  // ########
  TFile* theFile;
  std::vector<TCanvas*> theCanvas;
  std::vector<TH2F*>    theOccupancy;
  TCanvas* theCanvasGa1D;
  TH1F*    theGain1D;
  TCanvas* theCanvasIn1D;
  TH1F*    theIntercept1D;
  TCanvas* theCanvasGa2D;
  TH2F*    theGain2D;
  TCanvas* theCanvasIn2D;
  TH2F*    theIntercept2D;
};

#endif
