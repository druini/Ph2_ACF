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
  void Start (int currentRun)  override;
  void Stop  ()                override;
  void ConfigureCalibration () override;

  void initialize                                (const std::string fileRes_, const std::string fileReg_);
  void run                                       ();
  void draw                                      ();
  std::shared_ptr<DetectorDataContainer> analyze ();
  size_t getNumberIterations                     ()
  {
    return RD53ChannelGroupHandler::getNumberOfGroups(doInjection == true ? (doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups) : RD53GroupType::AllPixels) *
      nEvents/nEvtsBurst;
  }


 private:
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t nTRIGxEvent;
  size_t nEvtsBurst;
  bool   doInjection;
  bool   doFast;
  float  thrOccupancy;

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


 protected:
  std::string fileRes;
  std::string fileReg;
  bool doDisplay;
  bool doSave;
};

#endif
