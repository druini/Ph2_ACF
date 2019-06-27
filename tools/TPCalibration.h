#ifndef TPCalibration_h__
#define TPCalibration_h__

#include "Tool.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1F.h"

#include "PedeNoise.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


class TPCalibration : public PedeNoise
{
private: //attributes
  int fStartAmp;
  int fEndAmp;
  int fStepsize;
  int fTPCount;

public: //methods
  TPCalibration();
  ~TPCalibration();


  void Init(int pStartAmp, int pEndAmp, int pStepsize);
  void RunCalibration();
  void SaveResults();
  float ConvertAmpToElectrons(float pTPAmp, bool pOffset);

private: //methods
  void FillHistograms(int pTPAmp);
  void FitCorrelations();
};

#endif
