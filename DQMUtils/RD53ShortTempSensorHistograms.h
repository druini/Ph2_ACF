/*!
  \file                  RD53ShortTempSensorHistograms.h
  \brief                 Header file of ShortTempSensor histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  30/07/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53ShortTempSensorHistograms_H
#define RD53ShortTempSensorHistograms_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TGraph.h"
#include "TFitResult.h"
#include <TStyle.h>
#include <fstream>
#include "TObject.h"

class ShortTempSensorHistograms
{
  public:
    void fillSTS(const double (&temperature)[5], int run_counter = 0 );
	
  private:
    DetectorDataContainer DetectorData;
};

#endif
