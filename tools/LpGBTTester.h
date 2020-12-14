#ifndef LpGBTTester_h__
#define LpGBTTester_h__

#include "Tool.h"
#include "../HWInterface/DPInterface.h"

#ifdef __USE_ROOT__
#include "TAxis.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TObject.h"
#include "TString.h"
#include "TTree.h"
#endif

#ifdef __TCUSB__
#include "USB_a.h"
#endif

#include <map>
#include <string>

using namespace Ph2_HwDescription;

class LpGBTTester : public Tool
{
  public:
    LpGBTTester();
    ~LpGBTTester();

    void Initialise();
    void Start(int currentRun);
    void Stop();
    void Pause();
    void Resume();

    //Test lpGBT Up Link with internal pattern
    void InjectULInternalPattern(uint32_t pPattern);
    //Test lpGBT Up Link with external pattern
    void InjectULExternalPattern(uint8_t pPattern);
    //Check Up Link data in backend fc7
    void CheckULPattern(bool pIsExternal = false);
    //Test lpGBT Down Link with internal pattern (Hybrid Fast Command)
    void InjectDLInternalPattern(uint8_t pPattern);
    //Test lpGBT I2C Masters
    bool TestI2CMaster(const std::vector<uint8_t>& pMasters);
    //Test lpGBT ADC 
    void TestADC(const std::vector<std::string>& pADCs, uint32_t pMinDAC, uint32_t pMaxDAC, uint32_t pStep);
    //Set GPIO level
    void SetGPIOLevel(const std::vector<uint8_t>& pGPIOs, uint8_t Level);

};
#endif
