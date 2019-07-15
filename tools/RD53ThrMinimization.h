/*!
  \file                  RD53ThrMinimization.h
  \brief                 Implementaion of threshold minimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53ThrMinimization_H
#define RD53ThrMinimization_H

#include "../Utils/EmptyContainer.h"
#include "RD53PixelAlive.h"


// #####################################
// # Threshold minimization test suite #
// #####################################
class ThrMinimization : public PixelAlive
{
 public:
  ThrMinimization  (const char* fileRes, const char* fileReg, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, float targetOccupancy, size_t ThrStart = 0, size_t ThrStop = 0);
  ~ThrMinimization ();

  void Run     ();
  void Draw    (bool display, bool save);
  void Analyze ();

 private:
  const char* fileRes;
  const char* fileReg;
  size_t rowStart;
  size_t rowStop;
  size_t colStart;
  size_t colStop;
  size_t nPixels2Inj;
  size_t nEvents;
  size_t nEvtsBurst;
  size_t ThrStart;
  size_t ThrStop;
  float  targetOccupancy;

  DetectorDataContainer theThrContainer;

  void InitHisto       ();
  void FillHisto       ();
  void Display         ();
  void Save            ();
  void bitWiseScan     (const std::string& dacName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue);
  void ChipErrorReport ();


  // ########
  // # ROOT #
  // ########
  TFile* theFile;
  std::vector<TCanvas*> theCanvasThr;
  std::vector<TH1F*>    theThr;
};

#endif
