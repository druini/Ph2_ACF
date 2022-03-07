/*!
  \file                  RD53BCapMeasureHistograms.h
  \brief                 Header file of CapMeasure histograms
*/

#ifndef RD53BCapMeasureHistograms_H
#define RD53BCapMeasureHistograms_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TGraph.h"
#include <TStyle.h>
#include "TApplication.h"

class RD53BCapMeasureHistograms
{
  public:
    void fillCap( const double (&VMain)[16], const double (&VDDAMain)[16], const double (&VPara)[16], const double (&VDDAPara)[16], const double (&VTrim)[16]);
    double calcCapacitance(double vmeas, double vmeaspar, double vddameas);
	
  private:
    DetectorDataContainer DetectorData;
};

#endif
