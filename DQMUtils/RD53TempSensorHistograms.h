/*!
  \file                  RD53TempSensorHistograms.h
  \brief                 Header file of TempSensor histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  30/04/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53TempSensorHistograms_H
#define RD53TempSensorHistograms_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TFitResult.h"
#include "TGraph.h"
#include <TStyle.h>
#include <fstream>

#include "TF1.h"
#include "TH1.h"
#include "TMultiGraph.h"
#include <stdio.h>
#include <time.h>

#define LOGNAME_FORMAT "%Y%m%d_%H%M%S"
#define LOGNAME_SIZE 50

class TempSensorHistograms
{
  public:
    void fillTC( const double (&idealityFactor)[4], const double (&calibNTCtemp)[4][2], const double (&calibSenstemp)[4][2], const double (&power)[2] );
	
  private:
    DetectorDataContainer DetectorData;
};

#endif
