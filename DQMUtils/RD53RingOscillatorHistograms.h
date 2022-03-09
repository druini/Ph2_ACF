/*!
  \file                  RD53RingOscillatorHistograms.h
  \brief                 Header file of RingOscillator histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  19/04/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53RingOscillatorHistograms_H
#define RD53RingOscillatorHistograms_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TFitResult.h"
#include "TGraph.h"
#include <TStyle.h>
#include <fstream>

#include "TGraph.h"
#include "TH1.h"
#include "TMultiGraph.h"
#include <time.h>

#define LOGNAME_FORMAT "%d%m%y_%H%M%S"
#define LOGNAME_SIZE 50

class RingOscillatorHistograms
{
  static constexpr const char* oscNames[] = {"CKND0", "CKND4", "INV0", "INV4", "NAND0", "NAND4", "NOR0", "NOR4"};
  public:
    void fillRO(const double (&trimOscCounts)[42][16], const double (&trimOscFrequency)[42][16], const double (&trimVoltage)[16], int nPoints = 16, double fitResults[42][2] = nullptr);

  private:
    DetectorDataContainer DetectorData;
	//static constexpr const char oscNames[] = {"CKND0", "CKND4", "INV0", "INV4", "NAND0", "NAND4", "NOR0", "NOR4", 
	//										"CKND0 L","CKND0 R", "CKND4 L", "CKND4 R", "INV0 L","INV0 R", "INV4 L", "INV4 R", "NAND0 L","NAND0 R", "NAND4 L","NAND4 R", "NOR0 L","NOR0 R", "NOR4 L","NOR4 R",
	//										"SCAN DFF 0", "SCAN DFF 0", "DFF 0", "DFF 0", "NEG EDGE DFF 1", "NEG EDGE DFF 1",
	//										"LVT INV 0", "LVT INV 4","LVT 4-IN NAND0", "LVT 4-IN NAND 4",
	//										"0","1","2","3","4","5","6","7" };
};

#endif
