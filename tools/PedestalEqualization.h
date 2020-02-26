/*!
*
* \file PedestalEqualization.h
* \brief PedestalEqualization class, PedestalEqualization of the hardware
* \author Georg AUZINGER
* \date 13 / 11 / 15
*
* \Support : georg.auzinger@cern.ch
*
*/

#ifndef PedestalEqualization_h__
#define PedestalEqualization_h__

#include "Tool.h" 

#include <map>

#ifdef __USE_ROOT__
  #include "../DQMUtils/DQMHistogramPedestalEqualization.h"
#endif 

class PedestalEqualization : public Tool
{

  public:
    PedestalEqualization();
    ~PedestalEqualization();

    void Initialise ( bool pAllChan = false, bool pDisableStubLogic = true );
    void FindVplus();
    // offsets are found by taking pMultiple*fEvents triggers
    void FindOffsets();
    void writeObjects();

    void Start(int currentRun) override;
    void Stop() override;
    void ConfigureCalibration() override;
    void Pause() override;
    void Resume() override;

  private:

    // Settings
    bool     fTestPulse          {false};
    uint8_t  fTestPulseAmplitude {    0};
    uint32_t fEventsPerPoint     {   10};
    uint16_t fTargetVcth         {  0x0};
    uint8_t  fTargetOffset       { 0x80};
    bool     fCheckLoop          { true};
    bool     fAllChan            { true};
    bool     fDisableStubLogic   { true};

    //to hold the original register values
    DetectorDataContainer fStubLogicCointainer;
    DetectorDataContainer fHIPCountCointainer;

    #ifdef __USE_ROOT__
      DQMHistogramPedestalEqualization fDQMHistogramPedestalEqualization;
    #endif


};


#endif
