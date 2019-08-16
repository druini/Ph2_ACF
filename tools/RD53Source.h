/*!
  \file                  RD53Source.h
  \brief                 Implementaion of Source scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53Source_H
#define RD53Source_H

#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../DQMUtils/RD53SourceHistograms.h"
#include "../Utils/Containable.h"
#include "Tool.h"

#include "TApplication.h"


// #####################
// # Source test suite #
// #####################
class Source : public Tool
{
 public:
  Source (const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t startValue, size_t stopValue, size_t nSteps, size_t duration);
  ~Source() {
      for (auto container : detectorContainerVector)
        delete container;
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
  size_t startValue;
  size_t stopValue;
  size_t nSteps;
  size_t offset;
  size_t duration; // in ms

  std::vector<uint16_t> dacList;

  std::vector<std::vector<double>> hit_rates;
  
  std::vector<DetectorDataContainer*>      detectorContainerVector;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  RD53SourceHistograms histos;
};

#endif
