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
#include "../Utils/OccupancyAndPh.h"
#include "../Utils/GenericDataVector.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "Tool.h"

#include "TApplication.h"
#include "TH2F.h"


using namespace Ph2_System;

// #########################
// # PixelAlive test suite #
// #########################
class PixelAlive : public Tool
{
 public:
  PixelAlive(const char* fileName, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, bool inject);
  ~PixelAlive();

  void Run     ();
  void Draw    (bool display, bool save);
  void Analyze ();
  
 private:
  const char* fileName;
  size_t rowStart;
  size_t rowEnd;
  size_t colStart;
  size_t colEnd;
  size_t nPixels2Inj;
  size_t nEvents;
  size_t nEvtsBurst;  
  bool   inject;

  DetectorDataContainer theOccupancyContainer;
  DetectorDataContainer theBCIDcontainer;

  void InitHisto ();
  void FillHisto ();
  void Display   ();
  void Save      ();


  // ########
  // # ROOT #
  // ########
  TFile* theFile;
  std::vector<TCanvas*> theCanvasOcc2D;
  std::vector<TH2F*>    theOcc2D;
  std::vector<TCanvas*> theCanvasToT;
  std::vector<TH1F*>    theToT;
  std::vector<TCanvas*> theCanvasOcc1D;
  std::vector<TH1F*>    theOcc1D;
  std::vector<TCanvas*> theCanvasBCID;
  std::vector<TH1F*>    theBCID;
  std::vector<TCanvas*> theCanvasErr;
  std::vector<TH2F*>    theErr;
};

#endif
