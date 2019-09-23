/*!
*
* \file CalibrationExample.h
* \brief Calibration example -> use it as a template
* \author Fabio Ravera
* \date 25 / 07 / 19
*
* \Support : fabio.ravera@cern.ch
*
*/

#ifndef CalibrationExample_h__
#define CalibrationExample_h__

#include "Tool.h"
#include <map>
#ifdef __USE_ROOT__
  //Calibration is not running on the SoC: I need to instantiate the DQM histrgrammer here
  #include "../DQMUtils/DQMHistogramCalibrationExample.h"
#endif

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

class CalibrationExample : public Tool
{

  public:
    CalibrationExample();
    ~CalibrationExample();

    void runCalibrationExample(void);
    void writeObjects         (void);

    //State machine 
    void Initialise           (void);
    void Start                (int currentRun) override;
    void Stop                 (void) override;
  
  private:
    uint32_t fEventsPerPoint;

    #ifdef __USE_ROOT__
      //Calibration is not running on the SoC: Histogrammer is handeld by the calibration itself
      DQMHistogramCalibrationExample fDQMHistogramCalibrationExample;
    #endif
};

#endif
