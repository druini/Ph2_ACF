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

#include "Tool.h"
#include "RD53SCurve.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "../Utils/RD53SharedConstants.h"

#ifdef __USE_ROOT__
#include "TApplication.h"
#include "../DQMUtils/RD53ThrEqualizationHistograms.h"
#endif


// #############
// # CONSTANTS #
// #############
#define TARGETEFF 0.50      // Target efficiency for optimization algorithm
#define RESULTDIR "Results" // Directory containing the results


// #####################################
// # Threshold equalization test suite #
// #####################################
class ThrEqualization : public Tool
{
 public:
  void Start (int currentRun)  override;
  void Stop  ()                override;
  void ConfigureCalibration () override;

  void   sendData            ();
  void   initialize          (const std::string fileRes_, const std::string fileReg_);
  void   run                 ();
  void   draw                ();
  size_t getNumberIterations ()
  {
    uint16_t nBitTDAC       = 4;
    uint16_t moreIterations = 2;
    return RD53ChannelGroupHandler::getNumberOfGroups(doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups, nHITxCol)*(nBitTDAC + moreIterations) *
      nEvents/nEvtsBurst + sc.getNumberIterations();
  }


 private:
  SCurve sc;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nEvents;
  size_t nEvtsBurst;
  size_t nHITxCol;
  bool   doFast;

  std::shared_ptr<RD53ChannelGroupHandler> theChnGroupHandler;
  DetectorDataContainer theOccContainer;
  DetectorDataContainer theTDACcontainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void bitWiseScan     (const std::string& regName, uint32_t nEvents, const float& target, uint32_t nEvtsBurst);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
#ifdef __USE_ROOT__
  ThrEqualizationHistograms histos;
#endif


 protected:
  std::string fileRes;
  std::string fileReg;
  bool doUpdateChip;
  bool doDisplay;
  bool saveBinaryData;
};

#endif
