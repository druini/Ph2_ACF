/*!
*
* \file PedeNoise.h
* \brief Calibration class, calibration of the hardware
* \author Georg AUZINGER
* \date 12 / 11 / 15
*
* \Support : georg.auzinger@cern.ch
*
*/

#ifndef PedeNoise_h__
#define PedeNoise_h__

#include "Tool.h"
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"
#ifdef __USE_ROOT__
  #include "../DQMUtils/DQMHistogramPedeNoise.h"
#endif


#include <map>

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

class DetectorContainer;

class PedeNoise : public Tool
{

  public:
    PedeNoise();
    ~PedeNoise();

    void Initialise (bool pAllChan = false, bool pDisableStubLogic = true);
    void measureNoise (uint8_t pTPAmplitude = 0); //method based on the one below that actually analyzes the scurves and extracts the noise
    void sweepSCurves (uint8_t pTPAmplitude); // actual methods to measure SCurves
    void Validate (uint32_t pNoiseStripThreshold = 1, uint32_t pMultiple = 100);
    void writeObjects();

    void Start(int currentRun) override;
    void Stop() override;
    void ConfigureCalibration() override;
    void Pause() override;
    void Resume() override;

  private:

    DetectorDataContainer fThresholdAndNoiseContainer;
    //to hold the original register values
    DetectorDataContainer fStubLogicValue;
    DetectorDataContainer fHIPCountValue;

    // Settings
    bool fHoleMode;
    bool fPlotSCurves;
    bool fFitSCurves;
    uint8_t fTestPulseAmplitude;
    uint32_t fEventsPerPoint;
    bool fDisableStubLogic;

  private:
    void measureSCurves (uint16_t pStartValue = 0 );
    void extractPedeNoise ();
    
    // for validation
    void setThresholdtoNSigma (BoardContainer* board, uint32_t pNSigma);
    
    //helpers for SCurve measurement
    uint16_t findPedestal (bool forceAllChannels = false);

    std::map<uint16_t, DetectorDataContainer*> fSCurveOccupancyMap;

    #ifdef __USE_ROOT__
      DQMHistogramPedeNoise fDQMHistogramPedeNoise;
    #endif
};



#endif
