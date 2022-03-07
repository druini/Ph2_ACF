/*!
  \file                  RD53BCapMeasureResults.h
  \brief                 Header file of CapMeasureResults
*/

#ifndef RD53BCapMeasureResults_H
#define RD53BCapMeasureResults_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TGraph.h"
#include <TStyle.h>
#include "TApplication.h"

class RD53BCapMeasureResults
{
  public:
    void fillCapacitance( const double (&CapVoltages)[4]);
    double calcCapacitance(double vmeas, double vmeaspar, double vddameas);
	
  private:
    DetectorDataContainer DetectorData;
};

#endif
