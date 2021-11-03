/*!
  \file                  RD53BRingOscillator.h
  \brief                 Implementaion of RingOscillator
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  21/09/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53BRingOscillator_H
#define RD53BRingOscillator_H

#include "../tools/Tool.h"
#include "ITchipTestingInterface.h"

#include <chrono>
#include <cmath>

#include "../System/SystemController.h"
#include "../Utils/argvparser.h"


#include "BeBoardFWInterface.h"
#include "RD53FWInterface.h"
#include "RD53BInterface.h"
#include "ReadoutChipInterface.h"

#ifdef __USE_ROOT__
#include "../DQMUtils/RD53RingOscillatorHistograms.h"
#include "TApplication.h"
#endif

#include <string>

// #########################
// # RingOscillator test suite #
// #########################
class BRingOscillator : public Tool
{
  public:
    void   run();
    void   draw();
	
	#ifdef __USE_ROOT__
	RingOscillatorHistograms* histos;
	#endif
	
  private:	
	double gloPulse[128];
	double oscCounts[34][128];
	double oscFrequency[34][128];
	double trimOscCounts[34][16];
	double trimOscFrequency[34][16];
	double trimVoltage[16];
};

#endif
