/*!
*
* \file RD53CalibrationExample.h
* \brief Calibration example -> use it as a template
* \author Fabio Ravera
* \date 25 / 07 / 19
*
* \Support : fabio.ravera@cern.ch
*
*/

#ifndef RD53CalibrationExample_h__
#define RD53CalibrationExample_h__

#include "Tool.h"
#include <map>
#ifdef __USE_ROOT__
  //Calibration is not running on the SoC: I need to instantiate the DQM histrgrammer here
  #include "../DQMUtils/RD53DQMHistogramCalibrationExample.h"
#endif

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

class RD53CalibrationExample : public Tool
{

  public:
    RD53CalibrationExample();
    ~RD53CalibrationExample();

    void Initialise               (void);
    void runRD53CalibrationExample(void);
    void writeObjects             (void);

    //State machine 
    void Start                (int currentRun) override;
    void Stop                 (void) override;
  
  private:
    size_t fRowStart;
    size_t fRowStop;
    size_t fColStart;
    size_t fColStop;
    uint32_t fEventsPerPoint;

    #ifdef __USE_ROOT__
      //Calibration is not running on the SoC: Histogrammer is handeld by the calibration itself
      RD53DQMHistogramCalibrationExample fRD53DQMHistogramCalibrationExample;
    #endif
};

#endif
