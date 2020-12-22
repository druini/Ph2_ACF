/*!

        \file                   OTHybridTester.h
        \brief                  Base class for all hybrids test using a testcard 
        \author                 Younes OTARID
        \version                1.0
        \date                   17/12/2020
        Support :               mail to : younes.otarid@cern.ch

 */

#ifndef OTHybridTester_h__
#define OTHybridTester_h__

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

#include <map>
#include <string>

using namespace Ph2_HwDescription;

class OTHybridTester : public Tool 
{
  public:
    OTHybridTester();
    ~OTHybridTester();

    // ###################################
    // # LpGBT related functions #
    // ###################################
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

  private:

  protected:
};

#endif