/*!
  \file                  RD53SCurve.h
  \brief                 Implementaion of SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53SCurve_H
#define RD53SCurve_H

#include "../Utils/Container.h"
#include "../Utils/Occupancy.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/ThresholdAndNoise.h"
#include "Tool.h"

#include "TApplication.h"
#include "TStyle.h"
#include "TGaxis.h"
#include "TH2F.h"


// #####################
// # SCurve test suite #
// #####################
class SCurve : public Tool
{
 public:
  SCurve  (const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps);
  ~SCurve ();

  void Run                                       ();
  void Draw                                      (bool display, bool save);
  std::shared_ptr<DetectorDataContainer> Analyze ();

 private:
  const char* fileRes;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nPixels2Inj;
  size_t nEvents;
  size_t startValue;
  size_t stopValue;
  size_t nSteps;

  std::vector<uint16_t> dacList;
  
  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  std::vector<DetectorDataContainer*>      detectorContainerVector;
  std::shared_ptr<DetectorDataContainer>   theThresholdAndNoiseContainer;

  void InitHisto       ();
  void FillHisto       ();
  void Display         ();
  void Save            ();
  void ComputeStats    (std::vector<float>& measurements, int offset, float& nHits, float& mean, float& rms);
  void ChipErrorReport ();


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

  std::vector<TGaxis*>  theAxis;
};

#endif
