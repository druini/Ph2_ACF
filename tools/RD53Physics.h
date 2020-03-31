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
#include "../Utils/RD53Shared.h"
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
  using evtConvType = std::function<void(const std::vector<Ph2_HwInterface::RD53FWInterface::Event>&)>;

 public:
  Physics () { Physics::setGenericEvtConverter(RD53dummyEvtConverter()); }

  void Start (int currentRun)  override;
  void Stop  ()                override;
  void ConfigureCalibration () override;

  void sendData               (const BoardContainer* cBoard);
  void localConfigure         (const std::string fileRes_, int currentRun);
  void initializeFiles        (const std::string fileRes_, int currentRun);
  void run                    ();
  void draw                   ();
  void analyze                (bool doReadBinary = false);
  void fillDataContainer      (BoardContainer* const& cBoard);
  void monitor                ();

  void setGenericEvtConverter (evtConvType arg) { genericEvtConverter = std::move(arg); }


#ifdef __USE_ROOT__
  PhysicsHistograms* histos;
  TApplication* myApp;
#endif


 private:
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nTRIGxEvent;

  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  DetectorDataContainer theOccContainer;
  DetectorDataContainer theBCIDContainer;
  DetectorDataContainer theTrgIDContainer;

  void fillHisto         ();
  void chipErrorReport   ();
  void saveChipRegisters (int currentRun);


 protected:
  struct RD53dummyEvtConverter
  {
    void operator() (const std::vector<Ph2_HwInterface::RD53FWInterface::Event>& RD53EvtList) {};
  };

  std::string fileRes;
  int    theCurrentRun;
  size_t numberOfEventsPerRun;
  bool   doUpdateChip;
  bool   doDisplay;
  bool   saveBinaryData;
  std::atomic<bool> keepRunning;
  std::thread thrRun;
  std::thread thrMonitor;
  evtConvType genericEvtConverter;
};

#endif
