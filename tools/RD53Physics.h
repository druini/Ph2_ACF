/*!
  \file                  RD53Physics.h
  \brief                 Implementaion of Physics data taking
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53Physics_H
#define RD53Physics_H

#include "Tool.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/GenericDataArray.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/RD53SharedConstants.h"
#include "../HWInterface/RD53FWInterface.h"

#include <thread>

#ifdef __USE_ROOT__
#include "TApplication.h"
#include "../DQMUtils/RD53PhysicsHistograms.h"
#endif


// #############
// # CONSTANTS #
// #############
#define RESULTDIR "Results" // Directory containing the results


// #######################
// # Physics data taking #
// #######################
class Physics : public Tool
{
 public:
  void Start (int currentRun)  override;
  void Stop  ()                override;
  void ConfigureCalibration () override;

  void sendData          (BoardContainer* const& cBoard);
  void initialize        (const std::string fileRes_, const std::string fileReg_);
  void run               ();
  void draw              ();
  void analyze           (bool doReadBinary = false);
  void fillDataContainer (BoardContainer* const& cBoard);


 private:
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;

  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  DetectorDataContainer theOccContainer;
  DetectorDataContainer theBCIDContainer;
  DetectorDataContainer theTrgIDContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
#ifdef __USE_ROOT__
  PhysicsHistograms histos;
  TApplication* myApp;
#endif


 protected:
  std::string fileRes;
  std::string fileReg;
  bool doUpdateChip;
  bool doDisplay;
  bool saveBinaryData;
  bool doLocal;
  std::atomic<bool> keepRunning;
  std::thread thrRun;
};

#endif
