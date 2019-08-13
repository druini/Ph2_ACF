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
#include "../DQMUtils/RD53GainHistograms.h"
#include "Tool.h"

#include "TApplication.h"


// ##########################
// # Gain measurement suite #
// ##########################
class Gain : public Tool
{
 public:
  Gain  (const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps, size_t offset);
  ~Gain ()
    {
      for (auto container : detectorContainerVector) delete container;
    }
  
  void run                                       ();
  void draw                                      (bool display, bool save);
  std::shared_ptr<DetectorDataContainer> analyze ();

 private:
  const char* fileRes;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t startValue;
  size_t stopValue;
  size_t nSteps;
  size_t offset;

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
  RD53GainHistograms histos;
};

#endif
