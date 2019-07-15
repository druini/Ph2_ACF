/*!
  \file                  RD53ThrEqualization.h
  \brief                 Implementaion of threshold equalization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53ThrEqualization_H
#define RD53ThrEqualization_H

#include "../Utils/Container.h"
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/ThresholdAndNoise.h"
#include "Tool.h"

#include "TApplication.h"
#include "TH2F.h"


// #############
// # CONSTANTS #
// #############
#define TARGETeff 0.50 // Target efficiency for optimization algorithm


// #####################################
// # Threshold equalization test suite #
// #####################################
class ThrEqualization : public Tool
{
 public:
  ThrEqualization  (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst);
  ~ThrEqualization ();

  void Run  (std::shared_ptr<DetectorDataContainer> newVCal = nullptr);
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
  size_t nEvtsBurst;  
  
  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  DetectorDataContainer theOccContainer;
  DetectorDataContainer theTDACcontainer;

  void InitHisto       ();
  void FillHisto       ();
  void Display         ();
  void Save            ();
  void ChipErrorReport ();


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
