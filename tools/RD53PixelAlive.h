/*!
  \file                  RD53PixelAlive.h
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53PixelAlive_H
#define RD53PixelAlive_H

#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../DQMUtils/RD53PixelAliveHistograms.h"
#include "Tool.h"

#include "TApplication.h"


// #########################
// # PixelAlive test suite #
// #########################
class PixelAlive : public Tool
{
 public:
  PixelAlive (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, bool inject, float thresholdOccupancy = 0);

  void run                                       ();
  void draw                                      (bool display, bool save);
  std::shared_ptr<DetectorDataContainer> analyze ();

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
  bool   inject;
  float  thresholdOccupancy;

  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  std::shared_ptr<DetectorDataContainer>   theOccContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  RD53PixelAliveHistograms histos;
};

#endif
