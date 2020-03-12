/*!
  \file                  RD53InjectionDelay.h
  \brief                 Implementaion of Injection Delay scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53InjectionDelay_H
#define RD53InjectionDelay_H

#include "RD53Latency.h"

#ifdef __USE_ROOT__
#include "../DQMUtils/RD53InjectionDelayHistograms.h"
#endif


// #############
// # CONSTANTS #
// #############
#define RESULTDIR "Results" // Directory containing the results


// ##############################
// # Injection delay test suite #
// ##############################
class InjectionDelay : public PixelAlive
{
 public:
  void Start (int currentRun)  override;
  void Stop  ()                override;
  void ConfigureCalibration () override;

  void   sendData            ();
  void   localConfigure      (const std::string fileRes_, int currentRun);
  void   initializeFiles     (const std::string fileRes_, int currentRun);
  void   run                 ();
  void   draw                (int currentRun);
  void   analyze             ();
  size_t getNumberIterations () { return PixelAlive::getNumberIterations()*(stopValue - startValue); }


#ifdef __USE_ROOT__
  InjectionDelayHistograms* histos;
#endif


 private:
  Latency la;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t startValue;
  size_t stopValue;
  size_t nEvents;

  std::vector<uint16_t> dacList;

  DetectorDataContainer theOccContainer;
  DetectorDataContainer theInjectionDelayContainer;

  void fillHisto         ();
  void scanDac           (const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer);
  void chipErrorReport   ();
  void saveChipRegisters (int currentRun);


 protected:
  std::string fileRes;
  size_t saveInjection;
  size_t maxDelay;
  bool   doUpdateChip;
  bool   doDisplay;
  bool   saveBinaryData;
};

#endif
