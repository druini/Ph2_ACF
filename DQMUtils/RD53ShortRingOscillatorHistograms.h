/*!
  \file                  RD53ShortRingOscillatorHistograms.h
  \brief                 Header file of ShortRingOscillator histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  27/07/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53ShortRingOscillatorHistograms_H
#define RD53ShortRingOscillatorHistograms_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TGraph.h"
#include "TFitResult.h"
#include <TStyle.h>
#include <fstream>
#include "TObject.h"

class ShortRingOscillatorHistograms : public DQMHistogramBase
{
  public:
    void fillSRO( double oscCounts[42], int run_counter = 0 );
	
  private:
    DetectorDataContainer DetectorData;
	//static constexpr const char oscNames[] = {"CKND0", "CKND4", "INV0", "INV4", "NAND0", "NAND4", "NOR0", "NOR4", 
	//										"CKND0 L","CKND0 R", "CKND4 L", "CKND4 R", "INV0 L","INV0 R", "INV4 L", "INV4 R", "NAND0 L","NAND0 R", "NAND4 L","NAND4 R", "NOR0 L","NOR0 R", "NOR4 L","NOR4 R",
	//										"SCAN DFF 0", "SCAN DFF 0", "DFF 0", "DFF 0", "NEG EDGE DFF 1", "NEG EDGE DFF 1",
	//										"LVT INV 0", "LVT INV 4","LVT 4-IN NAND0", "LVT 4-IN NAND 4",
	//										"0","1","2","3","4","5","6","7" };
};

#endif
