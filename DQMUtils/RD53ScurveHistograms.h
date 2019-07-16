/*!
  \file                  RD53ScurveHistograms.h
  \brief                 Header file of SCurve calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53ScurveHistograms_H
#define RD53ScurveHistograms_H

#include "../Utils/GenericDataVector.h"
#include "../Utils/Occupancy.h"
#include "../Utils/ThresholdAndNoise.h"
#include "DQMHistogramBase.h"

#include <TH1F.h>
#include <TH2F.h>


class RD53ScurveHistograms : public DQMHistogramBase
{
public:
    RD53ScurveHistograms(int nEvents, int startValue, int stopValue, int nSteps)
        : nEvents    (nEvents)
        , startValue (startValue)
        , stopValue  (stopValue)
        , nSteps     (nSteps)
    {}

    void book               (TFile* theOutputFile, const DetectorContainer& theDetectorStructure) override;
    void fillOccupancy      (const DetectorDataContainer& data, int VCAL_HIGH);
    void fillThresholdNoise (const DetectorDataContainer& data);
    void process            ()                                                                    override;
    void fill               (std::vector<char>& dataBuffer)                                       override {};
    void reset              (void)                                                                override {};

private:
    DetectorDataContainer Occupancy2D;
    DetectorDataContainer Threshold1D;
    DetectorDataContainer Noise1D;
    DetectorDataContainer Threshold2D;
    DetectorDataContainer Noise2D;
    
    int nEvents;
    int nSteps;
    int startValue;
    int stopValue;
};

#endif
