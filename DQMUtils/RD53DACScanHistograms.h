/*!
  \file                  RD53DACScanHistograms.h
  \brief                 Header file of DACScan histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  09/04/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53DACScanHistograms_H
#define RD53DACScanHistograms_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TFitResult.h"
#include "TApplication.h"
#include "TGraph.h"
#include <TStyle.h>
#include <time.h>

#define LOGNAME_FORMAT "%Y%m%d_%H%M%S"
#define LOGNAME_SIZE 50

class DACScanHistograms
{
  public:
    void fillDAC(const double (&fitStart)[9], const double (&fitEnd)[9], const double (&VMUXvolt)[9][5000], const double (&DACcode)[9][5000], const std::string* writeVar);

  private:
    DetectorDataContainer DetectorData;
};

#endif
