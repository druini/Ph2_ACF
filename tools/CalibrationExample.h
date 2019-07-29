/*!
*
* \file CalibrationExample.h
* \brief Calibration example -> use it as a templare
* \author Fabio Ravera
* \date 25 / 07 / 19
*
* \Support : fabio.ravera@cern.ch
*
*/

#ifndef CalibrationExample_h__
#define CalibrationExample_h__

#include "Tool.h"
#ifdef __USE_ROOT__
  //Calibration is not running on the SoC: I need to instantiate the DQM histrgrammer here
  #include "../DQMUtils/DQMHistogramCalibrationExample.h"
#endif


#include <map>

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

class DetectorContainer;

class CalibrationExample : public Tool
{

  public:
    CalibrationExample();
    ~CalibrationExample();

    void Initialise ();
    void runCalibrationExample ();
    void writeObjects();

  private:
    uint32_t fEventsPerPoint;

    #ifdef __USE_ROOT__
      //Calibration is not running on the SoC: Histogrammer is handeld by the calibration itself
      DQMHistogramCalibrationExample fDQMHistogramCalibrationExample;
    #endif
};



#endif
