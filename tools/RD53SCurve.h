/*!
  \file                  RD53SCurve.h
  \brief                 Implementaion of SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53SCurve_H
#define RD53SCurve_H

#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../DQMUtils/RD53SCurveHistograms.h"
#include "Tool.h"

#include "TApplication.h"


// #############
// # CONSTANTS #
// #############
#define RESULTDIR "Results" // Directory containing the results


// #####################
// # SCurve test suite #
// #####################
class SCurve : public Tool
{
 public:
  SCurve  (std::string fileRes,
           std::string fileReg,
           size_t rowStart,
           size_t rowStop,
           size_t colStart,
           size_t colStop,
           size_t nEvents,
           size_t startValue,
           size_t stopValue,
           size_t nSteps,
           size_t offset,
           bool   doFast = false);
  ~SCurve () { for (auto container : detectorContainerVector) delete container; }

  void run                                       ();
  void draw                                      (bool display, bool save);
  std::shared_ptr<DetectorDataContainer> analyze ();
  size_t getNumberIterations                     ()
  {
    return RD53ChannelGroupHandler::getNumberOfGroups(doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups)*nSteps;
  }

 private:
  std::string fileRes;
  std::string fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t startValue;
  size_t stopValue;
  size_t nSteps;
  size_t offset;
  bool   doFast;

  std::vector<uint16_t> dacList;

  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  std::vector<DetectorDataContainer*>      detectorContainerVector;
  std::shared_ptr<DetectorDataContainer>   theThresholdAndNoiseContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void computeStats    (std::vector<float>& measurements, int offset, float& nHits, float& mean, float& rms);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  RD53SCurveHistograms histos;
};

#endif
