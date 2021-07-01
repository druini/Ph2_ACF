/*!
  \file                  RD53ADCScanHistograms.h
  \brief                 Header file of ADCScan histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  16/04/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53ADCScanHistograms_H
#define RD53ADCScanHistograms_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TGraph.h"
#include <TStyle.h>
#include "TApplication.h"

class ADCScanHistograms : public DQMHistogramBase
{
  public:
    void fillADC(DetectorContainer& DataContainer, double* fitStart, double* fitEnd, double** VMUXvolt, double** ADCcode, double** DNLcode, double** INLcode, std::string* writeVar);
	
  private:
    DetectorDataContainer DetectorData;
};

#endif
