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

#include "Tool.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/RD53SharedConstants.h"

#ifdef __USE_ROOT__
#include "TApplication.h"
#include "../DQMUtils/RD53SCurveHistograms.h"
#endif


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
  ~SCurve () { for (auto container : detectorContainerVector) delete container; }

  void Start (int currentRun)  override;
  void Stop  ()                override;
  void ConfigureCalibration () override;

  void sendData                                  ();
  void initialize                                (const std::string fileRes_, const std::string fileReg_);
  void run                                       ();
  void draw                                      ();
  std::shared_ptr<DetectorDataContainer> analyze ();
  size_t getNumberIterations                     ()
  {
    return RD53ChannelGroupHandler::getNumberOfGroups(doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups, nHITxCol)*nSteps;
  }


 private:
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t startValue;
  size_t stopValue;
  size_t nSteps;
  size_t offset;
  size_t nHITxCol;
  bool   doFast;

  std::vector<uint16_t> dacList;

  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  std::vector<DetectorDataContainer*>      detectorContainerVector;
  std::shared_ptr<DetectorDataContainer>   theThresholdAndNoiseContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void computeStats    (const std::vector<float>& measurements, int offset, float& nHits, float& mean, float& rms);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
#ifdef __USE_ROOT__
  SCurveHistograms histos;
#endif


 protected:
  std::string fileRes;
  std::string fileReg;
  bool doUpdateChip;
  bool doDisplay;
  bool saveBinaryData;
};

#endif
