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
#include "../Utils/ContainerRecycleBin.h"
#include "../Utils/CommonVisitors.h"
#ifdef __USE_ROOT__
  #include "../DQMUtils/DQMHistogramPedeNoise.h"
#endif


#include <map>

using namespace Ph2_System;

class DetectorContainer;
class Occupancy;

class PedeNoise : public Tool
{

  public:
    PedeNoise();
    ~PedeNoise();

    void Initialise (bool pAllChan = false, bool pDisableStubLogic = true);
    void measureNoise (); //method based on the one below that actually analyzes the scurves and extracts the noise
    void sweepSCurves (); // actual methods to measure SCurves
    void Validate (uint32_t pNoiseStripThreshold = 1, uint32_t pMultiple = 100);
    void writeObjects();


    void Start(int currentRun) override;
    void Stop() override;
    void ConfigureCalibration() override;
    void Pause() override;
    void Resume() override;

  protected:
    uint16_t findPedestal (bool forceAllChannels = false);
    void measureSCurves (uint16_t pStartValue = 0 );
    void extractPedeNoise ();
    void disableStubLogic();
    void reloadStubLogic();
    void cleanContainerMap();
    void initializeRecycleBin() {fRecycleBin.setDetectorContainer(fDetectorContainer);}

    uint8_t  fPulseAmplitude     {    0};
    uint32_t fEventsPerPoint     {    0};
    DetectorDataContainer fThresholdAndNoiseContainer;

  private:

    //to hold the original register values
    DetectorDataContainer fStubLogicValue;
    DetectorDataContainer fHIPCountValue;

    // Settings
    bool     fPlotSCurves        {false};
    bool     fFitSCurves         {false};
    bool     fDisableStubLogic   { true};

    void producePedeNoisePlots();
    
    // for validation
    void setThresholdtoNSigma (BoardContainer* board, uint32_t pNSigma);
    
    //helpers for SCurve measurement

    std::map<uint16_t, DetectorDataContainer*> fSCurveOccupancyMap;
    ContainerRecycleBin<Occupancy> fRecycleBin;

    #ifdef __USE_ROOT__
      DQMHistogramPedeNoise fDQMHistogramPedeNoise;
    #endif
};



#endif
