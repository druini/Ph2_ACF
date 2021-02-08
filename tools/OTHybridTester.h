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
#include "TH2.h"
#include "TH2I.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TObject.h"
#include "TRandom3.h"
#include "TString.h"
#include "TStyle.h"
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
#ifdef __TCUSB__
        return fTC_PSROH;
#endif
    }

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
    bool LpGBTTestFixedADCs(bool p2SSEH = false);
    bool LpGBTTestGPILines(bool p2SSEH = false);
    // Run Eye Openin Monitor
    void LpGBTRunEyeOpeningMonitor(uint8_t pEndOfCountSelect);

  private:
#ifdef __TCUSB__

    std::map<std::string, uint8_t> f2SSEHGPILines = {
        {"PG2V5", 13},
        {"PG1V25", 14},
    };
    std::map<std::string, uint8_t> fPSROHGPILines = {
        {"PWRGOOD", 13},
    };

    std::map<std::string, TC_2SSEH::resetMeasurement> f2SSEHResetLines = {{"RST_CBC_R", TC_2SSEH::resetMeasurement::RST_CBC_R},
                                                                          {"RST_CIC_R", TC_2SSEH::resetMeasurement::RST_CIC_R},
                                                                          {"RST_CBC_L", TC_2SSEH::resetMeasurement::RST_CBC_L},
                                                                          {"RST_CIC_L", TC_2SSEH::resetMeasurement::RST_CIC_L}};

    std::map<std::string, TC_PSROH::measurement> fResetLines             = {{"L_MPA", TC_PSROH::measurement::L_MPA_RST},
                                                                {"L_CIC", TC_PSROH::measurement::L_CIC_RST},
                                                                {"L_SSA", TC_PSROH::measurement::L_SSA_RST},
                                                                {"R_MPA", TC_PSROH::measurement::R_MPA_RST},
                                                                {"R_CIC", TC_PSROH::measurement::R_CIC_RST},
                                                                {"R_SSA", TC_PSROH::measurement::R_SSA_RST}};
    std::map<std::string, float>                 f2SSEHDefaultParameters = {{"Spannung", 2},
                                                            {"Strom", 0.5},
                                                            {"HV", 1},
                                                            {"VMON_P1V25_L_Nominal", 0.806},
                                                            {"VMIN_Nominal", 0.49},
                                                            {"TEMPP_Nominal", 0.6},
                                                            {"VTRX+_RSSI_ADC_Nominal", 0.6},
                                                            {"PTAT_BPOL2V5_Nominal", 0.6},
                                                            {"PTAT_BPOL12V_Nominal", 0.6}};
    std::map<std::string, std::string>           f2SSEHADCInputMap =
        {{"AMUX_L", "ADC0"}, {"VMON_P1V25_L", "ADC1"}, {"VMIN", "ADC2"}, {"AMUX_R", "ADC3"}, {"TEMPP", "ADC4"}, {"VTRX+_RSSI_ADC", "ADC5"}, {"PTAT_BPOL2V5", "ADC6"}, {"PTAT_BPOL12V", "ADC7"}};
    std::map<std::string, std::string> fPSROHADCInputMap       = {{"L_AMUX_OUT", "ADC0"},
                                                            {"1V_MONITOR", "ADC1"},
                                                            {"12V_MONITOR_VD", "ADC2"},
                                                            {"R_AMUX_OUT", "ADC3"},
                                                            {"TEMP", "ADC4"},
                                                            {"VTRX+.RSSI_ADC", "ADC5"},
                                                            {"1V25_MONITOR", "ADC6"},
                                                            {"2V55_MONITOR", "ADC7"}};
    std::map<std::string, float>       fPSROHDefaultParameters = {{"Spannung", 2},
                                                            {"Strom", 0.5},
                                                            {"HV", 1},
                                                            {"12V_MONITOR_VD_Nominal", 0.21},
                                                            {"TEMP_Nominal", 0.6},
                                                            {"VTRX+.RSSI_ADC_Nominal", 0.6},
                                                            {"1V25_MONITOR_Nominal", 0.806},
                                                            {"2V55_MONITOR_Nominal", 0.808}};

#endif

  protected:
#ifdef __TCUSB__
    TC_PSROH* fTC_PSROH;
    TC_2SSEH* fTC_2SSEH;
#endif
};

#endif
