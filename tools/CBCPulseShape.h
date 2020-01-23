/*!
*
* \file CBCPulseShape.h
* \brief Calibration example -> use it as a template
* \author Fabio Ravera
* \date 25 / 07 / 19
*
* \Support : fabio.ravera@cern.ch
*
*/

#ifndef CBCPulseShape_h__
#define CBCPulseShape_h__

#include "Tool.h"
#include <map>
#ifdef __USE_ROOT__
  //Calibration is not running on the SoC: I need to instantiate the DQM histrgrammer here
  #include "../DQMUtils/CBCHistogramPulseShape.h"
#endif

class CBCPulseShape : public Tool
{

  public:
    CBCPulseShape();
    ~CBCPulseShape();

    void Initialise           (void);
    void runCBCPulseShape(void);
    void writeObjects         (void);

    //State machine 
    void Start                (int currentRun) override;
    void Stop                 (void) override;
  
  private:
    uint32_t fEventsPerPoint {0} ;
    uint16_t fInitialVcth    {0} ;
    uint16_t fFinalVcth      {0} ;
    uint16_t fVCthStep       {0} ;
    uint16_t fInitialLatency {0} ;
    uint16_t  fInitialDelay   {0} ;
    uint16_t  fFinalDelay     {0} ;
    uint16_t  fDelayStep      {0} ;
    uint16_t  fPulseAmplitude {0} ;

    #ifdef __USE_ROOT__
      //Calibration is not running on the SoC: Histogrammer is handeld by the calibration itself
      CBCHistogramPulseShape fCBCHistogramPulseShape;
    #endif
};

#endif