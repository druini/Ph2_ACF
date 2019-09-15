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

#include "../DQMUtils/RD53InjectionDelayHistograms.h"
#include "RD53PixelAlive.h"


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
  InjectionDelay (std::string fileRes,
                  std::string fileReg,
                  size_t rowStart,
                  size_t rowStop,
                  size_t colStart,
                  size_t colStop,
                  size_t startValue,
                  size_t stopValue,
                  size_t nEvents,
                  bool   doFast = true);

  void   run                 ();
  void   draw                (bool display, bool save);
  void   analyze             ();
  size_t getNumberIterations () { return PixelAlive::getNumberIterations()*(stopValue - startValue + 1); }

 private:
  std::string fileRes;
  std::string fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t startValue;
  size_t stopValue;
  size_t nEvents;
  bool   doFast;

  std::vector<uint16_t> dacList;

  DetectorDataContainer theContainer;
  DetectorDataContainer theInjectionDelayContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void scanDac         (const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  RD53InjectionDelayHistograms histos;
};

#endif
