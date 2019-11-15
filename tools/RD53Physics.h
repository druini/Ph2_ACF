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
#include "../HWInterface/RD53FWInterface.h"

#include <thread>


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
  void writeObjects         () {}; // @TMP@

  void sendData          (BoardContainer* const& cBoard);
  void run               ();
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

  void chipErrorReport ();


 protected:
  bool keepRunning;
  std::thread thrRun;
};

#endif
