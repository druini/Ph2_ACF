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

#include "../HWInterface/DPInterface.h"
#include "Tool.h"

#ifdef __USE_ROOT__
#include "TAxis.h"
#include "TF1.h"
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

class OTHybridTester : public Tool
{
  public:
    OTHybridTester();
    ~OTHybridTester();

    void      FindUSBHandler(bool b2SSEH = false);
    TC_PSROH* GetTCUSBHandler()
    {
#ifndef __TC_USB__
        return fTC_PSROH;
#endif

    // ###################################
    // # LpGBT related functions #
    // ###################################
    // Test lpGBT Up Link with internal pattern
    void LpGBTInjectULInternalPattern(uint32_t pPattern);
    // Test lpGBT Up Link with external pattern
    void LpGBTInjectULExternalPattern(uint8_t pPattern);
    // Check Up Link data in backend fc7
    void LpGBTCheckULPattern(bool pIsExternal = false);
    // Test lpGBT Down Link with internal pattern (Hybrid Fast Command)
    void LpGBTInjectDLInternalPattern(uint8_t pPattern);
    // Test lpGBT I2C Masters
    bool LpGBTTestI2CMaster(const std::vector<uint8_t>& pMasters);
    // Test lpGBT ADC
    void LpGBTTestADC(const std::vector<std::string>& pADCs, uint32_t pMinDAC, uint32_t pMaxDAC, uint32_t pStep);
    // Set GPIO level
    void LpGBTSetGPIOLevel(const std::vector<uint8_t>& pGPIOs, uint8_t Level);
    bool LpGBTTestResetLines(uint8_t pLevel);

  private:
#ifdef __TCUSB__
    std::map<std::string, TC_2SSEH::resetMeasurement> f2SSEHResetLines = {{"RST_CBC_R", TC_2SSEH::resetMeasurement::RST_CBC_R},
                                                                          {"RST_CIC_R", TC_2SSEH::resetMeasurement::RST_CIC_R},
                                                                          {"RST_CBC_L", TC_2SSEH::resetMeasurement::RST_CBC_L},
                                                                          {"RST_CIC_L", TC_2SSEH::resetMeasurement::RST_CIC_L}};

    std::map<std::string, TC_PSROH::measurement> fResetLines = {{"L_MPA", TC_PSROH::measurement::L_MPA_RST},
                                                                {"L_CIC", TC_PSROH::measurement::L_CIC_RST},
                                                                {"L_SSA", TC_PSROH::measurement::L_SSA_RST},
                                                                {"R_MPA", TC_PSROH::measurement::R_MPA_RST},
                                                                {"R_CIC", TC_PSROH::measurement::R_CIC_RST},
                                                                {"R_SSA", TC_PSROH::measurement::R_SSA_RST}};

#endif

  protected:
#ifdef __TCUSB__
    TC_PSROH* fTC_PSROH;
    TC_2SSEH* fTC_2SSEH;
#endif
};

#endif
