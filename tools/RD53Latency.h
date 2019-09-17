/*!
  \file                  RD53Latency.h
  \brief                 Implementaion of Latency scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53Latency_H
#define RD53Latency_H

#include "../DQMUtils/RD53LatencyHistograms.h"
#include "RD53PixelAlive.h"


// #############
// # CONSTANTS #
// #############
#define RESULTDIR "Results" // Directory containing the results


// ######################
// # Latency test suite #
// ######################
class Latency : public PixelAlive
{
 public:
  Latency (std::string fileRes,
           std::string fileReg,
           size_t rowStart,
           size_t rowStop,
           size_t colStart,
           size_t colStop,
           size_t startValue,
           size_t stopValue,
           size_t nEvents);

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

  std::vector<uint16_t> dacList;

  DetectorDataContainer theContainer;
  DetectorDataContainer theLatencyContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void scanDac         (const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  LatencyHistograms histos;
};

#endif
