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
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../DQMUtils/RD53ThrEqualizationHistograms.h"
#include "Tool.h"

#include "TApplication.h"


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
  ThrEqualization  (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nEvents, size_t nEvtsBurst);

  void run  (std::shared_ptr<DetectorDataContainer> newVCal = nullptr);
  void draw (bool display, bool save);

 private:
  const char* fileRes;
  const char* fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t nEvtsBurst;  
  
  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  DetectorDataContainer theOccContainer;
  DetectorDataContainer theTDACcontainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void bitWiseScan     (const std::string& dacName, uint32_t nEvents, const float& target, uint32_t nEvtsBurst);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  RD53ThrEqualizationHistograms histos;
};

#endif
