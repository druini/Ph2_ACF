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

#include "../Utils/Container.h"
#include "../Utils/GenericDataVector.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/RD53ChannelGroupHandler.h"
#include "Tool.h"

#include "TApplication.h"
#include "TH1F.h"


// ######################
// # Latency test suite #
// ######################
class Latency : public Tool
{
 public:
  Latency(const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t startValue, size_t stopValue, size_t nEvents);
  ~Latency();

  void run     ();
  void draw    (bool display, bool save);
  void analyze ();

 private:
  const char* fileRes;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t startValue;
  size_t stopValue;
  size_t nEvents;
 
  std::vector<uint16_t> dacList;

  DetectorDataContainer theContainer;

  void initHisto       ();
  void fillHisto       ();
  void display         ();
  void save            ();
  void scanDac         (const std::string& dacName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer);
  void chipErrorReport ();


  // ########
  // # ROOT #
  // ########
  TFile* theFile;
  std::vector<TCanvas*> theCanvasLat;
  std::vector<TH1F*>    theLat;
};

#endif
