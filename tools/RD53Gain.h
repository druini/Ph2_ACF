/*!
  \file                  RD53Gain.h
  \brief                 Implementaion of Gain scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53Gain_H
#define RD53Gain_H

#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#ifdef __USE_ROOT__
  #include "../DQMUtils/RD53GainHistograms.h"
  #include "TApplication.h"
#endif
#include "Tool.h"



// #############
// # CONSTANTS #
// #############
#define RESULTDIR "Results" // Directory containing the results
#define INTERCEPT_HALFRANGE 6 // [ToT]
#define ISDISABLED -1.0       // Encoding disabled channels
#define FITERROR   -2.0       // Encoding fit errors


// ##########################
// # Gain measurement suite #
// ##########################
class Gain : public Tool
{
 public:
  ~Gain () { for (auto container : detectorContainerVector) delete container; }

  void Start (int currentRun)  override;
  void Stop  ()                override;
  void ConfigureCalibration () override;
  void writeObjects         () {}; // @TMP@

  void sendData                                  ();
  void initialize                                (const std::string fileRes_, const std::string fileReg_);
  void run                                       ();
  void draw                                      ();
  std::shared_ptr<DetectorDataContainer> analyze ();
  size_t getNumberIterations                     ()
  {
    return RD53ChannelGroupHandler::getNumberOfGroups(doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups)*nSteps;
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
  bool   doFast;

  std::vector<uint16_t> dacList;

  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  std::vector<DetectorDataContainer*>      detectorContainerVector;
  std::shared_ptr<DetectorDataContainer>   theGainAndInterceptContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void computeStats    (std::vector<float>& x, std::vector<float>& y, std::vector<float>& e, double& gain, double& gainErr, double& intercept, double& interceptErr);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  #ifdef __USE_ROOT__
    GainHistograms histos;
  #endif


 protected:
  std::string fileRes;
  std::string fileReg;
  bool doDisplay;
  bool doSave;
};

#endif
