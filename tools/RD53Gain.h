/*!
  \file                  RD53Gain.h
  \brief                 Implementaion of Gain scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53Gain_H
#define RD53Gain_H

#include "../Utils/Container.h"
#include "../Utils/OccupancyAndPh.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/GainAndIntercept.h"
#include "Tool.h"

#include "TApplication.h"
#include "TStyle.h"
#include "TGaxis.h"
#include "TH2F.h"


// #############
// # CONSTANTS #
// #############
#define INTERCEPT_HALFRANGE 6 // [ToT]


// ##########################
// # Gain measurement suite #
// ##########################
class Gain : public Tool
{
 public:
  Gain  (const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nPixels2Inj, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps);
  ~Gain ();

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
  std::shared_ptr<DetectorDataContainer>   theGainAndInterceptContainer;

  void InitHisto       ();
  void FillHisto       ();
  void Display         ();
  void Save            ();
  void ComputeStats    (std::vector<float>& x, std::vector<float>& y, std::vector<float>& e, double& gain, double& gainErr, double& intercept, double& interceptErr);
  void ChipErrorReport ();


  // ########
  // # ROOT #
  // ########
  TFile* theFile;
  std::vector<TCanvas*> theCanvasOcc;
  std::vector<TH2F*>    theOccupancy;
  std::vector<TCanvas*> theCanvasGa1D;
  std::vector<TH1F*>    theGain1D;
  std::vector<TCanvas*> theCanvasIn1D;
  std::vector<TH1F*>    theIntercept1D;
  std::vector<TCanvas*> theCanvasGa2D;
  std::vector<TH2F*>    theGain2D;
  std::vector<TCanvas*> theCanvasIn2D;
  std::vector<TH2F*>    theIntercept2D;

  std::vector<TGaxis*>  theAxis;
};

#endif
