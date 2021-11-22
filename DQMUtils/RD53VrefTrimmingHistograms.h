/*!
  \file                  RD53VrefTrimmingHistograms.h
  \brief                 Header file of VrefTrimming histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  22/11/21
  Support:               email to umberto.molinatti@cern.ch
*/

#ifndef RD53VrefTrimmingHistograms_H
#define RD53VrefTrimmingHistograms_H

#include "../Utils/ContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/GenericDataArray.h"
#include "DQMHistogramBase.h"

#include "TFitResult.h"
#include "TGraph.h"
#include "TApplication.h"
#include <TStyle.h>
#include <fstream>

#include "TH1.h"
#include "TMultiGraph.h"
#include <time.h>

#define LOGNAME_FORMAT "%Y%m%d_%H%M%S"
#define LOGNAME_SIZE 50

class VrefTrimmingHistograms
{
  public:
    void fillVT(const double (&vdddVoltage)[16], const double (&vddaVoltage)[16]);

  private:
    DetectorDataContainer DetectorData;
};

#endif
