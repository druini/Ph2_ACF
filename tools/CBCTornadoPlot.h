/*!
*
* \file CBCTornadoPlot.h
* \brief Calibration example -> use it as a template
* \author Fabio Ravera
* \date 25 / 07 / 19
*
* \Support : fabio.ravera@cern.ch
*
*/

#ifndef CBCTornadoPlot_h__
#define CBCTornadoPlot_h__

#include "Tool.h"
#include <map>
#ifdef __USE_ROOT__
  //Calibration is not running on the SoC: I need to instantiate the DQM histrgrammer here
  #include "../DQMUtils/CBCHistogramTornadoPlot.h"
#endif

class CBCTornadoPlot : public Tool
{

  public:
    CBCTornadoPlot();
    ~CBCTornadoPlot();

    void Initialise           (void);
    void runCBCTornadoPlot(void);
    void writeObjects         (void);

    //State machine 
    void Start                (int currentRun) override;
    void Stop                 (void) override;
  
  private:
    uint32_t fEventsPerPoint;
    uint16_t fInitialVcth   ;
    uint16_t fFinalVcth     ;
    uint16_t fVCthStep      ;
    uint8_t  fInitialDelay  ;
    uint8_t  fFinalDelay    ;
    uint8_t  fDelayStep     ;
    uint8_t  fPulseAmplitude;

    #ifdef __USE_ROOT__
      //Calibration is not running on the SoC: Histogrammer is handeld by the calibration itself
      CBCHistogramTornadoPlot fCBCHistogramTornadoPlot;
    #endif
};

#endif
