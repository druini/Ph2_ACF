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
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/DataContainer.h"

#include <map>

#ifdef __USE_ROOT__
  #include "../DQMUtils/DQMHistogramPedestalEqualization.h"
#endif 

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


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
    bool fHoleMode;
    bool fTestPulse;
    uint8_t fTestPulseAmplitude;
    uint32_t fEventsPerPoint;
    uint16_t fTargetVcth;
    uint8_t fTargetOffset;
    bool fCheckLoop;
    bool fAllChan;
    bool fDisableStubLogic;

    //to hold the original register values
    DetectorDataContainer fStubLogicCointainer;
    DetectorDataContainer fHIPCountCointainer;

    #ifdef __USE_ROOT__
      DQMHistogramPedestalEqualization fDQMHistogramPedestalEqualization;
    #endif


};


#endif
