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


// #############
// # CONSTANTS #
// #############
#define RESULTDIR "Results" // Directory containing the results


// #########################
// # PixelAlive test suite #
// #########################
class PixelAlive : public Tool
{
 public:
  PixelAlive (std::string fileRes,
              std::string fileReg,
              size_t rowStart,
              size_t rowStop,
              size_t colStart,
              size_t colStop,
              size_t nEvents,
              size_t nEvtsBurst,
              size_t nTRIGxEvent,
              bool   inject,
              bool   doFast = false,
              float  thresholdOccupancy = 0);

  void Start (int currentRun) override;
  void Stop  ()               override;

  void run                                       ();
  void draw                                      (bool display, bool save, bool runLocal = true);
  std::shared_ptr<DetectorDataContainer> analyze ();
  size_t getNumberIterations                     ()
  {
    return RD53ChannelGroupHandler::getNumberOfGroups(inject == true ? (doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups) : RD53GroupType::AllPixels) *
      nEvents/nEvtsBurst;
  }

 private:
  std::string fileRes;
  std::string fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t nTRIGxEvent;
  size_t nEvtsBurst;
  bool   inject;
  bool   doFast;
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
  PixelAliveHistograms histos;
};

#endif
