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

#include "TFitResult.h"
#include "TApplication.h"
#include "TGraph.h"
#include <TStyle.h>

class ADCScanHistograms
{
  public:
    void fillADC(const double (&fitStart)[9], const double (&fitEnd)[9], const double (&VMUXvolt)[9][5000], const double (&ADCcode)[9][5000], const std::string* writeVar);

  private:
    DetectorDataContainer DetectorData;
};

#endif