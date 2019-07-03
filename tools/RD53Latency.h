/*!
  \file                  RD53Latency.h
  \brief                 Implementaion of Latency scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53Latency_h_
#define _RD53Latency_h_

#include "../Utils/Container.h"
#include "../Utils/GenericDataVector.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "Tool.h"

#include "TApplication.h"
#include "TH1F.h"


using namespace Ph2_System;

// ######################
// # Latency test suite #
// ######################
class Latency : public Tool
{
 public:
  Latency(const char* fileRes, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t startValue, size_t stopValue, size_t nEvents);
  ~Latency();

  void Run     ();
  void Draw    (bool display, bool save);
  void Analyze ();

 private:
  const char* fileRes;
  size_t rowStart;
  size_t rowEnd;
  size_t colStart;
  size_t colEnd;
  size_t startValue;
  size_t stopValue;
  size_t nEvents;

  DetectorDataContainer theContainer;

  void InitHisto       ();
  void FillHisto       ();
  void Display         ();
  void Save            ();
  void ChipErrorReport ();


  // ########
  // # ROOT #
  // ########
  TFile* theFile;
  std::vector<TCanvas*> theCanvasLat;
  std::vector<TH1F*>    theLat;
};

#endif
